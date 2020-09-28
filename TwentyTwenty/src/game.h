#pragma once

#include "spine_manager.h"
#include "spine_entity.h"

class Game
{
public:
	GLuint texture;
	SpineManager spine_manager;

	bool init();
	void run(float deltatime);
	void draw();

	SpineEntity *test;
};