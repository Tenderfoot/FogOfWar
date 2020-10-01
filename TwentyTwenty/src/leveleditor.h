#pragma once

#include "common.h"
#include "entity.h"
#include <vector>
#include <map>

class LevelEditor
{
public:

	LevelEditor() {
		camera_transform.x = 0;
		camera_transform.y = 0;
	}

	t_transform camera_transform;
	std::vector<Entity*> selected_entities;
	std::map<boundinput, bool> keydown_map;
	void take_input(boundinput input, bool keydown);
	bool is_selected(Entity* test_entity);
	void update();
};