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

class ServerHandler;
class LevelEditor;
class MapWidget;
class UIProgressBar;

typedef enum {
	PLAY_MODE,
	EDIT_MODE
}e_gamestate;

class Game
{
public:
	Game()
	{
		relative_mouse_position = t_vertex(0, 0, 0);
		game_speed = 1;
	}

	static MapWidget* minimap;

	// PRE-Fog Of War Merge
	static e_gamestate game_state;
	static std::vector<GameEntity*> entities;
	static t_vertex raw_mouse_position;	// X, Y screen position
	static t_vertex real_mouse_position; // X, Y, Z world space
	static t_vertex coord_mouse_position;  // Integer coordinates for the grid
	static t_vertex relative_mouse_position;

	static float game_speed;
	static std::string mapname;
	static bool initialized;
	static UIProgressBar* new_bar;

	static bool init(std::string new_mapname);
	static void run(float deltatime);
	static void draw();
	static void draw_ui();
	static void take_input(SDL_Keycode input, bool keydown);
	static void get_mouse_in_space();
};