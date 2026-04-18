#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

static std::string readFile(const std::string &filename) {
	std::ifstream file(filename);

	if (!file.is_open())
		std::cerr << "Error opening the file!" << std::endl;

	std::ostringstream sstr;
	sstr << file.rdbuf();

	return sstr.str();
}
