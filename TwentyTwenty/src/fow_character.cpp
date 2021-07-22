
#include "fow_character.h"
#include "fow_building.h"
#include "fow_player.h"

void FOWGatherer::OnReachDestination()
{
	if (current_command.type == GATHER)
	{
		// if we're gathering and we've reached our destination we're either at a gold mine or a town hall
		if (has_gold == false)
		{
			position = current_command.target->position;
			draw_position = current_command.target->position;
			visible = false;
			state = GRID_COLLECTING;
			collecting_time = SDL_GetTicks();
		}
		else
		{
			t_vertex new_position = t_vertex(target_town_hall->position.x, target_town_hall->position.y, 0.0f);
			position = new_position;
			draw_position = new_position;
			owner->gold++;
			visible = false;
			state = GRID_COLLECTING;
			collecting_time = SDL_GetTicks();
		}
	}

	if (current_command.type == BUILD_BUILDING)
	{
		FOWTownHall *test = new FOWTownHall(current_command.position.x, current_command.position.y, 3);
		grid_manager->entities->push_back(test);
		set_idle();
	}

	FOWCharacter::OnReachDestination();
}

void FOWGatherer::process_command(FOWCommand next_command)
{
	if (next_command.type == GATHER)
	{
		desired_position = t_vertex(next_command.target->position.x, next_command.target->position.y-1, 0.0f);
		state = GRID_MOVING;
		draw_position = position;
		current_path = grid_manager->find_path(position, desired_position);
		animationState->setAnimation(0, "walk_two", true);
	}

	if (next_command.type == BUILD_BUILDING)
	{
		printf("Building Building\n");
		desired_position = next_command.position;
		state = GRID_MOVING;
		draw_position = position;
		current_path = grid_manager->find_path(position, desired_position);
		animationState->setAnimation(0, "walk_two", true);
	}

	FOWCharacter::process_command(next_command);
}

void FOWGatherer::PathBlocked()
{
	printf("I'm Blocked!");
	set_idle();
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

					// set the new position to be the closes town hall
					int i;
					for (i = 0; i < townhall_list.size(); i++)
					{
						GameEntity *town_hall = townhall_list.at(i);
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
					state = GRID_IDLE;
					set_idle();
				}
			}
			else
			{
				has_gold = false;
				desired_position = t_vertex(current_command.target->position.x, current_command.target->position.y-1, 0.0f);
				current_path = grid_manager->find_path(position, desired_position);
				state = GRID_MOVING;
				draw_position = position;
			}
		}
	}
	FOWCharacter::update(time_delta);
}