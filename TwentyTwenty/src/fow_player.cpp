
#include "fow_character.h"
#include "fow_building.h"
#include "fow_player.h"
#include "audiocontroller.h"
#include "gatherer.h"
#include <algorithm>
#include "user_interface.h"

FOWPlayer::FOWPlayer()
{
	queue_add_toggle = false;
	gold = 0;
	green_box = new GreenBox();
	green_box->visible = false;
	camera_distance = 15.0f;
	camera_pos.x = 15;
	camera_pos.y = -15;
	camera_pos.w = 5;

	attack_move_mode = false;
}

void FOWPlayer::update(float time_delta)
{
	green_box->size.x = grid_manager->real_mouse_position.x;
	green_box->size.y = grid_manager->real_mouse_position.y;

	// lets do scroll here...

	float x_percent, y_percent;

	x_percent = (((float)raw_mouse_position.x)/((float)screen.x))*100;
	y_percent = (((float)raw_mouse_position.y) / ((float)screen.y))*100;

	if (x_percent < 5 || move_camera_left)
	{
		camera_pos.x -= 20 * time_delta;
	}

	if (x_percent > 95 || move_camera_right)
	{
		camera_pos.x += 20 * time_delta;
	}

	if (y_percent < 5 || move_camera_up)
	{
		camera_pos.y += 20 * time_delta;
	}

	if (y_percent > 95 || move_camera_down)
	{
		camera_pos.y -= 20 * time_delta;
	}
}

std::vector<t_tile*> FOWPlayer::GetTiles()
{
	t_vertex position = green_box->position;
	t_vertex size = green_box->size;

	t_vertex maxes = t_vertex(std::max(position.x, size.x+1), std::max(-position.y, -size.y), 0.0f);
	t_vertex mins = t_vertex(std::min(position.x, size.x+1), std::min(-position.y, -size.y), 0.0f);

	std::vector<t_tile*> test;

	if (int(mins.x) > 0 && int(mins.x) < grid_manager->width)
	{
		if (int(mins.y) > 0 && int(mins.y) < grid_manager->height)
		{
			if (int(maxes.x) > 0 && int(maxes.x) < grid_manager->width)
			{
				if (int(maxes.y) > 0 && int(maxes.y) < grid_manager->height)
				{
					for (int widthItr = int(mins.x); widthItr<int(maxes.x) + 1; widthItr++)
					{
						for (int heightItr = int(mins.y); heightItr<int(maxes.y) + 1; heightItr++)
						{
							test.push_back(&grid_manager->tile_map[widthItr][heightItr]);
						}
					}
				}
			}
		}
	}
	return test;
}

void FOWPlayer::get_selection()
{
	// clear selected characters
	if (selection_group.size() > 0)
	{
		for (auto selectionItr : selection_group)
		{
			selectionItr->clear_selection();
		}
	}

	selection_group.clear();

	std::vector<t_tile*> tile_set = GetTiles();
	for (auto tile : tile_set)
	{
		if (tile->entity_on_position != nullptr)
		{
			if (std::find(selection_group.begin(), selection_group.end(), tile->entity_on_position) == selection_group.end())
			{
				selection_group.push_back((FOWSelectable*)tile->entity_on_position);
				((FOWSelectable*)tile->entity_on_position)->selected = true;
			}
		}
	}

	if (selection_group.size() > 0)
	{
		selection = selection_group.at(0);
	}
	else
	{
		selection = nullptr;
	}
}


void FOWPlayer::camera_input(SDL_Keycode input, bool type)
{
	if (keymap[RIGHT] == input)
	{
		move_camera_right = type;
	}

	if (keymap[LEFT] == input)
	{
		move_camera_left = type;
	}

	if (keymap[UP] == input)
	{
		move_camera_up = type;
	}

	if (keymap[DOWN] == input)
	{
		move_camera_down = type;
	}

	if (input == MWHEELUP)
	{
		if (camera_pos.w > 5)
		{
			camera_pos.w -= 0.5;
		}
	}

	if (input == MWHEELDOWN)
	{
		if (camera_pos.w < 100)
		{
			camera_pos.w += 0.5;
		}
	}
}

void FOWPlayer::take_input(SDL_Keycode input, bool type)
{
	camera_input(input, type);

	if (selection != nullptr)
	{
		for (auto selectionItr : selection_group)
		{
			if (selectionItr->team_id == 0)
			{
				selectionItr->take_input(input, type, queue_add_toggle);
			}
		}
	}

	if (keymap[ATTACK_MOVE_MODE] == input && type == true)
	{
		attack_move_mode = true;
	}

	if (input == LMOUSE && type == true)
	{
		green_box->position.x = grid_manager->real_mouse_position.x;
		green_box->position.y = grid_manager->real_mouse_position.y;
		green_box->mouse_in_space = grid_manager->real_mouse_position;
		green_box->visible = true;

		// if in attack move command mode, send attack move command to selected units...
		if (attack_move_mode)
		{
			attack_move_mode = false;
			for (auto selectionItr : selection_group)
			{
				if (selectionItr->team_id == 0)
				{
					if (selectionItr->is_unit())
					{
						((FOWCharacter*)selectionItr)->give_command(FOWCommand(ATTACK_MOVE, t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0.0f)));
					}
				}
			}
		}
	}

	if (input == LMOUSE && type == false)
	{
		green_box->visible = false;
		get_selection();
	}
}