#pragma once

#include <vector>
#include "spine_manager.h"
#include "spine_entity.h"
#include "common.h"

#include "grid_manager.h"
#include "fow_player.h"
#include "fow_character.h"
#include "fow_editor.h"
#include "fow_building.h"

class LevelEditor;

typedef enum {
	PLAY_MODE,
	EDIT_MODE
}e_gamestate;

class Game
{
public:
	Game()
	{
		relative_mouse_position = t_transform(0, 0, 0, 0);
		game_speed = 1;
	}

	GridManager grid_manager;
	FOWPlayer player;
	FOWEditor editor;

	FOWCharacter* new_character;

	// PRE-Fog Of War Merge
	GLuint texture;
	SpineManager spine_manager;
	e_gamestate game_state;
	static std::vector<GameEntity*> entities;
	t_transform raw_mouse_position;
	static t_transform real_mouse_position;
	static t_transform relative_mouse_position;

	float game_speed;

	bool init();
	void run(float deltatime);
	void draw();
	void take_input(boundinput input, bool keydown);
	void get_mouse_in_space();
};