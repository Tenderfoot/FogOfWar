#pragma once

#include <vector>
#include "spine_manager.h"
#include "spine_entity.h"
#include "common.h"

#include "grid_manager.h"
#include "fow_player.h"

class LevelEditor;

typedef enum {
	PLAY_MODE,
	EDIT_MODE
}e_gamestate;

class Game
{
public:
	
	GridManager grid_manager;
	FOWPlayer player;

	// PRE-Fog Of War Merge
	GLuint texture;
	SpineManager spine_manager;
	e_gamestate game_state;
	static std::vector<GameEntity*> entities;
	t_transform raw_mouse_position;
	static t_transform real_mouse_position;
	static t_transform relative_mouse_position;

	bool init();
	void run(float deltatime);
	void draw();
	void take_input(boundinput input, bool keydown);
	void draw_aabb(t_transform aabb);
	void get_mouse_in_space();
};