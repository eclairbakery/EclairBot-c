#include <bot/bot.h>
#include <bot/event_handlers/message_create.h>

#define ECLAIRBOT_INTENTS \
        ( DISCORD_GATEWAY_DIRECT_MESSAGES                       \
        | DISCORD_GATEWAY_GUILD_MESSAGES                        \
        | DISCORD_GATEWAY_MESSAGE_CONTENT                       \
        | DISCORD_GATEWAY_GUILDS                                \
        | DISCORD_GATEWAY_GUILD_MEMBERS                         \
        | DISCORD_GATEWAY_GUILD_MESSAGE_REACTIONS               \
        )

void bot_init(Bot* bot, const char* token) {
    bot->client = discord_init(token);
    discord_add_intents(bot->client, ECLAIRBOT_INTENTS);

    discord_set_data(bot->client, bot);
    discord_set_on_message_create(bot->client, bot_on_message_create);
}

void bot_run(Bot* bot) {
    discord_run(bot->client);
}

void bot_deinit(Bot* bot) {
    discord_cleanup(bot->client);
}
