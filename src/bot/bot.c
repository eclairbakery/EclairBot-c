#include <lib/concord.h>
#include <jansson.h>



typedef struct Bot {
    const char* token;
    

    DiscordClient* client;
    
} Bot;
