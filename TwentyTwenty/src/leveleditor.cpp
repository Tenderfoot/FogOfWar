
#include "leveleditor.h"
#include "game.h"
#include <stdio.h>

void LevelEditor::take_input(boundinput input, bool keydown)
{
	t_transform mouse_coords = Game::real_mouse_position;

	if(input == boundinput::LMOUSE)
	{ 
		printf("mouse coords: %f, %f\n", mouse_coords.x, mouse_coords.y);
	}

	for (std::vector<Entity*>::iterator it = Game::entities.begin(); it != Game::entities.end(); ++it)
	{
		t_transform aabb = ((GameEntity*)(*it))->get_aabb();
		if (mouse_coords.x > aabb.x && mouse_coords.x < aabb.w && mouse_coords.y>aabb.y && mouse_coords.y < aabb.h)
		{
			selected_entities.push_back(*it);
		}
	}
}

bool LevelEditor::is_selected(Entity* test_entity)
{
	for (std::vector<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); ++it)
		if (test_entity == (*it))
			return true;
	
	return false;
}