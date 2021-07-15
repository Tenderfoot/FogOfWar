
#include "leveleditor.h"
#include "game.h"
#include <stdio.h>
#include <math.h>
#include <iomanip>

void LevelEditor::take_input(boundinput input, bool keydown)
{
	keydown_map[input] = keydown;

	t_transform mouse_coords = Game::real_mouse_position;

	if (input == boundinput::LEFT && keydown)
	{
		selected_entities = Game::entities;
	}

	if (input == boundinput::ACTION && keydown)
	{
		GameEntity* game_entity = new GameEntity(0,0,5,5);
		game_entity->texture = PaintBrush::get_texture("data/greybrick.png", TEXTURE_REPEAT);
		game_entity->texture_coordinates.x = 0;
		game_entity->texture_coordinates.y = 0;
		game_entity->texture_coordinates.w = 1;
		game_entity->texture_coordinates.h = 1;
		game_entity->r = 1;
		game_entity->g = 1;
		game_entity->b = 1;
		game_entity->a = 1;
		game_entity->layer = 1;
		Game::entities.push_back(game_entity);
		selected_entities.push_back(game_entity);
	}

	if (input == boundinput::PAGE_UP)
	{
		//for (std::vector<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); ++it)
		//	((GameEntity*)(*it))->texture = (texture);
	}


	if (input == boundinput::SAVE)
	{
		save_level();
	}

	if (input == boundinput::ESCAPE)
		selected_entities.clear();

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
		GameEntity* selected_entity = nullptr;
		for (std::vector<Entity*>::iterator it = Game::entities.begin(); it != Game::entities.end(); ++it)
		{
			t_transform aabb = ((GameEntity*)(*it))->get_aabb();
			if (mouse_coords.x > aabb.x && mouse_coords.x < aabb.w && mouse_coords.y>aabb.y && mouse_coords.y < aabb.h)
			{
				selected_entity = (GameEntity*)*it;
				found = true;
			}
		}
		if (!found)
			selected_entities.clear();
		else
			selected_entities.push_back(selected_entity);
	}
}

bool LevelEditor::is_selected(Entity* test_entity)
{
	for (std::vector<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); ++it)
		if (test_entity == (*it))
			return true;
	
	return false;
}

void LevelEditor::save_level()
{
	nlohmann::json j =
	{
		{"name", "test"},
		{"entities", nlohmann::json({}) },
	};

	int i = 0;
	for (std::vector<Entity*>::iterator it = Game::entities.begin(); it != Game::entities.end(); ++it)
	{
		GameEntity* game_entity = ((GameEntity*)(*it));
		i++;
		if(game_entity->entity_type == GAME_ENTITY)
			j["entities"][std::to_string(i)] = nlohmann::json({ {"type", "GameEntity"}, 
															{"x", game_entity->transform.x}, 
															{"y", game_entity->transform.y}, 
															{"width", game_entity->transform.w}, 
															{"height", game_entity->transform.h}, 
															{"tex_x", game_entity->texture_coordinates.x},
															{"tex_y", game_entity->texture_coordinates.y},
															{"tex_width", game_entity->texture_coordinates.w},
															{"tex_height", game_entity->texture_coordinates.h},
															{"collision_enabled", game_entity->collision_enabled}, 
															{"layer", game_entity->layer}, 
															{"texture", "greybrick.png"},
															{"RGBA", nlohmann::json({{"R", game_entity->r},
																					{"G", game_entity->g},
																					{"B", game_entity->b},
																					{"A", game_entity->a}})},
			});
	}

	std::ofstream o("data/pretty.json");
	o << std::setw(4) << j << std::endl;

}

void LevelEditor::update()
{
	if (keydown_map[RMOUSE])
	{
		camera_transform.x -= Game::relative_mouse_position.x;
		camera_transform.y -= Game::relative_mouse_position.y;
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
			texture_scale.x += (Game::relative_mouse_position.x / 100);
			texture_scale.y += (Game::relative_mouse_position.y / 100);
		}
		
	}
}