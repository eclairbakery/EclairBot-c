#include <lib/concord.h>

typedef struct DscLogger {
    void (*call)(struct DscLogger* logger);

    const char* channel_id;
    DiscordChannel* fetched_channel;

    FILE* output;
} DscLogger;

DscLogger mk_dsc_logger(FILE* output);
