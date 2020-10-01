#pragma once

#include <vector>
#include "spine_manager.h"
#include "spine_entity.h"
#include "player.h"
#include "level.h"
#include "common.h"
#include "leveleditor.h"

class LevelEditor;

typedef enum {
	PLAY_MODE,
	EDIT_MODE
}e_gamestate;

class Game
{
public:
	GLuint texture;
	SpineManager spine_manager;
	e_gamestate game_state;
	LevelEditor level_editor;
	static std::vector<Entity*> entities;
	Level new_level;
	Player* witch;
	t_transform raw_mouse_position;
	static t_transform real_mouse_position;
	t_transform relative_mouse_position;

	bool init();
	void run(float deltatime);
	void draw();
	void take_input(boundinput input, bool keydown);
	void draw_aabb(t_transform aabb);
	bool load_level(std::string filename);
	void get_mouse_in_space();
};