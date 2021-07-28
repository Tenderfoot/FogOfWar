
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

void FOWCharacter::find_attack_path()
{
	// There is a dirtyness here that is the result of the characters being unaware of their grid entity position.... 
	// easy fix
	GameEntity* temp = nullptr;

	desired_position = ((FOWCharacter*)current_command.target)->entity_position;
	if (grid_manager->tile_map[desired_position.x][desired_position.y].entity_on_position != nullptr)
	{
		temp = grid_manager->tile_map[desired_position.x][desired_position.y].entity_on_position;
		grid_manager->tile_map[desired_position.x][desired_position.y].entity_on_position = nullptr;
	}

	current_path = grid_manager->find_path(position, desired_position);

	if (temp != nullptr)
		grid_manager->tile_map[desired_position.x][desired_position.y].entity_on_position = temp; // (GameEntity*)current_command.target;
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

void FOWCharacter::make_new_path()
{
	if (current_command.type == ATTACK)
	{
		if (check_attack() == false)
			find_attack_path();
		else
			attack();
	}
	else
		current_path = grid_manager->find_path(position, desired_position);
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
		grid_manager->tile_map[position.x][position.y].entity_on_position = nullptr;
		grid_manager->tile_map[next_stop->x][next_stop->y].entity_on_position = this;
		entity_position = t_vertex(next_stop->x, next_stop->y, 0);
	}
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

void FOWCharacter::process_command(FOWCommand next_command)
{
	
	current_command = next_command;

	if (next_command.type == MOVE)
		set_moving(next_command.position);
	
	if (next_command.type == ATTACK)
	{
		if (check_attack() == false)
			set_moving(next_command.target->position);
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
						set_moving(current_command.target->position);
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
}