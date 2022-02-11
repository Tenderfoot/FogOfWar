#include "gatherer.h"
#include "fow_building.h"
#include "fow_player.h"
#include "audiocontroller.h"
#include "game.h"
#include "client_handler.h"
#include "server_handler.h"
#include "fow_decoration.h"
#include "user_interface.h"

FOWGatherer::FOWGatherer()
{
	type = FOW_GATHERER;
	skin_name = "farm";
	attack_type = ATTACK_MELEE;

	// other stuff gatherer needs
	has_gold = false;
	target_town_hall = nullptr;
	target_mine = nullptr;
	build_mode = false;
	has_trees = false;
	size = 1;

	// to_build gets its skin changed to all the different buildings
	// when the gatherer is ghosting a building to build. (like the player is going to get them to build)
	// this is per gatherer right now and could probably be moved to the player

	// this is okay now because it doesn't make spine stuff until it draws it
	to_build = new FOWTownHall(t_vertex(0,0,0));

	// audio
	ready_sounds.push_back("data/sounds/worker_sounds/Psready.wav");
	select_sounds.push_back("data/sounds/worker_sounds/Pswhat1.wav");
	select_sounds.push_back("data/sounds/worker_sounds/Pswhat2.wav");
	select_sounds.push_back("data/sounds/worker_sounds/Pswhat3.wav");
	select_sounds.push_back("data/sounds/worker_sounds/Pswhat4.wav");
	command_sounds.push_back("data/sounds/worker_sounds/Psyessr1.wav");
	command_sounds.push_back("data/sounds/worker_sounds/Psyessr2.wav");
	command_sounds.push_back("data/sounds/worker_sounds/Psyessr3.wav");
	command_sounds.push_back("data/sounds/worker_sounds/Psyessr4.wav");
	death_sounds.push_back("data/sounds/death.wav");
	chop_sounds.push_back("data/sounds/worker_sounds/Tree1.wav");
	chop_sounds.push_back("data/sounds/worker_sounds/Tree2.wav");
	chop_sounds.push_back("data/sounds/worker_sounds/Tree3.wav");
	chop_sounds.push_back("data/sounds/worker_sounds/Tree4.wav");

	// there must be a better way to do this
	building_costs[FOW_TOWNHALL] = t_building_cost();
	building_costs[FOW_TOWNHALL].gold_cost = 1200;
	building_costs[FOW_TOWNHALL].wood_cost = 800;
	building_costs[FOW_FARM] = t_building_cost();
	building_costs[FOW_FARM].gold_cost = 500;
	building_costs[FOW_FARM].wood_cost = 250;
	building_costs[FOW_BARRACKS] = t_building_cost();
	building_costs[FOW_BARRACKS].gold_cost = 700;
	building_costs[FOW_BARRACKS].wood_cost = 450;

}

FOWGatherer::FOWGatherer(t_vertex initial_position) : FOWGatherer::FOWGatherer()
{
	set_position(initial_position);
}

void FOWGatherer::draw()
{
	if (to_build->spine_initialized == false)
	{
		to_build->build_spine();
	}

	if (build_mode)
	{
		if (building_type == FOW_TOWNHALL)
		{
			to_build->skin_name = "TownHall";
			to_build->size = 4;
		}
		else if (building_type == FOW_FARM)
		{
			to_build->skin_name = "Farm";
			to_build->size = 2;
		}
		else if (building_type == FOW_BARRACKS)
		{
			to_build->skin_name = "Barracks";
			to_build->size = 3;
		}

		// this is awful because its creating a new VBO every frame, needs fix
		// only needs to happen when building type changes I guess
		to_build->reset_skin();
		SpineManager::reset_vbo(to_build->skeleton, &to_build->VBO);

		good_spot = GridManager::space_free(Game::coord_mouse_position, to_build->size) == true ? true : false;

		to_build->position = Game::coord_mouse_position;
		to_build->draw_position = Game::coord_mouse_position;
		to_build->draw();
	}

	FOWCharacter::draw();
}

// "Collecting" is when a gatherer is off screen - in a goldmine getting gold, or in a town hall dropping it off
// during this state they are invisible and non-interactable
void FOWGatherer::set_collecting(t_vertex new_position)
{
	GridManager::tile_map[position.x][position.y].entity_on_position = nullptr;
	position = new_position;
	draw_position = new_position;
	visible = false;
	state = GRID_COLLECTING;
	collecting_time = SDL_GetTicks();
}

void FOWGatherer::char_init()
{
	animationState->addAnimation(0, "idle_two", true, 0);
	animationState->setListener(this);
}

void FOWGatherer::callback(spine::AnimationState* state, spine::EventType type, spine::TrackEntry* entry, spine::Event* event)
{
	// Inspect and respond to the event here.
	if (type == spine::EventType_Event)
	{
		// spine has its own string class that doesn't work with std::string
		// on second thought this should probably be .compare() == 0
		if (std::string(event->getData().getName().buffer()) == std::string("attack_event"))
		{
			AudioController::play_sound(chop_sounds.at(rand() % chop_sounds.size()));
		}
	}
}

void FOWGatherer::set_chopping(t_vertex tree_position)
{
	state = GRID_CHOPPING;
	current_tree = tree_position;
	std::string attack_prefix = "attack";
	add_to_skin("axe");

	if (tree_position.x > position.x || tree_position.x < position.x)
		if (tree_position.y < position.y)
			animationState->setAnimation(0, std::string(attack_prefix + "_sideup").c_str(), false);
		else if (tree_position.y > position.y)
			animationState->setAnimation(0, std::string(attack_prefix + "_sidedown").c_str(), false);
		else
			animationState->setAnimation(0, std::string(attack_prefix + "_side").c_str(), false);
	else
		if (tree_position.y < position.y)
			animationState->setAnimation(0, std::string(attack_prefix + "_up").c_str(), false);
		else
			animationState->setAnimation(0, std::string(attack_prefix + "_down").c_str(), false);

	// hack for flip
	desired_position.x = tree_position.x;
}

// theres a repeated code pattern happening in this method but I don't have the brain energy to fix it right now

void FOWGatherer::OnReachDestination()
{
	if (current_command.type == GATHER)
	{
		// if we're gathering and we've reached our destination we're either at a gold mine or a town hall
		if (has_gold == false)
		{
			set_collecting(target_mine->position);
		}
		else
		{
			set_collecting(get_entity_of_entity_type(FOW_TOWNHALL, team_id)->position);

			if (!ClientHandler::initialized && FOWPlayer::team_id == team_id)
			{
				FOWPlayer::gold+=100;
			}
			if (ServerHandler::initialized && ServerHandler::client.team_id == team_id)
			{
				ServerHandler::client.gold+=100;
			}
		}
	}

	if (current_command.type == CHOP)
	{
		if (has_trees == false)
		{
			auto tiles = get_adjacent_tiles(false, true);
			bool found = false;
			for (auto tile : tiles)
			{
				if (tile.type == TILE_TREES && tile.wall == 1)
				{
					set_chopping(t_vertex(tile.x, tile.y, 0.0f));
					chop_start_time = SDL_GetTicks();
					found = true;
				}
			}
			if (!found)
			{
				set_idle();
			}
		}
		else
		{
			set_collecting(get_entity_of_entity_type(FOW_TOWNHALL, team_id)->position);

			if (!ClientHandler::initialized && FOWPlayer::team_id == team_id)
			{
				FOWPlayer::wood+=100;
			}
			if (ServerHandler::initialized && ServerHandler::client.team_id == team_id)
			{
				ServerHandler::client.wood+=100;
			}
		}
	}

	if (current_command.type == BUILD_BUILDING)
	{
		bool can_build = true;

		// As long as we're not the client, and its us, check if we can build using FOWPlayer
		if (!ClientHandler::initialized && FOWPlayer::team_id == team_id)
		{
			can_build = (FOWPlayer::gold >= building_costs[building_type].gold_cost) && (FOWPlayer::wood >= building_costs[building_type].wood_cost);
			if (!can_build)
			{	
				if (!(FOWPlayer::gold >= building_costs[building_type].gold_cost))
				{
					Game::new_error_message->set_message("Not enough gold! Mine more gold!");
				}
				else
				{
					Game::new_error_message->set_message("Not enough wood! Chop more trees!");
				}
			}
		}

		// If we are the server, and its NOT us, then check the corrosponding client
		if (ServerHandler::initialized && ServerHandler::client.team_id == team_id)
		{
			can_build = (ServerHandler::client.gold >= building_costs[building_type].gold_cost) && (ServerHandler::client.wood >= building_costs[building_type].wood_cost);
		}

		if (can_build)
		{
			if (!ClientHandler::initialized && FOWPlayer::team_id == team_id)
			{
				FOWPlayer::gold -=building_costs[building_type].gold_cost;
				FOWPlayer::wood -= building_costs[building_type].wood_cost;
			}
			if (ServerHandler::initialized && ServerHandler::client.team_id == team_id)
			{
				ServerHandler::client.gold -= building_costs[building_type].gold_cost;
				ServerHandler::client.wood -= building_costs[building_type].wood_cost;
			}

			FOWBuilding* new_building = nullptr;
			new_building = ((FOWBuilding*)GridManager::build_and_add_entity(building_type, current_command.position));
			new_building->set_under_construction();
			new_building->builder = this;
			new_building->team_id = team_id;
			AudioController::play_sound("data/sounds/under_construction.ogg");

			for (int i = 0; i < new_building->size*new_building->size; i++)
			{
				t_tile* new_tile = &GridManager::tile_map[current_command.position.x+(i%new_building->size)][current_command.position.y+((int)(i/new_building->size))];
				for (auto decoration : new_tile->decorations)
				{
					((FOWDecoration*)decoration)->delete_decoration();
				}
			}

			visible = false;
			set_idle();
		}
		else
		{
			AudioController::play_sound("data/sounds/building_sounds/Mine.wav");
			set_idle();
		}
	}

	FOWCharacter::OnReachDestination();
}

void FOWGatherer::process_command(FOWCommand next_command)
{
	current_command = next_command;

	if (next_command.type == GATHER)
	{
		blocked_retry_count = 0;	// if they made it home they aren't blocked anymore
		if (has_gold)
		{
			set_moving(get_entity_of_entity_type(FOW_TOWNHALL, team_id));
		}
		else
		{
			target_mine = next_command.target;
			set_moving(next_command.target);
		}
	}

	if (next_command.type == BUILD_BUILDING)
	{
		set_moving(next_command.position);
	}

	if (next_command.type == CHOP)
	{
		if (has_trees == false)
		{
			// if beside the tile, start chopping
			if (abs(position.x - current_command.position.x) < 2 && abs(position.y - current_command.position.y) < 2)
			{
				set_chopping(current_command.position);
				chop_start_time = SDL_GetTicks();
			}
			else
			{
				auto tiles = GridManager::get_adjacent_tiles_from_position(t_vertex(current_command.position.x, current_command.position.y, 0.0f), true, false);
				if (tiles.size() > 0)
				{
					set_moving(t_vertex(tiles[0].x, tiles[0].y, 0.0f));
				}
				else
				{
					set_idle();
				}
			}
		}
		else
		{
			set_moving(get_entity_of_entity_type(FOW_TOWNHALL, team_id));
		}
	}

	FOWCharacter::process_command(next_command);
}

void FOWGatherer::take_input(SDL_Keycode input, bool type, bool queue_add_toggle)
{
	queue_add_toggle = false;
	t_vertex hit_position = Game::coord_mouse_position;
	FOWSelectable* hit_target = get_hit_target();

	if (input == LMOUSE && type == false)
	{
		if (build_mode && good_spot)
		{
			give_command(FOWCommand(BUILD_BUILDING, Game::coord_mouse_position));
		}
	}

	if (keymap[ESCAPE] == input && type == true)
	{
		build_mode = false;
	}

	if (keymap[BUILD_BARRACKS] == input && type == true)
	{
		build_mode = true;
		building_type = FOW_BARRACKS;
	}

	if (keymap[BUILD_TOWNHALL] == input && type == true)
	{
		build_mode = true;
		building_type = FOW_TOWNHALL;
	}

	if (keymap[BUILD_FARM] == input && type == true)
	{
		build_mode = true;
		building_type = FOW_FARM;
	}

	if (input == RMOUSE && type == true)
	{
		if (hit_target != nullptr)
		{
			if (hit_target->type == FOW_GOLDMINE && has_gold == false)
			{
				give_command(FOWCommand(GATHER, hit_target));
				return;
			}
			else if (hit_target->type == FOW_TOWNHALL)
			{
				if (has_gold == true)
				{
					give_command(FOWCommand(GATHER, hit_target));
					return;
				}
				if (has_trees == true)
				{
					give_command(FOWCommand(CHOP, hit_target));
					return;
				}
			}
		}

		if(GridManager::tile_map[hit_position.x][hit_position.y].type == TILE_TREES && GridManager::tile_map[hit_position.x][hit_position.y].wall == 1)
		{ 
			give_command(FOWCommand(CHOP, hit_position));
			return;
		}

	}

	FOWCharacter::take_input(input, type, queue_add_toggle);
}

void FOWGatherer::make_new_path()
{
	if (current_command.type == GATHER)
	{
		GameEntity *entity_on_pos = GridManager::tile_map[desired_position.x][desired_position.y].entity_on_position;
		if (entity_on_pos != nullptr && entity_on_pos != this)
		{
			if (has_gold)
			{
				find_path_to_target(get_entity_of_entity_type(FOW_TOWNHALL, team_id));
			}
			else
			{
				find_path_to_target(target_mine);
			}
		}
		else
		{
			current_path = GridManager::find_path(position, desired_position);
		}
	}
	else
	{
		FOWCharacter::make_new_path();
	}
}



FOWSelectable* FOWGatherer::get_entity_of_entity_type(entity_types type, int team_id)
{
	std::vector<GameEntity*> building_type_list = GridManager::get_entities_of_type(type);
	FOWSelectable *building = nullptr;

	for (auto building : building_type_list)
	{
		if (building->type == type)
		{
			if (team_id == -1)
				target_town_hall = ((FOWSelectable*)building);
			else
				if (((FOWSelectable*)building)->team_id == team_id)
					target_town_hall = ((FOWSelectable*)building);
		}
	}
	return target_town_hall;
}

void FOWGatherer::update(float time_delta)
{
	FOWSelectable* old_building = nullptr;
	FOWSelectable* new_building = nullptr;

	// Client doesn't do anything
	if (state == GRID_COLLECTING && !ClientHandler::initialized)
	{
		// done dropping off or collecting
		if (SDL_GetTicks() - collecting_time > 1000)
		{
			if (current_command.type == GATHER)
			{
				visible = true;
				if (has_gold == false)
				{
					has_gold = true;
					add_to_skin("moneybag");

					old_building = target_mine;
					std::vector<t_tile> tiles = old_building->get_adjacent_tiles(true);
					t_vertex new_position = t_vertex(tiles[0].x, tiles[0].y, 0);
					hard_set_position(new_position);

					new_building = get_entity_of_entity_type(FOW_TOWNHALL, team_id);
					if (new_building != nullptr)
					{
						set_moving(new_building);
					}
					else
					{
						set_idle();
					}
				}
				else
				{
					has_gold = false;
					reset_skin();

					old_building = get_entity_of_entity_type(FOW_TOWNHALL, team_id);
					std::vector<t_tile> tiles = old_building->get_adjacent_tiles(true);
					if (tiles.size() > 0)
					{
						t_vertex new_position = t_vertex(tiles[0].x, tiles[0].y, 0);
						hard_set_position(new_position);

						new_building = target_mine;
						if (new_building != nullptr)
						{
							set_moving(new_building);
						}
						else
						{
							set_idle();
						}
					}
					else
					{
						set_idle();
					}
				}
			}
			if (current_command.type == CHOP)
			{
				if (has_trees == true)
				{
					has_trees = false;
					reset_skin();
					visible = true;

					old_building = get_entity_of_entity_type(FOW_TOWNHALL, team_id);
					std::vector<t_tile> tiles = old_building->get_adjacent_tiles(true);
					if (tiles.size() > 0)
					{
						t_vertex new_position = t_vertex(tiles[0].x, tiles[0].y, 0);
						hard_set_position(new_position);

						// if you're holding wood and select a town hall,
						// this lets you turn it in without it crashing but sets you idle after
						if (current_command.target == nullptr)
						{
							set_moving(current_command.position);
						}
						else
						{
							set_idle();
						}
					}
					else
					{
						set_idle();
					}
				}
			}
		}
	}

	if (state == GRID_CHOPPING)
	{
		if (animationState->getCurrent(0)->isComplete())
		{
			// Client doesn't do anything
			// this is following the same pattern as attack
			if (ClientHandler::initialized)
			{
				set_chopping(current_tree);
			}
			else
			{
				// done dropping off or collecting
				if (SDL_GetTicks() - chop_start_time > 25000)
				{
					reset_skin();
					has_trees = true;
					add_to_skin("tree");
					t_tile* new_tile = &GridManager::tile_map[current_tree.x][current_tree.y];
					new_tile->type = TILE_GRASS;
					new_tile->wall = 0;
					GridManager::mow(current_tree.x, current_tree.y);
					GridManager::cull_orphans();
					GridManager::calc_all_tiles();
					new_building = get_entity_of_entity_type(FOW_TOWNHALL, team_id);
					if (new_building != nullptr)
					{
						set_moving(new_building);
					}
					else
					{
						set_idle();
					}
				}
				else
				{
					if (!(current_command == command_queue.at(0)))
					{
						reset_skin();
						process_command(command_queue.at(0));
					}
					else
						set_chopping(current_tree);
				}
			}
		}
	}

	FOWCharacter::update(time_delta);
}