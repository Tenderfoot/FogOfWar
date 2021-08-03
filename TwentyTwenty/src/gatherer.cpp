#include "gatherer.h"
#include "fow_building.h"
#include "fow_player.h"

FOWGatherer::FOWGatherer()
{
	type = FOW_GATHERER;
	has_gold = false;
	target_town_hall = nullptr;
	target_mine = nullptr;
	build_mode = false;
}

FOWGatherer::FOWGatherer(t_vertex initial_position) : FOWGatherer::FOWGatherer()
{
	this->position = initial_position;
}

void FOWGatherer::draw()
{
	if (build_mode)
	{
		FOWBuilding new_building(grid_manager->mouse_x, grid_manager->mouse_y, 0.1);

		if (grid_manager->space_free(t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0.0f), 3))
		{
			new_building.color = t_vertex(0.0f, 1.0f, 0.0f);
			good_spot = true;
		}
		else
		{
			new_building.color = t_vertex(1.0f, 0.0f, 0.0f);
			good_spot = false;
		}

		new_building.draw();
	}

	FOWCharacter::draw();
}

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
		FOWTownHall* test = new FOWTownHall(current_command.position.x, current_command.position.y, 3);
		grid_manager->entities->push_back(test);
		set_idle();
	}

	FOWCharacter::OnReachDestination();
}

void FOWGatherer::process_command(FOWCommand next_command)
{
	current_command = next_command;

	if (next_command.type == GATHER)
		set_moving(next_command.target);

	if (next_command.type == BUILD_BUILDING)
		set_moving(next_command.position);

	FOWCharacter::process_command(next_command);
}

void FOWGatherer::take_input(boundinput input, bool type, bool queue_add_toggle)
{
	queue_add_toggle = false;
	t_transform hit_position = grid_manager->mouse_coordinates();
	FOWSelectable* hit_target = get_hit_target();

	if (input == LMOUSE && type == false)
	{
		if (build_mode && good_spot)
		{
			give_command(FOWCommand(BUILD_BUILDING, t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0.0f)));
		}
	}

	if (input == ALT && type == true)
	{
		build_mode = true;
	}

	if (input == RMOUSE && type == true)
	{
		if (hit_target != nullptr)
		{
			if (hit_target->type == FOW_GOLDMINE && has_gold == false)
				give_command(FOWCommand(GATHER, hit_target));
			else if (hit_target->type == FOW_TOWNHALL && has_gold == true)
				give_command(FOWCommand(GATHER, hit_target));
			else if (hit_target->type == FOW_GATHERER)
			{
				if (hit_target == this)
					printf("Stop hittin' yourself");
				else
				{
					give_command(FOWCommand(ATTACK, hit_target));
				}
			}
			else
				give_command(FOWCommand(MOVE, hit_target));
		}
		else
		{
			if(!(hit_position.x==this->position.x && hit_position.y==this->position.y))
				give_command(FOWCommand(MOVE, t_vertex(hit_position.x, hit_position.y, 0.0f)));
		}
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
				target_town_hall = (FOWSelectable*)building_type_list.at(i);
		}
	}
	return ((FOWSelectable*)building);
}

void FOWGatherer::update(float time_delta)
{
	FOWSelectable* building = nullptr;

	if (state == GRID_COLLECTING)
	{
		// done dropping off or collecting
		if (SDL_GetTicks() - collecting_time > 1000)
		{
			visible = true;
			if (has_gold == false)
			{
				has_gold = true;

				// This is popping the character out
				// needs to be replaced with the get_adjacent_tiles 
				t_vertex new_position = t_vertex(position.x - 1, position.y, 0);
				position = new_position;
				draw_position = new_position;

				building = get_entity_of_entity_type(FOW_TOWNHALL);
				if(building != nullptr)
					set_moving(building);
				else
					set_idle();
			}
			else
			{
				has_gold = false;
				t_vertex new_position = t_vertex(position.x - 1, position.y, 0);
				position = new_position;
				draw_position = new_position;
 				
				building = get_entity_of_entity_type(FOW_GOLDMINE);
				if (building != nullptr)
					set_moving(building);
				else
					set_idle();
			}
		}
	}
	FOWCharacter::update(time_delta);
}