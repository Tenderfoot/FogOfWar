
#include <algorithm>
#include "fow_character.h"
#include "audiocontroller.h"
#include "game.h"
#include "user_interface.h"
#include "client_handler.h"

FOWCharacter::FOWCharacter()
{
	type = FOW_CHARACTER;
	visible = true;
	size = 1;
	attack_move_target = nullptr;
	sight = 4;
	maximum_hp = 60;
	current_hp = maximum_hp;
	spine_initialized = false;
	skeleton_name = "spine";
	skin_name = "Knight";
	network_target = nullptr;
	speed = 500;	// it takes half a second
}

void FOWCharacter::char_init()
{
	animationState->addAnimation(0, "idle_two", true, 0);
	dirty_tile_map();
}

void FOWCharacter::take_damage(int amount)
{
	current_hp -= amount;
	if (current_hp < 0)
	{
		die();
	}
}

void FOWCharacter::set_position(t_vertex initial_position)
{
	this->position = initial_position;
	this->entity_position = initial_position;
}

void FOWCharacter::die()
{
	state = GRID_DYING;
	animationState->setAnimation(0, "die", false);
	play_audio_queue(SOUND_DEATH);
	GridManager::tile_map[entity_position.x][entity_position.y].entity_on_position = nullptr;
	clear_selection();
}

void FOWCharacter::draw()
{
	// if this is a client, flip is sent from the server
	if (ClientHandler::initialized == false)
	{
		if (state == GRID_ATTACKING)
		{
			desired_position.x = get_attack_target()->position.x;
		}

		flip = (draw_position.x < desired_position.x);
	}
	FOWSelectable::draw();
}

void FOWCharacter::callback(spine::AnimationState* state, spine::EventType type, spine::TrackEntry* entry, spine::Event* event)
{
	// Inspect and respond to the event here.
	if (type == spine::EventType_Event)
	{
		// spine has its own string class that doesn't work with std::string
		if (std::string(event->getData().getName().buffer()) == std::string("attack_event"))
		{
			if (!ClientHandler::initialized)	// dealing damage? not the clients job
			{									// but client still needs impact sound and to perform this animation
				get_attack_target()->take_damage(10);
			}
			AudioController::play_sound("data/sounds/weapon_impact/impact1.ogg");
		}
	}
}

void FOWCharacter::hard_set_position(t_vertex new_position)
{
	position = new_position;
	entity_position = new_position;
	draw_position = new_position;
	dirty_tile_map();
}

void FOWCharacter::take_input(SDL_Keycode input, bool type, bool queue_add_toggle)
{
	FOWSelectable* hit_target = get_hit_target();
	t_vertex hit_position;

	// this was a minimap click
	if (Game::minimap->coords_in_ui())
	{
		hit_position = Game::minimap->get_click_position();
	}
	else
	{
		hit_position = Game::coord_mouse_position;
	}

	if (input == RMOUSE && type == true)
	{
		if (hit_target != nullptr)
		{
			if (hit_target == this)
			{
				printf("Stop hittin' yourself");
				return;
			}
			else if (hit_target->team_id != team_id)
			{
				give_command(FOWCommand(ATTACK, hit_target));
			}
			else
				give_command(FOWCommand(MOVE, hit_target));
		}
		else
		{
			if (!(hit_position.x == this->position.x && hit_position.y == this->position.y))
				give_command(FOWCommand(MOVE, t_vertex(hit_position.x, hit_position.y, 0.0f)));
		}
	}
}

FOWSelectable* FOWCharacter::get_hit_target()
{
	t_vertex hit_position = Game::coord_mouse_position;

	// This is if they are moving, we can still properly interpret if its not a direct click
	if (GridManager::tile_map[hit_position.x][hit_position.y].entity_on_position != nullptr)
		return (FOWSelectable*)(GridManager::tile_map[hit_position.x][hit_position.y].entity_on_position);

	// lets see if theres something on the hit position...
	for (auto entityItr : Game::entities)
	{
		if (is_selectable(entityItr->type))
		{
			if (entityItr->position.x == hit_position.x && entityItr->position.y == hit_position.y
				&& entityItr->position.x == hit_position.x && entityItr->position.y == hit_position.y)
			{
				return (FOWSelectable*)entityItr;
			}
		}
	}

	return nullptr;
}

void FOWCharacter::set_idle()
{
	state = GRID_IDLE;
	animationState->setAnimation(0, "idle_two", true);

	if (command_queue.size() > 0)
	{
		command_queue.erase(command_queue.begin());
	}
}

struct sort_for_distance {
	sort_for_distance(t_vertex char_position) { this->char_position = char_position; }
	bool operator () (t_tile i, t_tile j) { return (t_vertex(char_position.x-i.x, char_position.y - i.y, 0).Magnitude() < t_vertex(char_position.x - j.x, char_position.y - j.y, 0).Magnitude()); }
	t_vertex char_position;
};

void FOWCharacter::find_path_to_target(FOWSelectable *target)
{
	if (target == nullptr)
	{
		printf("Target for find_path_to_target was nullptr!\n");
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
	current_path = GridManager::find_path(position, desired_position);
}

// Okay it looks like this method isn't just making a new path
// its re-evaluating, for a unit, whether or not they are ready to attack their target
// for an attack command, its checking if the unit is beside them (or in range), and if not, finding a new path to the attack command target
// for attack move, first it checks for both melee/range, but if that fails, for melee it checks the range for a new move path
void FOWCharacter::make_new_path()
{
	// this allows us to mixed in ranged
	bool should_check_sight = attack_type == ATTACK_MELEE ? false : true;

	if (current_command.type == ATTACK)
	{
		if (check_attack(should_check_sight) == false)
		{
			find_path_to_target(current_command.target);
		}
		else
		{
			attack();
		}
	}
	else if (current_command.type == ATTACK_MOVE)
	{
		if (check_attack(should_check_sight) == false)
		{
			if (check_attack(true) == false)
			{
				if (current_path.size() > 1)
				{
					t_tile* next_stop = current_path.at(current_path.size() - 2);
					if (GridManager::tile_map[next_stop->x][next_stop->y].entity_on_position == nullptr)
					{
						current_path.pop_back();
					}
					else
					{
						set_moving(current_command.position);
					}
				}
				else
				{
					set_moving(current_command.position);
				}
			}
			else
			{
				find_path_to_target(attack_move_target);
			}
		}
		else
		{
			attack();
		}
	}
	else
	{
		// here I need to add in
		// remaking a path to the gather target (finding another empty adjacent square)
		// or decide what I want to do about moving to a square that is now occupied
		current_path = GridManager::find_path(position, desired_position);
	}
}

void FOWCharacter::OnReachNextSquare()
{
	t_tile* next_stop = current_path.at(current_path.size() - 1);

	position.x = next_stop->x;
	position.y = next_stop->y;
	draw_position = position;
	time_reached_last_square = SDL_GetTicks();

	// a new move command came in, process after you hit the next grid space
	if (!(current_command == command_queue.at(0)))
	{
		process_command(command_queue.at(0));
		return;
	}

	// this block checks if we can continue on the path, or if we need to re-evaluate things
	// in the case of an attack command, its possible the targets position has changed
	// in the case of the attack move command, we need to (see make_new_path)
	if (current_path.size() > 1)
	{
		next_stop = current_path.at(current_path.size() - 2);

		if (GridManager::tile_map[next_stop->x][next_stop->y].entity_on_position != nullptr || current_command.type == ATTACK || current_command.type == ATTACK_MOVE)
			make_new_path();
		else
			current_path.pop_back();	// this is what we want to happen most of the time
	}
	else
	{
		make_new_path();
	}

	move_entity_on_grid();
}

void FOWCharacter::OnReachDestination()
{
	if (current_command.type == MOVE)
	{
		set_idle();
	}

	if (current_command.type == ATTACK)
	{
		printf("Something went wrong, this shouldn't get hit ever\n");
		set_idle();
	}

	if (current_command.type == ATTACK_MOVE)
	{
		printf("Made it!\n");
		set_idle();
	}
}

// Basic idea is that when a character is blocked
// instead of clearing and going idle, store the command and go into blocked state
// blocked state is the same as idle but after a short time the previous command is reattempted
void FOWCharacter::PathBlocked()
{
	// every 3 times you get blocked, it clears you to idle
	// this makes it inconsistent if the player is giving orders to
	// blocked units but otherwise is a net win
	blocked_retry_count++;
	if (blocked_retry_count % 3 == 0)
	{
		set_idle();
		return;
	}

	state = GRID_BLOCKED;

	printf("I'm Blocked!\n");
	// this got called with spine_initialized false
	// lets catch it but also communicate it
	if (spine_initialized == true)
	{
		animationState->setAnimation(0, "idle_two", true);
	}
	else
	{
		printf("tried to set idle with spine initialized.\n");
	}

	blocked_command_queue = command_queue;
	blocked_time = SDL_GetTicks();

	if (command_queue.size() > 0)
	{
		command_queue.erase(command_queue.begin());
	}
}

// Check to see if there is a potential target is beside you
// this is for the "Attack_Move" command
// this should be refactored and combined with the method above
bool FOWCharacter::check_attack(bool use_far)
{
	// We want to test the adjacent 8 squares for a target
	int i, j;
	for (i = -1; i < 2; i++)
	{
		for (j = -1; j < 2; j++)
		{
			FOWSelectable* entity_on_pos = (FOWSelectable*)GridManager::tile_map[i + entity_position.x][j + entity_position.y].entity_on_position;
			if (entity_on_pos != nullptr)
			{
				if (entity_on_pos->team_id != team_id)
				{
					// this should be the closest entity, not just the first one iterated on
					if (current_command.type == ATTACK)
					{
						// for attack move, current_command.target = nullptr, and if entity_on_position is also nullptr,
						// technically current_command.target = entity_on_position (because nullptr == nullptr)
						if (GridManager::tile_map[i + entity_position.x][j + entity_position.y].entity_on_position == current_command.target)
							return true;
					}
					else if (current_command.type == ATTACK_MOVE)
					{
						attack_move_target = entity_on_pos;
						return true;
					}
				}
			}
		}
	}

	// if they weren't there, we want to check the squares away up to (sight)
	// ignoring the squares we've already checked
	if (use_far)
	{
		for (i = -sight; i < sight; i++)
		{
			for (j = -sight; j < sight; j++)
			{
				if (!((i < 2 && i > -2) && (j < 2 && j > -2)))	// just don't look in the range we've already looked at
				{
					FOWSelectable* entity_on_pos = (FOWSelectable*)GridManager::tile_map[i + entity_position.x][j + entity_position.y].entity_on_position;
					if (entity_on_pos != nullptr)
						if (entity_on_pos->is_unit() && entity_on_pos->team_id != team_id)
						{
							// this should be the closest entity, not just the first one iterated on
							attack_move_target = entity_on_pos;
							return true;
						}
				}
			}
		}
	}

	return false;
}

void FOWCharacter::attack()
{
	state = GRID_ATTACKING;

	FOWSelectable* target = nullptr;
	std::string attack_prefix;
	target = get_attack_target();

	if (attack_type == ATTACK_MELEE)
	{
		attack_prefix = "attack";
	}
	else if (attack_type == ATTACK_RANGED)
	{
		attack_prefix = "shoot";
	}

	if (target->position.x > position.x || target->position.x < position.x)
		if (target->position.y < position.y)
			animationState->setAnimation(0, std::string(attack_prefix+"_sideup").c_str(), false);
		else if (target->position.y > position.y)
			animationState->setAnimation(0, std::string(attack_prefix + "_sidedown").c_str(), false);
		else
			animationState->setAnimation(0, std::string(attack_prefix + "_side").c_str(), false);
	else
		if(target->position.y < position.y)
			animationState->setAnimation(0, std::string(attack_prefix + "_up").c_str(), false);
		else
			animationState->setAnimation(0, std::string(attack_prefix + "_down").c_str(), false);

	if (target->position.x > position.x)
		dir = true;
	else if (target->position.x < position.x)
		dir = false;

}

// this should be part of gridmanager
// here you can check if someone is there maybe someday... *****
void FOWCharacter::move_entity_on_grid()
{
	if (current_path.size() > 0)
	{
		t_tile* next_stop = current_path.at(current_path.size() - 1);
		GridManager::mow(entity_position.x, entity_position.y);
		GridManager::tile_map[entity_position.x][entity_position.y].entity_on_position = nullptr;
		GridManager::tile_map[next_stop->x][next_stop->y].entity_on_position = this;
		entity_position = t_vertex(next_stop->x, next_stop->y, 0);
	}
}

void FOWCharacter::process_command(FOWCommand next_command)
{
	current_command = next_command;

	if (next_command.type == MOVE)
	{
		if (next_command.target != nullptr)
		{
			set_moving(next_command.target);
		}
		else
		{
			set_moving(next_command.position);
		}
	}

	if (next_command.type == ATTACK)
	{
		handle_attack();
	}

	if (next_command.type == ATTACK_MOVE)
	{
		handle_attack_move();
	}

	FOWSelectable::process_command(next_command);
};

void FOWCharacter::give_command(FOWCommand command)
{
	command_queue.clear();

	if (ClientHandler::initialized)		// client will send the command to the server
	{
		command.self_ref = this;
		ClientHandler::command_queue.push_back(command);
		play_audio_queue(SOUND_COMMAND);
	}
	else  // The Server or Local player just gives the command here
	{
		command_queue.push_back(command);

		if (team_id == FOWPlayer::team_id)
		{
			play_audio_queue(SOUND_COMMAND);
		}
	}
}

FOWSelectable* FOWCharacter::get_attack_target()
{
	FOWSelectable* target = nullptr;

	if (ClientHandler::initialized)
	{
		target = network_target;
		if (network_target == nullptr)
		{
			printf("Target not available\n");
			return network_target;
		}
	}
	else
	{
		if (current_command.type == ATTACK)
			target = current_command.target;

		else if (current_command.type == ATTACK_MOVE)
			target = attack_move_target;
	}

	return target;
}

void FOWCharacter::set_moving(t_vertex new_position)
{
	if (state != GRID_MOVING)
	{
		state = GRID_MOVING;
		animationState->setAnimation(0, "walk_two", true);
		time_reached_last_square = SDL_GetTicks();
	}

	desired_position = t_vertex(new_position.x, new_position.y, 0);
	current_path = GridManager::find_path(position, desired_position, true, team_id);
	move_entity_on_grid();
}

void FOWCharacter::set_moving(FOWSelectable *move_target)
{
	if (state != GRID_MOVING)
	{
		state = GRID_MOVING;
		animationState->setAnimation(0, "walk_two", true);
		time_reached_last_square = SDL_GetTicks();
	}
	
	find_path_to_target(move_target);
	move_entity_on_grid();
}

void FOWCharacter::handle_attack()
{
	// this allows us to mixed in ranged
	bool should_check_sight = attack_type == ATTACK_MELEE ? false : true;

	if (check_attack(should_check_sight) == false)
		set_moving(get_attack_target());
	else
		attack();
}

void FOWCharacter::handle_attack_move()
{
	// this allows us to mixed in ranged
	bool should_check_sight = attack_type == ATTACK_MELEE ? false : true;

	// if someone is beside you, attack (else)
	if (check_attack(should_check_sight) == false)
	{
		// if someone is in your vision, attack
		// otherwise move to position
		if (check_attack(true) == false)
		{
			set_moving(current_command.position);
		}
		else
		{
			set_moving(get_attack_target());
		}
	}
	else
	{
		attack();
	}
}

void FOWCharacter::update(float time_delta)
{
	float game_speed = GridManager::game_speed;

	if (state == GRID_MOVING)
	{
		if (current_path.size() > 0)
		{
			t_tile* next_stop = current_path.at(current_path.size() - 1);
			float time_diff = SDL_GetTicks() - time_reached_last_square;

			draw_position.x = std::lerp(position.x, next_stop->x, time_diff / speed);
			draw_position.y = std::lerp(position.y, next_stop->y, time_diff / speed);

			if (time_diff > speed && !ClientHandler::initialized)
			{
				OnReachNextSquare();
			}
		}
		else if(!ClientHandler::initialized)
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
			if (ClientHandler::initialized)
			{
				attack();
			}
			else
			{
				if ((!(current_command == command_queue.at(0))))
				{
					process_command(command_queue.at(0));
					return;
				}

				if (is_unit(get_attack_target()->type))
				{
					if (((FOWCharacter*)get_attack_target())->state == GRID_DYING)
					{
						if (current_command.type == ATTACK)
							set_idle();
						else if (current_command.type == ATTACK_MOVE)
							process_command(current_command);		// this is the worst hack
					}
					else
					{
						if (current_command.type == ATTACK)
						{
							handle_attack();
						}

						if (current_command.type == ATTACK_MOVE)
						{
							handle_attack_move();
						}
					}
				}
				else if (is_building(get_attack_target()->type))
				{
					if (((FOWBuilding*)get_attack_target())->destroyed == true)
					{
						set_idle();
					}
					else
						attack();
				}
			}
		}
	}
	else if (state == GRID_DYING)
	{
	}
	else if (state == GRID_IDLE || state == GRID_BLOCKED)
	{
		// moved this line from draw()
		draw_position = position;
	
		if (state == GRID_BLOCKED && (SDL_GetTicks() - blocked_time) > 2000)
		{
			command_queue = blocked_command_queue;
		}

		if (command_queue.size() > 0)
			process_command(command_queue.at(0));
	}

	if (state != GRID_DEAD)
	{
		FOWSelectable::update(time_delta);
	}

	// this attacks units beside you if you are idle
	think();
}

void FOWCharacter::think()
{
	// we don't belong to the player, so use AI
	
	if (state == GRID_IDLE)
	{
		std::vector<t_tile> tiles = this->get_adjacent_tiles(false);

		if (tiles.size() > 0)
		{
			for (int i = 0; i < tiles.size(); i++)
			{
				FOWSelectable* entity = (FOWSelectable*)tiles[i].entity_on_position;

				if (!ClientHandler::initialized)
				{
					if (entity != nullptr && entity != this && (entity->state != GRID_DYING || entity->state != GRID_DEAD) && entity->team_id != team_id)
					{
						give_command(FOWCommand(ATTACK, entity));
					}
				}
			}
		}
	}
}