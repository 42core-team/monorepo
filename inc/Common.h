#ifndef COMMON_H
#define COMMON_H

#include <cmath>
#include <time.h>
#include <random>

struct Position
{
	int x;
	int y;

	Position(int x, int y) : x(x), y(y) {}
	Position() : x(-1), y(-1) {} // purposeful overflow, max val to indicate invalidity

	bool isValid(int maxX, int maxY) const
	{
		return x < maxX && y < maxY && x >= 0 && y >= 0;
	}

	bool operator==(const Position &other) const
	{
		return x == other.x && y == other.y;
	}

	Position operator+(const Position &other) const
	{
		return {x + other.x, y + other.y};
	}
	Position operator+(int scalar) const
	{
		return {x + scalar, y + scalar};
	}
	Position operator-(const Position &other) const
	{
		return {x - other.x, y - other.y};
	}
	Position operator*(int scalar) const
	{
		return {x * scalar, y * scalar};
	}

	double distance(const Position &other) const
	{
		return std::abs(x - other.x) + std::abs(y - other.y);
	}

	static Position random(int maxSizeSquare)
	{
		static std::default_random_engine eng = std::default_random_engine(time(nullptr));
		std::uniform_int_distribution<int> posX(0, maxSizeSquare - 1);
		std::uniform_int_distribution<int> posY(0, maxSizeSquare - 1);
		return Position(posX(eng), posY(eng));
	}
};

#endif // COMMON_H