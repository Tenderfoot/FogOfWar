
#include "fow_building.h"
#include "fow_player.h"
#include "gatherer.h"
#include "knight.h"
#include "audiocontroller.h"
#include "server_handler.h"
#include "client_handler.h"
#include "user_interface.h"
#include "game.h"

UIProgressBar* FOWBuilding::progress_bar = nullptr;
std::map<entity_types, int> FOWBuilding::unit_cost = { {FOW_KNIGHT,600}, {FOW_ARCHER,600}, {FOW_GATHERER,400} };

FOWBuilding::FOWBuilding(int x, int y, int size)
{
	type = FOW_BUILDING;
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
	dirty_tile_map();
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
	if(ClientHandler::initialized)
	{
		under_construction = false;
	}
	else
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
}

// there seems to be a lot of !ClientHandler::initialized in here 
// but it shouldn't get here anyway if you're the client
void FOWBuilding::process_command(FOWCommand next_command)
{
	if (next_command.type == BUILD_UNIT)
	{
		if (this->type == FOW_ENEMYSPAWNER && currently_making_unit == false)
		{
			currently_making_unit = true;
			unit_start_time = SDL_GetTicks();
			entity_to_build = next_command.unit_type;
			return;
		}

		printf("Build Unit command recieved\n");
		std::vector<t_tile> tiles = get_adjacent_tiles(true);
		if (tiles.size() < 1)
		{
			printf("nowhere to put unit!");
		}
		else
		{
			if (currently_making_unit == false && under_construction == false)
			{
				bool can_make_unit = false;
				int* gold = nullptr;

				if (team_id == FOWPlayer::team_id && !ClientHandler::initialized)
				{
					gold = &FOWPlayer::gold;
				}
				else if (ServerHandler::initialized)
				{
					t_tracked_player* client = ServerHandler::get_client(team_id);
					gold = &client->gold;
				}

				if (*gold >= unit_cost[next_command.unit_type])
				{
					if (Game::get_used_supply_for_team(team_id) < Game::get_supply_for_team(team_id))
					{
						can_make_unit = true;
						*gold -= unit_cost[next_command.unit_type];
					}
					else
					{
						Game::send_error_message(std::string("Not enough supply! Build more farms!"), team_id);
					}
				}
				else
				{
					Game::send_error_message(std::string("Not enough gold! (").append(std::to_string(unit_cost[next_command.unit_type])).append(")"), team_id);
				}
				

				if (can_make_unit)
				{
					currently_making_unit = true;
					entity_to_build = next_command.unit_type;
					unit_start_time = SDL_GetTicks();
				}
			}
			else
			{
				Game::send_error_message("Already Building Unit or Under Construction", team_id);
			}
		}
	}
	FOWSelectable::process_command(next_command);
}

// I'm currently overwriting the selectable type with whether its keyup or keydown - not good
void FOWBuilding::take_input(SDL_Keycode input, bool type, bool queue_add_toggle)
{
	if (this->type == FOW_BARRACKS)
	{
		if (keymap[BUILD_FOOTMAN] == input && can_build_units && type == true)
		{
			if (ClientHandler::initialized)	// client doesn't have authority to do something like this, has to ask the server
			{
				FOWCommand build_unit_command = FOWCommand(BUILD_UNIT, FOW_KNIGHT);
				build_unit_command.self_ref = this;
				ClientHandler::command_queue.push_back(build_unit_command);
			}
			else
			{
				process_command(FOWCommand(BUILD_UNIT, FOW_KNIGHT));
			}
		}

		if (keymap[BUILD_ARCHER] == input && can_build_units && type == true)
		{
			if (ClientHandler::initialized)	// client doesn't have authority to do something like this, has to ask the server
			{
				FOWCommand build_unit_command = FOWCommand(BUILD_UNIT, FOW_ARCHER);
				build_unit_command.self_ref = this;
				ClientHandler::command_queue.push_back(build_unit_command);
			}
			else
			{
				process_command(FOWCommand(BUILD_UNIT, FOW_ARCHER));
			}
		}
	}

	if (this->type == FOW_TOWNHALL)
	{
		if (keymap[BUILD_GATHERER] == input && can_build_units && type == true)
		{
			if (ClientHandler::initialized)	// client doesn't have authority to do something like this, has to ask the server
			{
				FOWCommand build_unit_command = FOWCommand(BUILD_UNIT, FOW_GATHERER);
				build_unit_command.self_ref = this;
				ClientHandler::command_queue.push_back(build_unit_command);
			}
			else
			{
				process_command(FOWCommand(BUILD_UNIT, FOW_GATHERER));
			}
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

float FOWBuilding::get_depth()
{
	return 1 - ((draw_position.y + size/2)/GridManager::size.y);
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

void FOWBuilding::clear_selection()
{
	progress_bar->visible = false;
	FOWSelectable::clear_selection();
}

 void FOWBuilding::update(float time_delta)
{

	 if ((under_construction || currently_making_unit) && selected)
	 {
		 progress_bar->visible = true;
		 progress_bar->current = (SDL_GetTicks()) - (under_construction == true ? construction_start_time : unit_start_time);
		 progress_bar->maximum = under_construction == true ? time_to_build : time_to_build_unit;
	 }

	 if (under_construction)
	 {
		 if (SDL_GetTicks() - construction_start_time > time_to_build)
		 {
			 construction_finished();
		 }
	 }

	 if (currently_making_unit && !ClientHandler::initialized)
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
			 progress_bar->visible = false;
		 }
	 }

	 FOWSelectable::update(time_delta);
}

 void FOWEnemySpawner::update(float time_delta)
{
	 /*
	 if (SDL_GetTicks() - last_spawn > 5000 && (ServerHandler::initialized || (!ServerHandler::initialized && !ClientHandler::initialized)))
	 {
		 // find an empty tile
		 FOWCharacter* new_skeleton;
		 process_command(FOWCommand(BUILD_UNIT, FOW_SKELETON));
		 last_spawn = SDL_GetTicks();
	 }
	 */
	 FOWBuilding::update(time_delta);
}