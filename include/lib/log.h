#pragma once
#include <lib/concord.h>

typedef enum LoggerFlags {
    LoggerFlagsNone = 0,
    LoggerFlagsEnableFormat = 1 << 0,
    LoggerFlagsEnableColor  = 1 << 1,
} LoggerFlags;

typedef struct LoggerLocalConfigEntry {
    FILE* stream;
} LoggerLocalConfigEntry;

typedef struct LoggerLocalConfig {
    LoggerLocalConfigEntry log;
    LoggerLocalConfigEntry warn;
    LoggerLocalConfigEntry err;
} LoggerLocalConfig;

typedef struct LoggerDiscordConfigEntry {
    DiscordChannelID channel_id;
} LoggerDiscordConfigEntry;

typedef struct LoggerDiscordConfig {
    LoggerDiscordConfigEntry log;
    LoggerDiscordConfigEntry warn;
    LoggerDiscordConfigEntry err;
} LoggerDiscordConfig;

void logger_init(LoggerFlags flags);
void logger_init_local(LoggerLocalConfig local_cfg);
void logger_init_discord(LoggerDiscordConfig discord_cfg);
void logger_deinit(void);

void show_log(const char* text);
void show_logv(const char* fmt, va_list args);
void show_logf(const char* fmt, ...);

void show_warn(const char* text);
void show_warnv(const char* fmt, va_list args);
void show_warnf(const char* fmt, ...);

void show_err(const char* text);
void show_errv(const char* fmt, va_list args);
void show_errf(const char* fmt, ...);

void panic(const char* text);
void panicf(const char* fmt, ...);
