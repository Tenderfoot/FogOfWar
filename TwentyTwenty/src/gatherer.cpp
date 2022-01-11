#include "gatherer.h"
#include "fow_building.h"
#include "fow_player.h"
#include "audiocontroller.h"
#include "game.h"

FOWGatherer::FOWGatherer()
{
	type = FOW_GATHERER;
	skin_name = "farm";

	// other stuff gatherer needs
	has_gold = false;
	target_town_hall = nullptr;
	target_mine = nullptr;
	build_mode = false;

	// to_build gets its skin changed to all the different buildings
	// when the gatherer is ghosting a building to build. (like the player is going to get them to build)
	// this is per gatherer right now and could probably be moved to the player
	to_build = new FOWTownHall(0,0);
	
	char_init();
}

FOWGatherer::FOWGatherer(t_vertex initial_position) : FOWGatherer::FOWGatherer()
{
	set_position(initial_position);
}

void FOWGatherer::draw()
{
	if (build_mode)
	{
		// this is awful because its creating a new VBO every frame, needs fix
		// only needs to happen when building type changes I guess
		if (building_type == FOW_TOWNHALL)
		{
			to_build->skin_name = "TownHall";
			to_build->reset_skin();
		}
		else if (building_type == FOW_FARM)
		{
			to_build->skin_name = "Farm";
			to_build->reset_skin();
		}
		else if (building_type == FOW_BARRACKS)
		{
			to_build->skin_name = "Barracks";
			to_build->reset_skin();
		}

		good_spot = grid_manager->space_free(Game::coord_mouse_position, 3) == true ? true : false;

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
	grid_manager->tile_map[position.x][position.y].entity_on_position = nullptr;
	position = new_position;
	draw_position = new_position;
	visible = false;
	state = GRID_COLLECTING;
	collecting_time = SDL_GetTicks();
}

void FOWGatherer::OnReachDestination()
{
	if (current_command.type == GATHER)
	{
		// if we're gathering and we've reached our destination we're either at a gold mine or a town hall
		if (has_gold == false)
		{
			set_collecting(get_entity_of_entity_type(FOW_GOLDMINE)->position);
		}
		else
		{
			set_collecting(get_entity_of_entity_type(FOW_TOWNHALL)->position);
			owner->gold++;
		}
	}

	if (current_command.type == BUILD_BUILDING)
	{
		FOWBuilding* new_building = nullptr;
		new_building = ((FOWBuilding*)grid_manager->build_and_add_entity(building_type, current_command.position));
		new_building->set_under_construction();
		new_building->builder = this;
		AudioController::play_sound("data/sounds/under_construction.ogg");
		visible = false;
		set_idle();
	}

	FOWCharacter::OnReachDestination();
}

void FOWGatherer::process_command(FOWCommand next_command)
{
	current_command = next_command;

	if (next_command.type == GATHER)
	{
		if (has_gold)
		{
			set_moving(get_entity_of_entity_type(FOW_TOWNHALL));
		}
		else
		{
			set_moving(next_command.target);
		}
	}

	if (next_command.type == BUILD_BUILDING)
	{
		set_moving(next_command.position);
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
			else if (hit_target->type == FOW_TOWNHALL && has_gold == true)
			{
				give_command(FOWCommand(GATHER, hit_target));
				return;
			}
		}
	}

	FOWCharacter::take_input(input, type, queue_add_toggle);
}

void FOWGatherer::make_new_path()
{
	if (current_command.type == GATHER)
	{
		GameEntity *entity_on_pos = grid_manager->tile_map[desired_position.x][desired_position.y].entity_on_position;
		if (entity_on_pos != nullptr && entity_on_pos != this)
		{
			printf("in this\n");
			if (has_gold)
			{
				find_path_to_target(get_entity_of_entity_type(FOW_TOWNHALL));
			}
			else
			{
				find_path_to_target(get_entity_of_entity_type(FOW_GOLDMINE));
			}
		}
		else
		{
			current_path = grid_manager->find_path(position, desired_position);
		}
	}
	else
	{
		FOWCharacter::make_new_path();
	}
}

FOWSelectable* FOWGatherer::get_entity_of_entity_type(entity_types type)
{
	std::vector<GameEntity*> building_type_list = grid_manager->get_entities_of_type(type);
	FOWSelectable *building = nullptr;

	if (building_type_list.size() > 0)
	{
		// set the new position to be the closest town hall 
		int i;
		for (i = 0; i < building_type_list.size(); i++)
		{
			building = (FOWSelectable*)building_type_list.at(i);
			if (building->type == type)
			{
				target_town_hall = (FOWSelectable*)building_type_list.at(i);
			}
		}
	}
	return ((FOWSelectable*)building);
}

void FOWGatherer::update(float time_delta)
{
	FOWSelectable* old_building = nullptr;
	FOWSelectable* new_building = nullptr;

	if (state == GRID_COLLECTING)
	{
		// done dropping off or collecting
		if (SDL_GetTicks() - collecting_time > 1000)
		{
			visible = true;
			if (has_gold == false)
			{
				has_gold = true;
				add_to_skin("moneybag");

				old_building = get_entity_of_entity_type(FOW_GOLDMINE);
				std::vector<t_tile> tiles = old_building->get_adjacent_tiles(true);
				t_vertex new_position = t_vertex(tiles[0].x, tiles[0].y, 0);
				hard_set_position(new_position);

				new_building = get_entity_of_entity_type(FOW_TOWNHALL);
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

				old_building = get_entity_of_entity_type(FOW_TOWNHALL);
				std::vector<t_tile> tiles = old_building->get_adjacent_tiles(true);
				if (tiles.size() > 0)
				{
					t_vertex new_position = t_vertex(tiles[0].x, tiles[0].y, 0);
					hard_set_position(new_position);

					new_building = get_entity_of_entity_type(FOW_GOLDMINE);
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
	}
	FOWCharacter::update(time_delta);
}