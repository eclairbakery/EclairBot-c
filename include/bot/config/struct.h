#pragma once

typedef struct Config {
    struct BotConfig {
        const char* db_path;
    } bot;

    struct GeneralConfig {
                
    } general;


    struct FeaturesConfig {
        struct LevelingFeatureConfig {
            int xp_per_message;
            int level_divider;
            
        } leveling;

        struct EconomyFeatureConfig {
            
        } economy;
    } features;
} Config;
