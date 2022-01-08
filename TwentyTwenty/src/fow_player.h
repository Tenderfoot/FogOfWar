#pragma once

#include "common.h"
#include "game_entity.h"
#include "grid_manager.h"

class FOWSelectable;
class GreenBox;

class FOWPlayer
{
public:

	FOWPlayer();

	// FOWPlayer for this one
	void get_selection();
	std::vector<t_tile*> GetTiles();
	virtual void take_input(SDL_Keycode input, bool type);


	bool is_selectable(entity_types type)
	{
		return (type == FOW_CHARACTER || type == FOW_GATHERER || type == FOW_BUILDING || type == FOW_TOWNHALL || type == FOW_GOLDMINE);
	}

	virtual void update(float time_delta);
	void camera_input(SDL_Keycode input, bool type);

	int gold;

	t_vertex gridstart_ui;
	t_vertex gridstart_world;
	t_transform camera_pos;
	float camera_distance;
	std::vector<FOWSelectable*> selection_group;
	GreenBox *green_box;
	float last_poor_warning;
	FOWSelectable *selection;
	GridManager *grid_manager;

	// for mouse scroll
	t_transform raw_mouse_position;
	t_transform screen;
	bool move_camera_left;
	bool move_camera_right;
	bool move_camera_up;
	bool move_camera_down;
	bool attack_move_mode;

	bool queue_add_toggle;
};