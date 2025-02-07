#ifndef CONFIG_H
#define CONFIG_H

struct GameConfig
{
	unsigned int width;
	unsigned int height;
	unsigned int tickRate;  // ticks per second
	unsigned int coreHP;
	unsigned int unitHP;
};

GameConfig defaultConfig();

#endif // CONFIG_H
