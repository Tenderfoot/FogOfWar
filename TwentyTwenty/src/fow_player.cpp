
#include "fow_character.h"
#include "fow_building.h"
#include "fow_player.h"
#include "audiocontroller.h"
#include "gatherer.h"

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

void FOWPlayer::get_selection()
{
	t_vertex maxes = t_vertex(std::max(green_box->x, green_box->width), std::max(-green_box->y, -green_box->height), 0.0f);
	t_vertex mins = t_vertex(std::min(green_box->x, green_box->width), std::min(-green_box->y, -green_box->height), 0.0f);

	// clear selected characters
	if (selection_group.size() > 0)
	{
		for (int i = 0; i < selection_group.size(); i++)
		{
			selection_group.at(i)->clear_selection();
		}
	}

	selection_group.clear();

	// if the box is valid, make a new selection group
	if (int(mins.x) > 0 && int(mins.x) < grid_manager->width)
		if (int(mins.y) > 0 && int(mins.y) < grid_manager->height)
			if (int(maxes.x) > 0 && int(maxes.x) < grid_manager->width)
				if (int(maxes.y) > 0 && int(maxes.y) < grid_manager->height)
				{
					for (int i = 0; i < grid_manager->entities->size(); i++)
					{
						Entity *test = grid_manager->entities->at(i);
						if (is_selectable(test->type))
						{
							if (test->position.x >= mins.x && test->position.y >= mins.y
								&& test->position.x <= maxes.x && test->position.y <= maxes.y)
							{
								selection_group.push_back((FOWSelectable*)test);
								((FOWSelectable*)test)->selected = true;
							}
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


// Most of this is passing input through to selected units
void FOWPlayer::take_input(boundinput input, bool type)
{
	camera_input(input, type);

	if (selection != nullptr)
		selection->take_input(input, type, queue_add_toggle);

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