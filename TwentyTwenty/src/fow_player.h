#pragma once

#include "common.h"
#include "game_entity.h"
#include "grid_manager.h"

class FOWSelectable;
class GreenBox;

class FOWPlayer
{
public:

	static void init();

	// FOWPlayer for this one
	static void get_selection();
	static std::vector<t_tile*> GetTiles();
	static void take_input(SDL_Keycode input, bool type);
	static FOWSelectable* get_hit_target();
	static bool supply_available();

	bool is_selectable(entity_types type)
	{
		return (type == FOW_CHARACTER || type == FOW_GATHERER || type == FOW_BUILDING || type == FOW_TOWNHALL || type == FOW_GOLDMINE);
	}

	static void update(float time_delta);
	static void camera_input(SDL_Keycode input, bool type);

	static int gold;
	static int wood;
	static float last_poor_warning;
	static int team_id;
	static int current_tex;

	static t_vertex camera_pos;
	static float camera_distance;
	static std::vector<FOWSelectable*> selection_group;
	static GreenBox *green_box;
	static FOWSelectable *selection;

	// for mouse scroll
	static bool move_camera_left;
	static bool move_camera_right;
	static bool move_camera_up;
	static bool move_camera_down;
	static bool attack_move_mode;

	static bool queue_add_toggle;
};