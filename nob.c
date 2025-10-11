#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NOB_IMPLEMENTATION
#include "external/nob.h"

#define CHECK(ret) do { if (ret != 0) return ret; } while (0)

#define BUILD_DIR "build/"
#define OUT_DIR "out/"
#define OUT OUT_DIR"eclairbot.elf"

// helper
bool read_entire_dir_recursive(const char *parent, Nob_File_Paths *children, size_t base_len) {
    Nob_File_Paths entries = {0};
    if (!nob_read_entire_dir(parent, &entries)) return false;

    for (size_t i = 0; i < entries.count; i++) {
        const char* name = entries.items[i];
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", parent, name);

        Nob_File_Type type = nob_get_file_type(path);

        if (type == NOB_FILE_DIRECTORY) {
            read_entire_dir_recursive(path, children, base_len);
        } else {
            size_t len = strlen(path);
            if (len > 2 && strcmp(path + len - 2, ".c") == 0) {
                const char *rel_path = path + base_len; // skip "src/"
                if (*rel_path == '/') rel_path++;       // skip leading '/'
                nob_da_append(children, nob_temp_strdup(rel_path));
            }
        }
    }

    nob_da_free(entries);
    return true;
}

typedef struct Flags {
    const char** items;
    size_t count;
    size_t capacity;
} Flags;


int chdir_to_project_root() {
    return chdir(getenv("__ECLAIRBOTC_PROJECT_ROOT"));
}

int build_concord(Flags* additional_compile_flags, Flags* additional_link_flags) {
    chdir("external/concord");

    Nob_Cmd make = {0};
    nob_cmd_append(&make, "make");

    if (!nob_cmd_run_sync_and_reset(&make)) {
        nob_log(NOB_ERROR, "Failed to build concord: Makefile failed");
        goto error;
    }

    nob_cmd_free(make);

    nob_da_append(additional_compile_flags, "-Iexternal/concord/include");
    nob_da_append(additional_compile_flags, "-Iexternal/concord/core");
    nob_da_append(additional_compile_flags, "-Iexternal/concord/gencodecs");

    nob_da_append(additional_link_flags, "-Lexternal/concord/lib");

success:
    chdir_to_project_root();
    nob_log(NOB_INFO, "Concord compiled successfully");
    return 0;
error:
    chdir_to_project_root();
    return 1;
}

int build_external_libs(Flags* additional_compile_flags, Flags* additional_link_flags) {
    CHECK(build_concord(additional_compile_flags, additional_link_flags));

    return 0;
}

bool needs_rebuild(const char *src, const char *obj) {
    struct stat src_stat, obj_stat;
    if (stat(src, &src_stat) != 0) return true;  // just in case
    if (stat(obj, &obj_stat) != 0) return true;  // rebuild if object file does not exists
    return src_stat.st_mtime > obj_stat.st_mtime;
}

int build(int argc, char** argv) { 
    chdir_to_project_root();
    nob_mkdir_if_not_exists(BUILD_DIR);
    nob_mkdir_if_not_exists(OUT_DIR);

    Flags compile_flags = {0}, link_flags = {0};
    build_external_libs(&compile_flags, &link_flags);

    nob_da_append(&compile_flags, "-Isrc/include");

    chdir_to_project_root();

    Nob_File_Paths sources = {0};
    read_entire_dir_recursive("src", &sources, strlen("str"));

    Nob_File_Paths objects = {0};

    for (size_t i = 0; i < sources.count; ++i) {
        char source_path[PATH_MAX];
        snprintf(source_path, sizeof source_path, "./src/%s", sources.items[i]);

        char path_normalized[PATH_MAX];
        strncpy(path_normalized, sources.items[i], sizeof path_normalized);
        path_normalized[sizeof(path_normalized) - 1] = '\0';
        
        int len = strlen(path_normalized);
        if (len > 2 && strcmp(path_normalized + len - 2, ".c") == 0) {
            path_normalized[len - 2] = '\0'; // remove .c
            len -= 2;
        }
        
        for (int i = 0; i < len; ++i) {
            if (path_normalized[i] == '/') path_normalized[i] = '_';
        }
        
        char out_obj_path[PATH_MAX];
        snprintf(out_obj_path, sizeof out_obj_path, "./build/%s.o", path_normalized);

        if (!needs_rebuild(source_path, out_obj_path)) {
            nob_log(NOB_INFO, "Skipping %s.c (up to date)", path_normalized);
            nob_da_append(&objects, out_obj_path);
            continue;
        }

        Nob_Cmd cc = {0};
        nob_cc(&cc);
        nob_cmd_append(&cc, source_path, "-o", out_obj_path, "-c");
        nob_cc_flags(&cc);
        nob_cmd_extend(&cc, &compile_flags);

        if (!nob_cmd_run_sync_and_reset(&cc)) {
            nob_log(NOB_ERROR, "Failed to compile %s.c", path_normalized);
            nob_cmd_free(cc);
            return 1;
        }

        nob_cmd_free(cc);
        nob_da_append(&objects, out_obj_path);
    }

    Nob_Cmd link = {0};
    nob_cc(&link);
    nob_cmd_extend(&link, &link_flags);
    nob_cmd_extend(&link, &objects);
    nob_cmd_append(&link, "-o", OUT);

    if (!nob_cmd_run_sync_and_reset(&link)) {
        nob_log(NOB_ERROR, "Failed to link executable %s.", OUT);
    }

    nob_cmd_free(link);

    nob_da_free(compile_flags);
    nob_da_free(link_flags);

    nob_log(NOB_INFO, "EclairBot-C compiled and linked successfully");
    return 0;
}

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (argc == 1) {
        CHECK(build(argc, argv));
    }
}
