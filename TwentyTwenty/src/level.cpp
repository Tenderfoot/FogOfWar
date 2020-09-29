
#include "level.h"

void from_json(const nlohmann::json& j, Level& l) {
	j.at("name").get_to(l.name);
}

bool Level::LoadLevel(std::string filename) {
	std::ifstream i(filename);
	i >> level_data;

	// import settings
	Level test = level_data.get<Level>();

	printf("Level Name: %s\n", test.name.c_str());

	return true;
}