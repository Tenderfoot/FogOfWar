#pragma once

#include "spine_manager.h"

class Game
{
public:
	GLuint texture;
	SpineManager spine_manager;

	bool init();
	void run(float deltatime);
	void draw();
};