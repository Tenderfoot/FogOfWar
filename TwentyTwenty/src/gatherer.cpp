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
			set_collecting(current_command.target->position);
		else
		{
			set_collecting(target_town_hall->position);
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
			if (hit_target->type == FOW_GOLDMINE)
			{
				give_command(FOWCommand(GATHER, hit_target));
			}
			else if (hit_target->type == FOW_GATHERER)
			{
				if (hit_target == this)
					printf("Stop hittin' yourself");
				else
				{
					give_command(FOWCommand(ATTACK, hit_target));
				}
			}
		}
		else
		{
			if(!(hit_position.x==this->position.x && hit_position.y==this->position.y))
				give_command(FOWCommand(MOVE, t_vertex(hit_position.x, hit_position.y, 0.0f)));
		}
	}
}

void FOWGatherer::update(float time_delta)
{
	if (state == GRID_COLLECTING)
	{
		// done dropping off or collecting
		if (SDL_GetTicks() - collecting_time > 1000)
		{
			visible = true;
			if (has_gold == false)
			{
				has_gold = true;
				t_vertex new_position = t_vertex(current_command.target->position.x - 1, current_command.target->position.y, 0);
				position = new_position;
				draw_position = new_position;

				std::vector<GameEntity*> townhall_list = grid_manager->get_entities_of_type(FOW_TOWNHALL);

				if (townhall_list.size() > 0)
				{
					state = GRID_MOVING;
					target_mine = (FOWSelectable*)current_command.target;

					// set the new position to be the closest town hall
					int i;
					for (i = 0; i < townhall_list.size(); i++)
					{
						GameEntity* town_hall = townhall_list.at(i);
						if (town_hall->type == FOW_TOWNHALL)
						{
							desired_position = t_vertex(town_hall->position.x, town_hall->position.y - 1, 0.0f);
							target_town_hall = (FOWSelectable*)townhall_list.at(i);
						}
					}
					current_path = grid_manager->find_path(position, desired_position);
				}
				else
				{
					set_idle();
				}
			}
			else
			{
				has_gold = false;
				set_moving(current_command.target);
			}
		}
	}
	FOWCharacter::update(time_delta);
}