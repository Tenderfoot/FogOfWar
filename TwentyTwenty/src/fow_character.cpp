
#include "fow_character.h"

FOWCharacter::FOWCharacter()
{
	type = FOW_CHARACTER;
	visible = true;
}

void FOWCharacter::die()
{
	state = GRID_DYING;
	animationState->setAnimation(0, "die", false);
}

void FOWCharacter::draw()
{
	if (state == GRID_IDLE)
	{
		draw_position = position;
	}

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
		SpineManager::drawSkeleton(skeleton);
		glPopMatrix();
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
}

FOWSelectable* FOWCharacter::get_hit_target()
{
	t_transform hit_position = grid_manager->mouse_coordinates();
	FOWSelectable* hit_target = nullptr;

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
	command_queue.erase(command_queue.begin());
}

void FOWCharacter::find_attack_path()
{
	// maybe theres a better way to do this... path to the unit
	// but path around other units by temporarily freeing the square
	// the unit is on - moving will stop premature anyway because
	// the enemy unit will be within one square
	desired_position = current_command.target->position;
	grid_manager->tile_map[desired_position.x][desired_position.y].entity_on_position = nullptr;
	current_path = grid_manager->find_path(position, desired_position);
	grid_manager->tile_map[desired_position.x][desired_position.y].entity_on_position = (GameEntity*)current_command.target;
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

	// follow that enemy!
	if (current_command.type == ATTACK)
		find_attack_path();
	else
		current_path = grid_manager->find_path(position, desired_position);

	move_entity_on_grid();
}

void FOWCharacter::move_entity_on_grid()
{
	if (current_path.size() > 0)
	{
		t_tile* next_stop = current_path.at(current_path.size() - 1);
		grid_manager->tile_map[position.x][position.y].entity_on_position = nullptr;
		grid_manager->tile_map[next_stop->x][next_stop->y].entity_on_position = this;
	}
}

void FOWCharacter::OnReachDestination()
{
	if (current_command.type == MOVE)
	{
		set_idle();
	}
	if (current_command.type == ATTACK)
	{
		((FOWCharacter*)current_command.target)->die();
		FOWCharacter::set_idle();
	}
}

void FOWCharacter::PathBlocked()
{
	printf("I'm Blocked!");
	set_idle();
}

void FOWCharacter::process_command(FOWCommand next_command)
{
	
	current_command = next_command;

	if (next_command.type == MOVE)
		set_moving(next_command.position);
	
	if (next_command.type == ATTACK)
		set_moving(next_command.target->position);

	FOWSelectable::process_command(next_command);
};

void FOWCharacter::give_command(FOWCommand command)
{
	command_queue.clear();
	command_queue.push_back(command);
}

void FOWCharacter::set_moving(t_vertex new_position)
{
	desired_position = t_vertex(new_position.x, new_position.y, 0);

	state = GRID_MOVING;
	animationState->setAnimation(0, "walk_two", true);
	draw_position = position;

	if (current_command.type == ATTACK)
		find_attack_path();
	else
		current_path = grid_manager->find_path(position, desired_position);
	 
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
		// why in the fuck is this not a "animationfinished" method on SpineEntity
		if (animationState->getCurrent(0) == NULL)
		{
			state = GRID_IDLE;
		}
	}
	else if (state == GRID_DYING)
	{
		if (animationState->getCurrent(0) == NULL)
		{
			state = GRID_IDLE;
		}
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
}