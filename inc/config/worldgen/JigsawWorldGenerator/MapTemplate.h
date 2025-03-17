#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <random>

class MapTemplate {
public:
	std::vector<std::vector<char>> grid;
	int width;
	int height;
	std::string name;

	MapTemplate(): width(0), height(0) {}
	MapTemplate(const std::string &filepath) {
		loadFromFile(filepath);
		name = filepath;
	}

	MapTemplate getTransformedTemplate(std::default_random_engine &eng) const;

	MapTemplate rotate90() const;

private:
	void loadFromFile(const std::string &filepath);

	MapTemplate mirrorHorizontally() const;
	MapTemplate mirrorVertically() const;
};
