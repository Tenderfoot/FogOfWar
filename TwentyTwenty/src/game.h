#pragma once

#include <vector>
#include "spine_manager.h"
#include "spine_entity.h"
#include "player.h"
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

	static std::vector<Entity*> entities;
	Player *witch;
};