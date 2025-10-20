#include <bot/bot.h>
#include <lib/log.h>

#include <stdlib.h>

Bot bot;
int main(int argc, char** argv) {
    logger_init(LoggerFlagsNone);
    logger_init_local((LoggerLocalConfig) {
        .log.stream = stdout,
        .warn.stream = stderr,
        .err.stream = stderr,
    });

    const char* token = getenv("ECLAIRBOT_TOKEN");
    if (token == NULL) {
        panic("env variable ECLAIRBOT_TOKEN must be set.");
    }

    bot_init(&bot, token);
    bot_run(&bot);
    bot_deinit(&bot);
}
