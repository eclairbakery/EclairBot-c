#include <bot/bot.h>

#include <stdlib.h>

int main(int argc, char** argv) {
    const char* token = getenv("ECLAIRBOT_TOKEN");
    if (token == NULL) {
        fputs("ERROR: env variable ECLAIRBOT_TOKEN must be set.", stderr);
    }

    Bot bot;
    bot_init(&bot, token);
    bot_run(&bot);
    bot_deinit(&bot);
}
