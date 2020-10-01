
#include "leveleditor.h"
#include "game.h"
#include <stdio.h>

void LevelEditor::take_input(boundinput input, bool keydown)
{
	keydown_map[input] = keydown;

	t_transform mouse_coords = Game::real_mouse_position;

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
	if (keydown_map[MOVE_ENTITY])
	{
		for (std::vector<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); ++it)
		{
			((GameEntity*)(*it))->transform.x += (Game::relative_mouse_position.x/10);
			((GameEntity*)(*it))->transform.y += (Game::relative_mouse_position.y/10);
		}
	}
}