#pragma once

#include <string>
#include <fstream>
#include "json.hpp"

class Level;
void from_json(const nlohmann::json& j, Level& l);

class Level
{
public:

	std::string name;
	nlohmann::json level_data;
};