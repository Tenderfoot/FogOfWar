
#include "fow_building.h"
#include "fow_player.h"
#include "gatherer.h"
#include "knight.h"
#include "audiocontroller.h"
#include "server_handler.h"
#include "client_handler.h"

FOWBuilding::FOWBuilding(int x, int y, int size)
{
	type = FOW_BUILDING;
	can_build_units = false;
	skeleton_name = "buildings";
	currently_making_unit = false;

	this->position.x = (float)x;
	this->position.y = (float)y;
	this->size = size;
	color = t_vertex(1, 1, 1);

	maximum_hp = 60;
	current_hp = maximum_hp;
	draw_position = position;
	draw_offset = t_vertex(-0.5f, +0.5f, 0);
	destroyed = false;
}

t_transform FOWBuilding::get_aabb()
{
	t_transform aabb;
	float x1 = position.x + 0.5;
	float y1 = position.y - 0.5;
	float x2 = position.x - 0.5 + size;
	float y2 = position.y + size - 0.5;

	aabb.x = std::min(x1, x2);
	aabb.w = std::max(x1, x2);
	aabb.y = std::min(y1, y2);
	aabb.h = std::max(y1, y2);

	return aabb;
}

void FOWBuilding::construction_finished()
{
	// TODO: This class specific code shouldn't be in the parent
	skin_name = base_skin;
	reset_skin();

	AudioController::play_sound("data/sounds/workcomplete.ogg");
	under_construction = false;

	std::vector<t_tile> tiles = get_adjacent_tiles(true);
	t_vertex new_position = t_vertex(tiles[0].x, tiles[0].y, 0.0f);
	((FOWCharacter*)builder)->hard_set_position(new_position);
	builder->visible = true;
}

void FOWBuilding::process_command(FOWCommand next_command)
{
	if (next_command.type == BUILD_UNIT)
	{
		printf("Build Unit command recieved\n");
		std::vector<t_tile> tiles = get_adjacent_tiles(true);
		if (tiles.size() < 1)
		{
			printf("nowhere to put unit!");
		}
		else
		{
			if (currently_making_unit == false)
			{
				bool can_make_unit = false;
				if (team_id == FOWPlayer::team_id && !ClientHandler::initialized)
				{
					if (FOWPlayer::gold > 0)
					{
						can_make_unit = true;
						FOWPlayer::gold--;
					}
				}

				if (ServerHandler::initialized && team_id == ServerHandler::client.team_id)
				{
					if (ServerHandler::client.gold > 0)
					{
						can_make_unit = true;
						ServerHandler::client.gold--;
					}
				}

				if (can_make_unit)
				{
					currently_making_unit = true;
					unit_start_time = SDL_GetTicks();
				}
			}
			else
			{
				printf("already building unit!");
			}
		}
	}
	FOWSelectable::process_command(next_command);
}

void FOWBuilding::take_input(SDL_Keycode input, bool type, bool queue_add_toggle)
{
	if (keymap[ACTION] == input && can_build_units && type == true)
	{
		if (ClientHandler::initialized)	// client doesn't have authority to do something like this, has to ask the server
		{
			FOWCommand build_unit_command = FOWCommand(BUILD_UNIT, entity_to_build);
			build_unit_command.self_ref = this;
			ClientHandler::command_queue.push_back(build_unit_command);
		}
		else
		{
			process_command(FOWCommand(BUILD_UNIT, entity_to_build));
		}
	}
}

void FOWBuilding::char_init()
{
	animationState->addAnimation(0, "animation", true, 0);
}

void FOWBuilding::set_under_construction()
{
	// just in case it hasn't been drawn yet
	if (spine_initialized == false)
	{
		build_spine();
	}

	under_construction = true;
	construction_start_time = SDL_GetTicks();
	skin_name = skin_name.append("_UC");
	reset_skin();
}

void FOWBuilding::take_damage(int amount)
{
	current_hp -= amount;

	if (current_hp < 0)
	{
		destroyed = true;

		int widthItr = 0, heightItr = 0;

		for (widthItr = position.x; widthItr < position.x + (size); widthItr++)
		{
			for (heightItr = position.y; heightItr < position.y + (size); heightItr++)
			{
				GridManager::tile_map[widthItr][heightItr].entity_on_position = nullptr;
			}
		}

		visible = false;
	}
}

 void FOWBuilding::update(float time_delta)
{
	 if (under_construction)
	 {
		 if (SDL_GetTicks() - construction_start_time > time_to_build)
		 {
			 construction_finished();
		 }
	 }

	 if (currently_making_unit)
	 {
		 if (SDL_GetTicks() - unit_start_time > time_to_build_unit)
		 {
			 std::vector<t_tile> tiles = get_adjacent_tiles(true);
			 if (tiles.size() < 1)
			 {
				 printf("nowhere to put unit!");
			 }
			 else
			 {
				 t_vertex new_unit_position = t_vertex(tiles[0].x, tiles[0].y, 0);
				 last_built_unit = ((FOWCharacter*)GridManager::build_and_add_entity(entity_to_build, new_unit_position));
				 last_built_unit->team_id = team_id;
				 if (team_id == FOWPlayer::team_id)
				 {
					 ((FOWSelectable*)last_built_unit)->play_audio_queue(SOUND_READY);
				 }
			 }
			 currently_making_unit = false;
		 }
	 }
}

 void FOWEnemySpawner::update(float time_delta)
{
	 //if (SDL_GetTicks() - last_spawn > 5000 && (ServerHandler::initialized || (!ServerHandler::initialized && !ClientHandler::initialized)))
	 //{	
		// // find an empty tile
		// auto adjacent_tiles = get_adjacent_tiles(true);
		// if (adjacent_tiles.size() > 0)
		// {
		//	 FOWCharacter* new_skeleton;

		//	 // this is kind of hacky but also reduces repeated code so...
		//	 process_command(FOWCommand(BUILD_UNIT, FOW_SKELETON));
		//	 auto town_halls = GridManager::get_entities_of_type(FOW_TOWNHALL);
		//	 if (town_halls.size() > 0)
		//	 {
		//		last_built_unit->give_command(FOWCommand(ATTACK_MOVE, t_vertex(town_halls[0]->position.x+1, town_halls[0]->position.y-1, 0)));
		//	 }
		// }
		// last_spawn = SDL_GetTicks();
	 //}
	 
}