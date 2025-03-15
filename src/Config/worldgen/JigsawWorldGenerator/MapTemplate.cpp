#include "MapTemplate.h"

void MapTemplate::loadFromFile(const std::string &filepath)
{
	std::ifstream infile(filepath);
	if (!infile)
		throw std::runtime_error("Unable to open template file: " + filepath);
	
	std::string line;
	while (std::getline(infile, line))
	{
		std::vector<char> row(line.begin(), line.end());
		grid.push_back(row);
	}
	if (grid.empty())
		throw std::runtime_error("Empty template file: " + filepath);
	
	height = grid.size();
	width = grid[0].size();
	for (const auto &row : grid)
		if ((int)row.size() != width)
			throw std::runtime_error("Inconsistent row widths in template: " + filepath);
}
