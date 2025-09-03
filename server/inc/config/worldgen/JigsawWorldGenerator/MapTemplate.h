#pragma once

#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class MapTemplate
{
  public:
	std::vector<std::vector<char>> grid;
	int width;
	int height;
	std::string name;

	MapTemplate() : width(0), height(0) {}
	MapTemplate(const std::string &filepath)
	{
		loadFromFile(filepath);
		name = filepath;
	}

	MapTemplate getTransformedTemplate(std::mt19937_64 &eng) const;

	MapTemplate rotate90() const;

  private:
	void loadFromFile(const std::string &filepath);

	MapTemplate mirrorHorizontally() const;
	MapTemplate mirrorVertically() const;
};
