
#include "fow_character.h"
#include "fow_building.h"
#include "fow_player.h"
#include "audiocontroller.h"
#include "gatherer.h"
#include "Settings.h"
#include <algorithm>
#include "user_interface.h"
#include "game.h"

extern Settings user_settings;
t_vertex FOWPlayer::camera_pos;
int FOWPlayer::gold;
float FOWPlayer::last_poor_warning;
bool FOWPlayer::attack_move_mode;
bool FOWPlayer::queue_add_toggle;

float FOWPlayer::camera_distance;
std::vector<FOWSelectable*> FOWPlayer::selection_group;
GreenBox* FOWPlayer::green_box;
FOWSelectable* FOWPlayer::selection;
bool FOWPlayer::move_camera_left;
bool FOWPlayer::move_camera_right;
bool FOWPlayer::move_camera_up;
bool FOWPlayer::move_camera_down;


void FOWPlayer::init()
{
	queue_add_toggle = false;
	gold = 0;
	camera_distance = 15.0f;
	camera_pos.x = 15;
	camera_pos.y = -15;
	camera_pos.z = 15;
	attack_move_mode = false;
}

void FOWPlayer::update(float time_delta)
{
	green_box->size = Game::raw_mouse_position;

	// lets do scroll here...

	float x_percent=50, y_percent=50;	// pretend mouse was in center, this is a hack
	if (user_settings.use_scroll)
	{
		x_percent = (((float)Game::raw_mouse_position.x) / ((float)user_settings.width)) * 100;
		y_percent = (((float)Game::raw_mouse_position.y) / ((float)user_settings.height)) * 100;
	}

	if (x_percent < 2 || move_camera_left)
	{
		camera_pos.x -= 20 * time_delta;
	}

	if (x_percent > 98 || move_camera_right)
	{
		camera_pos.x += 20 * time_delta;
	}

	if (y_percent < 2 || move_camera_up)
	{
		camera_pos.y += 20 * time_delta;
	}

	if (y_percent > 98 || move_camera_down)
	{
		camera_pos.y -= 20 * time_delta;
	}
	
}

std::vector<t_tile*> FOWPlayer::GetTiles()
{
	t_vertex position = green_box->mouse_in_space;
	t_vertex size = Game::real_mouse_position;

	t_vertex maxes = t_vertex(std::max(position.x, size.x+1), std::max(-position.y, -size.y), 0.0f);
	t_vertex mins = t_vertex(std::min(position.x, size.x+1), std::min(-position.y, -size.y), 0.0f);

	std::vector<t_tile*> test;

	if (int(mins.x) > 0 && int(mins.x) < GridManager::size.x)
	{
		if (int(mins.y) > 0 && int(mins.y) < GridManager::size.y)
		{
			if (int(maxes.x) > 0 && int(maxes.x) < GridManager::size.x)
			{
				if (int(maxes.y) > 0 && int(maxes.y) < GridManager::size.y)
				{
					for (int widthItr = int(mins.x); widthItr<int(maxes.x) + 1; widthItr++)
					{
						for (int heightItr = int(mins.y); heightItr<int(maxes.y) + 1; heightItr++)
						{
							test.push_back(&GridManager::tile_map[widthItr][heightItr]);
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

	// select units


	if (selection_group.size() > 0)
	{
		selection = selection_group.at(0);
		selection->play_audio_queue(SOUND_SELECT);
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
		if (camera_pos.z > 5)
		{
			camera_pos.z -= 0.5;
		}
	}

	if (input == MWHEELDOWN)
	{
		if (camera_pos.z < 100)
		{
			camera_pos.z += 0.5;
		}
	}
}

void FOWPlayer::take_input(SDL_Keycode input, bool key_down)
{
	camera_input(input, key_down);

	if (selection != nullptr)
	{
		for (auto selectionItr : selection_group)
		{
			if (selectionItr->team_id == 0)
			{
				selectionItr->take_input(input, key_down, queue_add_toggle);
			}
		}
	}

	if (keymap[ATTACK_MOVE_MODE] == input && key_down == true)
	{
		attack_move_mode = true;
	}

	if (keymap[FULLSCREEN] == input && key_down == true)
	{
		user_settings.toggleFullScreen();
	}
	if (keymap[TOGGLE_SOUND] == input && key_down == true)
	{
		user_settings.toggleSound();
		if (user_settings.use_sound)
		{
			AudioController::play_music();
		}
		else
		{
			AudioController::stop_music();
		}
	}
	if (input == LMOUSE && key_down == true)
	{
		t_vertex attack_move_position;
		// this was a minimap click
		if (Game::minimap->coords_in_ui())
		{
			attack_move_position = Game::minimap->get_click_position();
		}
		else
		{
			green_box->position = Game::raw_mouse_position;
			green_box->mouse_in_space = Game::real_mouse_position;
			green_box->visible = true;
			attack_move_position = Game::coord_mouse_position;
		}

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
						((FOWCharacter*)selectionItr)->give_command(FOWCommand(ATTACK_MOVE, attack_move_position));
					}
				}
			}
		}
	}

	if (input == LMOUSE && key_down == false && !UserInterface::mouse_focus())
	{
		// if the user interface absorbed the initial click, theres no greenbox to select
		if (green_box->visible)
		{
			green_box->visible = false;
			get_selection();
		}
	}
}