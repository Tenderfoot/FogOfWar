#pragma once

#include "fow_selectable.h"

class FOWBuilding;
class FOWPlayer;

class FOWCharacter : public FOWSelectable
{
public:

	FOWCharacter()
	{
		type = FOW_CHARACTER;
		visible = true;
	}

	void die()
	{
		state = GRID_DYING;
		animationState->setAnimation(0, "die", false);
	}

	void draw()
	{
		if (state == GRID_IDLE)
		{
			draw_position = position;
		}

		if (visible)
		{
			glEnable(GL_BLEND);
			glDepthMask(GL_FALSE);
			glPushMatrix();
				glTranslatef(draw_position.x, -draw_position.y, 0.1f);
				SpineManager::drawSkeleton(skeleton);
			glPopMatrix();
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
	}

	void set_idle()
	{
		state = GRID_IDLE;
		animationState->setAnimation(0, "idle_two", true);
		command_queue.erase(command_queue.begin());
	}

	virtual void OnReachNextSquare()
	{
		t_tile *next_stop = current_path.at(current_path.size() - 1);

		position.x = next_stop->x;
		position.y = next_stop->y;

		// follow that enemy!
		if (current_command.type == ATTACK)
			desired_position = t_vertex(current_command.target->position.x, 0, current_command.target->position.z - 1);

		current_path = grid_manager->find_path(position, desired_position);

		draw_position = position;
		dirty_visibiltiy = true;

		// a new move command came in, process after you hit the next grid space
		if (!(current_command == command_queue.at(0)))
		{
			process_command(command_queue.at(0));
		}
	}

	virtual void OnReachDestination()
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

	virtual void PathBlocked()
	{
	}

	void process_command(FOWCommand next_command)
	{
		// Every Character can move, buildings can't
		// Some Buildings can attack but thats ok
		if (next_command.type == MOVE)
		{
			desired_position = next_command.position;
			state = GRID_MOVING;
			current_path = grid_manager->find_path(position, desired_position);
			animationState->setAnimation(0, "walk_two", true);
		}

		if (next_command.type == ATTACK)
		{
			desired_position = t_vertex(next_command.target->position.x, 0, next_command.target->position.z - 1);
			state = GRID_MOVING;
			draw_position = position;
			current_path = grid_manager->find_path(position, desired_position);
		}

		current_command = next_command;

		FOWSelectable::process_command(next_command);
	};

	void give_command(FOWCommand command)
	{
		command_queue.push_back(command);
	}

	virtual void update(float time_delta)
	{
		if (state == GRID_MOVING)
		{
			if (current_path.size() > 0)
			{
				t_tile *next_stop = current_path.at(current_path.size() - 1);

				if (abs((draw_position.x) - next_stop->x) > 0.01)
				{
					if (draw_position.x < next_stop->x)
						draw_position.x += 2*time_delta;
					else
						draw_position.x -= 2*time_delta;
				}

				if (abs(draw_position.y - next_stop->y) > 0.01)
				{
					if (draw_position.y < next_stop->y)
						draw_position.y += 2*time_delta;
					else
						draw_position.y -= 2*time_delta;
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

	t_vertex draw_position;
	t_vertex desired_position;
	bool dirty_visibiltiy;
	std::vector<t_tile*> current_path;
	FOWPlayer *owner;

};

class FOWGatherer : public FOWCharacter
{
public:

	FOWGatherer()
	{
		type = FOW_GATHERER;
		has_gold = false;
		target_town_hall = nullptr;
		target_mine = nullptr;
		build_mode = false;
	}

	FOWGatherer(t_vertex initial_position) : FOWGatherer()
	{
		this->position = initial_position;
	}

	bool has_gold;
	float collecting_time;

	FOWSelectable *target_mine;
	FOWSelectable *target_town_hall;
	bool build_mode;
	bool good_spot;

	virtual void update(float time_delta);
	virtual void OnReachDestination();
	virtual void PathBlocked();
	virtual void process_command(FOWCommand next_command);

	void clear_selection() 
	{
		build_mode = false;
		FOWSelectable::clear_selection();
	}

};