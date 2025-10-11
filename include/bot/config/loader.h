#include <bot/config/struct.h>

typedef struct ConfigLoadResult {
    enum ConfigLoadErrorCode {
        /*
         * Indicates successful configuration loading.
         * error_details.other = NULL
         */
        ConfigLoadSuccess = 0,
    
        /*
         * Returned exclusively by load_config_from_file when the specified file 
         *  does not exist or cannot be read.
         * error_details.view = strerror(errno) after failed fopen
         */
        ConfigLoadErrorFailedToReadFile,
    
        /*
         * Returned when the configuration contains invalid JSON syntax.
         * error_details.owned = error message from jansson library
         */
        ConfigLoadErrorJsonSyntaxError,
    
        /*
         * Returned when a required field (without default value) is missing from config.
         * error_details.owned = path to missing field in category.(subcategories.*).name format (e.g., bot.db_path)
         */
        ConfigLoadErrorMissingRequiredField,
    
        /*
         * Returned when a field contains an invalid value (e.g., wrong data type).
         * error_details.owned = path to problematic field in category.(subcategories.*).name format (e.g., bot.db_path)
         */
        ConfigLoadErrorInvalidValue,
    } code;
    union {
        char* owned; // malloc'ed
        const char* view;
        void* other;
    } error_details;
} ConfigLoadResult;

ConfigLoadResult load_config_from_file(Config* cfg, const char* filepath);
ConfigLoadResult load_config_from_string(Config* cfg, const char* str);

void free_config_load_result(ConfigLoadResult* result);
