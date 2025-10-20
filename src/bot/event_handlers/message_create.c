#include <bot/event_handlers/message_create.h>

#include <lib/concord.h>
#include <lib/log.h>

#include <string.h>

void bot_on_message_create(DiscordClient* client, const DiscordMessage* msg) {
    Bot* bot = discord_get_data(client);
    if (strcmp(msg->content, "sudo ping") == 0) {
        discord_reply_text(client, msg, "Pong!");
        show_log("command: sudo ping");
    }
}
