#include "MapTemplate.h"

void MapTemplate::loadFromFile(const std::string &filepath)
{
	std::ifstream infile(filepath);
	if (!infile)
		throw std::runtime_error("Unable to open template file: " + filepath);

	std::string line;
	int maxWidth = 0;
	while (std::getline(infile, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		std::vector<char> row(line.begin(), line.end());
		maxWidth = std::max(maxWidth, static_cast<int>(row.size()));
		grid.push_back(row);
	}
	if (grid.empty())
		throw std::runtime_error("Empty template file: " + filepath);

	for (auto &row : grid)
	{
		if (static_cast<int>(row.size()) < maxWidth)
			row.resize(maxWidth, ' ');
	}
	height = static_cast<int>(grid.size());
	width = maxWidth;
}

MapTemplate MapTemplate::rotate90() const
{
	MapTemplate result;
	result.width = height;
	result.height = width;
	result.grid.resize(result.height, std::vector<char>(result.width, ' '));
	for (int i = 0; i < height; ++i)
		for (int j = 0; j < width; ++j)
			result.grid[j][result.width - 1 - i] = grid[i][j];
	return result;
}

MapTemplate MapTemplate::mirrorHorizontally() const
{
	MapTemplate result;
	result.height = height;
	result.width = width;
	result.grid = grid;
	for (int i = 0; i < height; ++i)
		std::reverse(result.grid[i].begin(), result.grid[i].end());
	return result;
}

MapTemplate MapTemplate::mirrorVertically() const
{
	MapTemplate result;
	result.height = height;
	result.width = width;
	result.grid = grid;
	std::reverse(result.grid.begin(), result.grid.end());
	return result;
}

MapTemplate MapTemplate::getTransformedTemplate(std::mt19937_64 &eng) const
{
	MapTemplate result = *this;

	std::uniform_int_distribution<int> rotationDist(0, 3);
	int rotations = rotationDist(eng);
	for (int i = 0; i < rotations; ++i)
		result = result.rotate90();

	std::uniform_int_distribution<int> mirrorDist(0, 2);
	int mirrorType = mirrorDist(eng);
	if (mirrorType == 1)
		result = result.mirrorHorizontally();
	else if (mirrorType == 2)
		result = result.mirrorVertically();

	return result;
}
