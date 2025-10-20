#include <bot/event_handlers/ready.h>

#include <lib/concord.h>
#include <lib/log.h>

void bot_on_ready(DiscordClient* client, const DiscordReady* ready) {
    LoggerDiscordConfig logger_discord_cfg = {
        .log.channel_id = 1420415468384288768,
        .warn.channel_id = 0,
        .err.channel_id = 0,
    };
    logger_init_discord(logger_discord_cfg);
}
