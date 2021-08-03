
#include "fow_character.h"
#include "fow_building.h"
#include "fow_player.h"
#include "audiocontroller.h"
#include "gatherer.h"
#include <algorithm>

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
}

void FOWPlayer::update()
{
	green_box->width = grid_manager->real_mouse_position.x;
	green_box->height = grid_manager->real_mouse_position.y;
}

std::vector<t_tile*> FOWPlayer::GetTiles()
{
	t_vertex maxes = t_vertex(std::max(green_box->x, green_box->width+1), std::max(-green_box->y, -green_box->height), 0.0f);
	t_vertex mins = t_vertex(std::min(green_box->x, green_box->width+1), std::min(-green_box->y, -green_box->height), 0.0f);

	std::vector<t_tile*> test;

	if (int(mins.x) > 0 && int(mins.x) < grid_manager->width)
		if (int(mins.y) > 0 && int(mins.y) < grid_manager->height)
			if (int(maxes.x) > 0 && int(maxes.x) < grid_manager->width)
				if (int(maxes.y) > 0 && int(maxes.y) < grid_manager->height)
				{
					int i, j;
					for (i = int(mins.x); i<int(maxes.x)+1; i++)
						for (j = int(mins.y); j<int(maxes.y)+1; j++)
							test.push_back(&grid_manager->tile_map[i][j]);
				}
	return test;
}

void FOWPlayer::get_selection()
{
	t_vertex maxes = t_vertex(std::max(green_box->x, green_box->width), std::max(-green_box->y, -green_box->height), 0.0f);
	t_vertex mins = t_vertex(std::min(green_box->x, green_box->width), std::min(-green_box->y, -green_box->height), 0.0f);

	// clear selected characters
	if (selection_group.size() > 0)
		for (int i = 0; i < selection_group.size(); i++)
			selection_group.at(i)->clear_selection();

	selection_group.clear();

	std::vector<t_tile*> test = GetTiles();
	for (int i = 0; i < test.size(); i++)
	{
		if (test[i]->entity_on_position != nullptr)
		{
			if (std::find(selection_group.begin(), selection_group.end(), test[i]->entity_on_position) == selection_group.end()) 
			{
				selection_group.push_back((FOWSelectable*)test[i]->entity_on_position);
				((FOWSelectable*)test[i]->entity_on_position)->selected = true;
			}
		}
	}

	if (selection_group.size() > 0)
		selection = selection_group.at(0);
	else
		selection = nullptr;
}


void FOWPlayer::camera_input(boundinput input, bool type)
{
	if (input == RIGHT && type == true)
	{
		camera_pos.x++;
	}

	if (input == LEFT && type == true)
	{
		camera_pos.x--;
	}

	if (input == UP && type == true)
	{
		camera_pos.y++;
	}

	if (input == DOWN && type == true)
	{
		camera_pos.y--;
	}

	if (input == MWHEELUP)
	{
		if (camera_pos.w > 5)
			camera_pos.w -= 0.5;
	}

	if (input == MWHEELDOWN)
	{
		if (camera_pos.w < 100)
			camera_pos.w += 0.5;
	}
}

void FOWPlayer::take_input(boundinput input, bool type)
{
	camera_input(input, type);

	if (selection != nullptr)
		for(int i=0; i<selection_group.size(); i++)
			selection_group.at(i)->take_input(input, type, queue_add_toggle);

	if (input == LMOUSE && type == true)
	{
		green_box->x = grid_manager->real_mouse_position.x;
		green_box->y = grid_manager->real_mouse_position.y;
		green_box->mouse_in_space = grid_manager->real_mouse_position;
		green_box->visible = true;
	}

	if (input == LMOUSE && type == false)
	{
		green_box->visible = false;
		get_selection();

		/*if (selection_group.size() > 0)
			char_widget->character = new_player->selection_group.selected_characters.at(0);
		else
			char_widget->character = nullptr;*/ 
	}
}