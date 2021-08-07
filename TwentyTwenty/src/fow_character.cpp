
#include "fow_character.h"

FOWCharacter::FOWCharacter()
{
	type = FOW_CHARACTER;
	visible = true;
	size = 1;
}

void FOWCharacter::die()
{
	state = GRID_DYING;
	animationState->setAnimation(0, "die", false);
	grid_manager->tile_map[entity_position.x][entity_position.y].entity_on_position = nullptr;
}

void FOWCharacter::draw()
{
	if (state == GRID_IDLE)
		draw_position = position;

	if(selected)
		draw_selection_box();

	if (visible)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glPushMatrix();
		glTranslatef(draw_position.x, -draw_position.y, 0.1f);
		if (draw_position.x < desired_position.x)
			glRotatef(180, 0.0f, 1.0f, 0.0f);
		if (team_id != 0)
			glColor3f(1.0f, 0.5f, 0.5f);
		SpineManager::drawSkeleton(skeleton);
		glColor3f(1.0f, 1.0f, 1.0f);
		glPopMatrix();
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
}

FOWSelectable* FOWCharacter::get_hit_target()
{
	t_transform hit_position = grid_manager->mouse_coordinates();
	FOWSelectable* hit_target = nullptr;

	// ideally this bounding box checks for visible entities but for now...

	// This is if they are moving, we can still properly interpret if its not a direct click
	if (grid_manager->tile_map[hit_position.x][hit_position.y].entity_on_position != nullptr)
		return (FOWSelectable*)(grid_manager->tile_map[hit_position.x][hit_position.y].entity_on_position);

	// lets see if theres something on the hit position...
	for (int i = 0; i < grid_manager->entities->size(); i++)
	{
		FOWSelectable *test = (FOWSelectable*)grid_manager->entities->at(i);
		if (is_selectable(test->type))
		{
			if (test->position.x == hit_position.x && test->position.y == hit_position.y
				&& test->position.x == hit_position.x && test->position.y == hit_position.y)
			{
				hit_target = test;
			}
		}
	}

	return hit_target;
}

void FOWCharacter::set_idle()
{
	state = GRID_IDLE;
	animationState->setAnimation(0, "idle_two", true);

	if(command_queue.size() > 0)
		command_queue.erase(command_queue.begin());
}

struct sort_for_distance {
	sort_for_distance(t_vertex char_position) { this->char_position = char_position; }
	bool operator () (t_tile i, t_tile j) { return (t_vertex(char_position.x-i.x, char_position.y - i.y, 0).Magnitude() < t_vertex(char_position.x - j.x, char_position.y - j.y, 0).Magnitude()); }
	t_vertex char_position;
};

void FOWCharacter::find_path_to_target(FOWSelectable *target)
{

	if (current_command.target == nullptr)
	{
		printf("Something went wrong\n");
		return;
	}

	std::vector<t_tile> possible_tiles = target->get_adjacent_tiles(true);
	
	if (possible_tiles.size() == 0)
	{
		PathBlocked();
		return;
	}
	std::sort(possible_tiles.begin(), possible_tiles.end(), sort_for_distance(entity_position));

	desired_position = t_vertex(possible_tiles.at(0).x, possible_tiles.at(0).y, 0);
	current_path = grid_manager->find_path(position, desired_position);
}


void FOWCharacter::make_new_path()
{
	if (current_command.type == ATTACK)
	{
		if (check_attack() == false)
			find_path_to_target(current_command.target);
		else
			attack();
	}
	else
	{
		// here I need to add in
		// remaking a path to the gather target (finding another empty adjacent square)
		// or decide what I want to do about moving to a square that is now occupied
		current_path = grid_manager->find_path(position, desired_position);
	}
}

void FOWCharacter::OnReachNextSquare()
{
	t_tile* next_stop = current_path.at(current_path.size() - 1);

	position.x = next_stop->x;
	position.y = next_stop->y;
	draw_position = position;

	// a new move command came in, process after you hit the next grid space
	if (!(current_command == command_queue.at(0)))
	{
		process_command(command_queue.at(0));
		return;
	}

	if (current_path.size() > 1)
	{
		next_stop = current_path.at(current_path.size() - 2);

		if (grid_manager->tile_map[next_stop->x][next_stop->y].entity_on_position != nullptr || current_command.type == ATTACK)
			make_new_path();
		else
			current_path.pop_back();
	}
	else
 		make_new_path();

	move_entity_on_grid();
}

void FOWCharacter::OnReachDestination()
{
	if (current_command.type == MOVE)
		set_idle();

	if (current_command.type == ATTACK)
		printf("Something went wrong, this shouldn't get hit ever");
}

void FOWCharacter::PathBlocked()
{
	printf("I'm Blocked!");
	set_idle();
}


bool FOWCharacter::check_attack()
{
	// We want to test the adjacent 8 squares for the target
	int i, j;
	for (i = -1; i < 2; i++)
		for (j = -1; j < 2; j++)
			if (grid_manager->tile_map[i + entity_position.x][j + entity_position.y].entity_on_position == current_command.target)
				return true;

	return false;
}

void FOWCharacter::attack()
{
	state = GRID_ATTACKING;
	animationState->setAnimation(0, "roll", false);
}

void FOWCharacter::move_entity_on_grid()
{
	if (current_path.size() > 0)
	{
		t_tile* next_stop = current_path.at(current_path.size() - 1);
		grid_manager->tile_map[entity_position.x][entity_position.y].entity_on_position = nullptr;
		grid_manager->tile_map[next_stop->x][next_stop->y].entity_on_position = this;
		entity_position = t_vertex(next_stop->x, next_stop->y, 0);
	}
}

void FOWCharacter::process_command(FOWCommand next_command)
{
	current_command = next_command;

	if (next_command.type == MOVE)
		if(next_command.target != nullptr)
			set_moving(next_command.target);
		else
			set_moving(next_command.position);
	
	if (next_command.type == ATTACK)
	{
		if (check_attack() == false)
			set_moving(next_command.target);
		else
			attack();
	}

	FOWSelectable::process_command(next_command);
};

void FOWCharacter::give_command(FOWCommand command)
{
	command_queue.clear();
	command_queue.push_back(command);
}

void FOWCharacter::set_moving(t_vertex new_position)
{
	if (state != GRID_MOVING)
	{
		state = GRID_MOVING;
		animationState->setAnimation(0, "walk_two", true);
	}

	desired_position = t_vertex(new_position.x, new_position.y, 0);
	current_path = grid_manager->find_path(position, desired_position);
	move_entity_on_grid();
}

void FOWCharacter::set_moving(FOWSelectable *move_target)
{
	if (state != GRID_MOVING)
	{
		state = GRID_MOVING;
		animationState->setAnimation(0, "walk_two", true);
	}
	
	find_path_to_target(move_target);
	move_entity_on_grid();
}


void FOWCharacter::update(float time_delta)
{
	float game_speed = grid_manager->game_speed;

	if (state == GRID_MOVING)
	{
		if (current_path.size() > 0)
		{
			t_tile* next_stop = current_path.at(current_path.size() - 1);

			if (abs((draw_position.x) - next_stop->x) > 0.01)
			{
				if (draw_position.x < next_stop->x)
					draw_position.x += 2 * game_speed * time_delta;
				else
					draw_position.x -= 2 * game_speed * time_delta;
			}

			if (abs(draw_position.y - next_stop->y) > 0.01)
			{
				if (draw_position.y < next_stop->y)
					draw_position.y += 2 * game_speed * time_delta;
				else
					draw_position.y -= 2 * game_speed * time_delta;
			}

			if (t_vertex(t_vertex(next_stop->x, next_stop->y, 0) - draw_position).Magnitude() < 0.025)
			{
				OnReachNextSquare();
			}
		}
		else
		{
			if (position.x != desired_position.x || position.y != desired_position.y)
			{
				PathBlocked();
			}
			else
			{
				OnReachDestination();
			}
		}
	}
	else if (state == GRID_ATTACKING)
	{
		if (animationState->getCurrent(0)->isComplete())
		{
			((FOWCharacter*)current_command.target)->die();

			if (((FOWCharacter*)current_command.target)->state == GRID_DYING)
			{
				if ((!(current_command == command_queue.at(0))))
					process_command(command_queue.at(0));
				else
					set_idle();
			}
			else
			{
				if (!(current_command == command_queue.at(0)))
					process_command(command_queue.at(0));
				else
				{
					if (check_attack() == false)
						set_moving(current_command.target);
					else
						attack();
				}
			}
		}
	}
	else if (state == GRID_DYING)
	{
		// code goes here
	}
	else if (state == GRID_IDLE)
	{
		if (command_queue.size() > 0)
			process_command(command_queue.at(0));
	}

	if (state != GRID_DEAD)
	{
		SpineEntity::update(time_delta);
	}

	if (team_id != 0)
		think();
}

void FOWCharacter::think()
{
	// we don't belong to the player, so use AI
	std::vector<t_tile> tiles = this->get_adjacent_tiles(false);

	if (tiles.size() > 0)
	{
		for (int i = 0; i < tiles.size(); i++)
		{
			FOWSelectable* entity = (FOWSelectable*)tiles[i].entity_on_position;

			// type here to fix a bug - was attacking building - building had no die animation
			if (entity != nullptr && entity != this && entity->state != GRID_DYING && entity->type == FOW_GATHERER && entity->team_id != team_id)
				if (state == GRID_IDLE)
					give_command(FOWCommand(ATTACK, entity));
		}
	}


}