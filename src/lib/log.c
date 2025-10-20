#include <lib/log.h>

#include <lib/concord.h>
#include <bot/bot.h>
#include <defs.h>

#include <sys/stat.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct LogEntry {
    LoggerDiscordConfigEntry discord;
    LoggerLocalConfigEntry local;
} LogEntry;

typedef struct LoggerSharedContext {
    FILE* logs_file;
    LoggerFlags flags;

    LogEntry log;
    LogEntry warn;
    LogEntry err;
} LoggerSharedContext;

static LoggerSharedContext logger_shared_ctx;

static void panic_internal(const char* msg) {
    fputs(msg, stderr);
    exit(1);
}

void logger_init(LoggerFlags flags) {
    logger_shared_ctx.flags = flags;

    logger_shared_ctx.log.discord.channel_id  = 0;
    logger_shared_ctx.warn.discord.channel_id = 0;
    logger_shared_ctx.err.discord.channel_id  = 0;

    logger_shared_ctx.log.local.stream = NULL;
    logger_shared_ctx.warn.local.stream = NULL;
    logger_shared_ctx.err.local.stream = NULL;
}

void logger_init_local(LoggerLocalConfig local_cfg) {
    if (mkdir("logs", 0755) == -1 && errno != EEXIST)
        panic_internal("Failed to create logs/ directory\n");

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    char buf[128] = "logs/";
    size_t prefix_len = strlen(buf);
    if (strftime(buf + prefix_len, sizeof(buf) - prefix_len, "%Y-%m-%d@%H-%M-%S", t) == 0)
        panic_internal("strftime buffer too small\n");

    strcat(buf, ".log");

    FILE* f = fopen(buf, "w");
    if (!f) panic_internal("Failed to open log file\n");

    logger_shared_ctx.logs_file = f;
    logger_shared_ctx.log.local  = local_cfg.log;
    logger_shared_ctx.warn.local = local_cfg.warn;
    logger_shared_ctx.err.local  = local_cfg.err;
}

void logger_init_discord(LoggerDiscordConfig discord_cfg) {
    logger_shared_ctx.log.discord  = discord_cfg.log;
    logger_shared_ctx.warn.discord = discord_cfg.warn;
    logger_shared_ctx.err.discord  = discord_cfg.err;
}

static inline void _logl(LogEntry* entry, const char* text, usize len) {
    char timebuf[64];
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    if (strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t) == 0)
        panic_internal("strftime buffer too small\n");

    // local stream
    if (entry->local.stream)
        fprintf(entry->local.stream, "[%s]: %.*s\n", timebuf, (int) len, text);

    // discord
    if (entry->discord.channel_id) {
        char buf[DISCORD_MESSAGE_CONTENT_MAX_LEN];
        for (usize i = 0; i < len; i += DISCORD_MESSAGE_CONTENT_MAX_LEN - 15) {
            usize chunk_len = len - i;
            if (chunk_len > DISCORD_MESSAGE_CONTENT_MAX_LEN - 15)
                chunk_len = DISCORD_MESSAGE_CONTENT_MAX_LEN - 15;
            snprintf(buf, sizeof(buf), "```\n%.*s%s\n```",
                     (int) chunk_len, text + i,
                     (i + chunk_len < len) ? "..." : "");
            discord_send_text_with_len(bot.client, entry->discord.channel_id, buf, strlen(buf));
        }
    }

    // main logs file
    if (logger_shared_ctx.logs_file) {
        fprintf(logger_shared_ctx.logs_file, "[%s]: %.*s\n", timebuf, (int) len, text);
        fflush(logger_shared_ctx.logs_file);
    }
}

static inline void _log(LogEntry* e, const char* text) {
    _logl(e, text, strlen(text));
}

static inline void _logv(LogEntry* e, const char* fmt, va_list args) {
    char buf[4096];
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    _logl(e, buf, len > 0 ? (usize) len : 0);
}

#define DEFINE_LOG_FUNCS(NAME) \
    void show_##NAME(const char* text) { _log(&logger_shared_ctx.NAME, text); } \
    void show_##NAME##v(const char* fmt, va_list args) { _logv(&logger_shared_ctx.NAME, fmt, args); } \
    void show_##NAME##f(const char* fmt, ...) { \
        va_list args; \
        va_start(args, fmt); \
        _logv(&logger_shared_ctx.NAME, fmt, args); \
        va_end(args); \
    }

DEFINE_LOG_FUNCS(log);
DEFINE_LOG_FUNCS(warn);
DEFINE_LOG_FUNCS(err);

void panic(const char* text) {
    show_err(text);
    panic_internal(text);
}

void panicf(const char* fmt, ...) {
    va_list args; va_start(args, fmt);
    show_errv(fmt, args);
    va_end(args);
    panic_internal("panicf() called\n");
}

void logger_deinit(void) {
    if (logger_shared_ctx.logs_file) {
        fflush(logger_shared_ctx.logs_file);
        fclose(logger_shared_ctx.logs_file);
        logger_shared_ctx.logs_file = NULL;
    }
}

