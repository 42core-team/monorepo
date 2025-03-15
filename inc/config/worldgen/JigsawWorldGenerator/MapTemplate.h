#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

class MapTemplate {
public:
	std::vector<std::vector<char>> grid;
	int width;
	int height;

	MapTemplate(const std::string &filepath) {
		loadFromFile(filepath);
	}

private:
	void loadFromFile(const std::string &filepath);
};
