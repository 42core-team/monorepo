#ifndef COMMON_H
#define COMMON_H

enum class MovementDirection
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};

struct Position
{
	unsigned int x;
	unsigned int y;
};

#endif // COMMON_H