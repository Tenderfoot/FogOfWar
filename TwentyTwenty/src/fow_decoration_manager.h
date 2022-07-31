#pragma once

#include "spine_entity.h"
#include "grid_manager.h"

class FOWDecorationManager
{
public:

	FOWDecorationManager();

	static void make_decorations();
	static void draw_vao();
	// This runs a thread, updating all of the verticies for all the decorations
	static void update();
	// this rebinds the updated data to the vertex array
	static void game_update();

	static std::vector<GameEntity*> decorations;

};