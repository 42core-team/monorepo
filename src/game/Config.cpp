#include "Config.h"

GameConfig defaultConfig() {
    GameConfig config;
    config.width = 10000;
    config.height = 10000;
    config.tickRate = 30;    // 30 ticks per second
    config.coreHP = 50000;
    config.unitHP = 3500;    // for example
    return config;
}
