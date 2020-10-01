
#include "leveleditor.h"
#include "game.h"
#include <stdio.h>
#include <math.h>
#include <cmath>

void LevelEditor::take_input(boundinput input, bool keydown)
{
	keydown_map[input] = keydown;

	t_transform mouse_coords = Game::real_mouse_position;

	if (input == boundinput::MWHEELDOWN && keydown)
	{
		camera_transform.w += 5;
	}
	if (input == boundinput::MWHEELUP && keydown)
	{
		camera_transform.w -= 5;
	}

	if(input == boundinput::LMOUSE && keydown)
	{ 
		printf("mouse coords: %f, %f\n", mouse_coords.x, mouse_coords.y);
		bool found = false;
		for (std::vector<Entity*>::iterator it = Game::entities.begin(); it != Game::entities.end(); ++it)
		{
			t_transform aabb = ((GameEntity*)(*it))->get_aabb();
			if (mouse_coords.x > aabb.x && mouse_coords.x < aabb.w && mouse_coords.y>aabb.y && mouse_coords.y < aabb.h)
			{
				selected_entities.push_back(*it);
				found = true;
			}
		}
		if (!found)
			selected_entities.clear();
	}
}

bool LevelEditor::is_selected(Entity* test_entity)
{
	for (std::vector<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); ++it)
		if (test_entity == (*it))
			return true;
	
	return false;
}

void LevelEditor::update()
{
	if (keydown_map[RMOUSE])
	{
		camera_transform.x += Game::relative_mouse_position.x;
		camera_transform.y += Game::relative_mouse_position.y;
	}
	if (keydown_map[SHIFT])
	{
		for (std::vector<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); ++it)
		{
			((GameEntity*)(*it))->transform.x += (Game::relative_mouse_position.x/10);
			((GameEntity*)(*it))->transform.y += (Game::relative_mouse_position.y/10);
		}
	}
	if (keydown_map[CTRL])
	{
		for (std::vector<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); ++it)
		{
			((GameEntity*)(*it))->transform.w += (Game::relative_mouse_position.x / 10);
			((GameEntity*)(*it))->transform.h += (Game::relative_mouse_position.y / 10);
		}
	}
	if (keydown_map[ALT])
	{
		if (selected_entities.size() > 0)
		{
			// so the plan is to get the bounds of all entities, then compute the coordinates per entity by 
			t_transform bounds;
			bounds.x = 999999;
			bounds.y = 999999;
			bounds.w = -999999;
			bounds.h = -999999;
			for (std::vector<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); ++it)
			{
				bounds.x = std::min(bounds.x, ((GameEntity*)(*it))->transform.x);
				bounds.y = std::min(bounds.y, ((GameEntity*)(*it))->transform.y);
				bounds.w = std::max(bounds.w, ((GameEntity*)(*it))->transform.w);
				bounds.h = std::max(bounds.h, ((GameEntity*)(*it))->transform.h);
			}

			printf("Bounds were: %f %f %f %f\n", bounds.x, bounds.y, bounds.w, bounds.h);
			for (std::vector<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); ++it)
			{
				GameEntity* the_entity = ((GameEntity*)(*it));
				t_transform aabb = the_entity->get_aabb();

				the_entity->texture_coordinates.x = (((aabb.x - bounds.x)) / (bounds.w - bounds.x)) * texture_scale.x;
				the_entity->texture_coordinates.y = (((aabb.y - bounds.y)) / (bounds.h - bounds.y)) * texture_scale.y;
				the_entity->texture_coordinates.w = (((aabb.w - bounds.x)) / (bounds.w - bounds.x)) * texture_scale.x;
				the_entity->texture_coordinates.h = (((aabb.h - bounds.y)) / (bounds.h - bounds.y)) * texture_scale.y;
			}
		}
		
		for (std::vector<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); ++it)
		{
			GameEntity* the_entity = ((GameEntity*)(*it));
			t_transform aabb = the_entity->get_aabb();
			texture_scale.x += (Game::relative_mouse_position.x / 10);
			texture_scale.y += (Game::relative_mouse_position.y / 10);
		}
		
	}
}