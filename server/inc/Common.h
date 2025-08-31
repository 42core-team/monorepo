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
	Position() : x(-1), y(-1) {}

	bool isValid(int maxGrid) const
	{
		return x < maxGrid && y < maxGrid && x >= 0 && y >= 0;
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

	// needed for storing positions in sets, otherwise rather nonsensical
	bool operator<(const Position &other) const
	{
		return (y < other.y) || (y == other.y && x < other.x);
	}

	double distance(const Position &other) const
	{
		return std::abs(x - other.x) + std::abs(y - other.y);
	}
};

#endif // COMMON_H