#pragma once

#include <vector>
#include "spine_manager.h"
#include "spine_entity.h"
#include "player.h"
#include "level.h"
#include "common.h"

class Game
{
public:
	GLuint texture;
	SpineManager spine_manager;

	bool init();
	void run(float deltatime);
	void draw();
	void take_input(boundinput input, bool keydown);
	void draw_aabb(t_transform aabb);
	bool load_level(std::string filename);

	static std::vector<Entity*> entities;
	Level new_level;
	Player *witch;
};