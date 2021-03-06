
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
int FOWPlayer::wood;
int FOWPlayer::team_id=0;
float FOWPlayer::last_poor_warning;
bool FOWPlayer::attack_move_mode;
bool FOWPlayer::queue_add_toggle;
int FOWPlayer::current_tex;

float FOWPlayer::camera_distance;
std::vector<FOWSelectable*> FOWPlayer::selection_group;
GreenBox* FOWPlayer::green_box;
FOWSelectable* FOWPlayer::selection;
bool FOWPlayer::move_camera_left;
bool FOWPlayer::move_camera_right;
bool FOWPlayer::move_camera_up;
bool FOWPlayer::move_camera_down;

extern bool is_unit(entity_types type);
extern bool is_building(entity_types type);

void FOWPlayer::init()
{
	queue_add_toggle = false;
	gold = 2000;
	wood = 1000;
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

bool check_collision(t_transform aabb1, t_transform aabb2)
{
	if (aabb1.w > aabb2.x && aabb1.x < aabb2.w && aabb1.h > aabb2.y && aabb1.y < aabb2.h)
		return true;

	return false;
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

	t_vertex position = green_box->mouse_in_space;
	t_vertex size = Game::real_mouse_position;
	t_vertex maxes = t_vertex(std::max(position.x, size.x + 1), std::max(-position.y, -size.y), 0.0f);
	t_vertex mins = t_vertex(std::min(position.x, size.x + 1), std::min(-position.y, -size.y), 0.0f);
	t_transform greenbox_aabb(mins.x,mins.y,maxes.x,maxes.y);
	
	for (auto entity : Game::entities)
	{
		if (is_unit(entity->type) || is_building(entity->type))
		{
			if (entity->visible == true && ((FOWCharacter*)entity)->state != GRID_DYING)
			{
				t_transform aabb2 = entity->get_aabb();
				if (check_collision(greenbox_aabb, aabb2))
				{
					FOWSelectable* selected_entity = (FOWSelectable*)entity;
					selected_entity->select_unit();
					selection_group.push_back(selected_entity);
				}
			}
		}
	}


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
		if (camera_pos.z < 35)
		{
			camera_pos.z += 0.5;
		}
	}
}

int FOWPlayer::get_supply()
{
	return Game::get_supply_for_team(team_id);
}

int FOWPlayer::get_used_supply()
{
	return Game::get_used_supply_for_team(team_id);
}

bool FOWPlayer::supply_available()
{
	return get_used_supply() < get_supply();
}

void FOWPlayer::take_input(SDL_Keycode input, bool key_down)
{
	camera_input(input, key_down);

	if (selection != nullptr)
	{
		for (auto selectionItr : selection_group)
		{
			// Clienthandler TEAMID thing
			if (selectionItr->team_id == FOWPlayer::team_id)
			{
				selectionItr->take_input(input, key_down, queue_add_toggle);
			}
		}
	}

	if (keymap[ESCAPE] == input && key_down == true)
	{
		for (auto unit : selection_group)
		{
			unit->clear_selection();
		}
		selection_group.clear();
	}

	if (keymap[PAGE_UP] == input && key_down == true)
	{
		current_tex++;
		GridManager::new_vbo.texture = GridManager::tile_atlas.at(current_tex%GridManager::tile_atlas.size());
	}
	if (keymap[PAGE_DOWN] == input && key_down == true)
	{
		current_tex--;
		if (current_tex < 0)
		{
			current_tex = GridManager::tile_atlas.size()-1;
		}
		GridManager::new_vbo.texture = GridManager::tile_atlas.at(current_tex % GridManager::tile_atlas.size());
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
				if (selectionItr->team_id == FOWPlayer::team_id)
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