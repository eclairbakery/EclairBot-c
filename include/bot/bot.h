#pragma once
#include <lib/concord.h>

typedef struct Bot {
    const char* token;
    DiscordClient* client;
} Bot;

void bot_init(Bot* bot, const char* token);
void bot_run(Bot* bot);
void bot_deinit(Bot* bot);
