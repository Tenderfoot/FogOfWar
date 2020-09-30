
#include <math.h>
#include "player.h"
#include "game.h"

void Player::init()
{
	t_transform aabb = SpineManager::getAABB(skeleton);

	float width = abs(aabb.x - aabb.w);
	float height = abs(aabb.y - aabb.h);

	transform.w = width;
	transform.h = height;

	printf("dimensions: %f, %f\n", width, height);

	transform.x = 0;
	transform.y = 30;
	velocity.x = 0;
	velocity.y = 0;

	state = IDLE;

	animationState->setListener(new MyListener());
}

void Player::update(float timedelta) 
{
	state_machine();
	player_update(timedelta);

	if (keydown_map[LEFT] == false && keydown_map[RIGHT] == false && !falling)
	{
		if (velocity.x > 0)
		{
			velocity.x -= FRICTION_COEFFICIENT * timedelta;
			if (velocity.x < 0)
				velocity.x = 0;
		}
		if (velocity.x < 0)
		{
			velocity.x += FRICTION_COEFFICIENT * timedelta;
			if (velocity.x > 0)
				velocity.x = 0;
		}
	}

	if (velocity.y > MAX_FALL_SPEED)
	{
		velocity.y -= ACCELERATION_DTG * timedelta;
	}

	transform.y += velocity.y;

	falling = true;
	for (std::vector<Entity*>::iterator it = Game::entities.begin(); it != Game::entities.end(); ++it)
	{
		if ((*it)->entity_type == GAME_ENTITY && (*it) != this)
		{
			GameEntity* test = (GameEntity*)(*it);
			if (check_collision(test))
			{
				if (velocity.y < 0)
				{
					transform.y = (test->get_aabb().h);
					velocity.y = 0;
					falling = false;

					if (state == JUMPING)
					{
						check_slide();
						state = IDLE;
					}
				}
				else
				{
					transform.y = (test->get_aabb().y) - (transform.h);
					velocity.y = UNDER_HEAD_BOUNCE;
				}
			}
		}
	}

	transform.x += velocity.x;

	for (std::vector<Entity*>::iterator it = Game::entities.begin(); it != Game::entities.end(); ++it)
	{
		if ((*it)->entity_type == GAME_ENTITY && (*it) != this)
		{
			GameEntity* test = (GameEntity*)(*it);
			if (check_collision(test))
			{
				if(velocity.x > 0)
					transform.x = (test->get_aabb().x)-(transform.w/2);

				if (velocity.x < 0)
					transform.x = (test->get_aabb().w) + (transform.w / 2);

				velocity.x = 0; 
			}
		}
	}

	SpineEntity::update(timedelta);
};

void Player::draw()
{
	glPushMatrix();
		SpineEntity::draw();
	glPopMatrix();
}

void Player::make_floor()
{
	GameEntity* floor = new GameEntity(0, -30, 30, 10);
	floor->init();
	Game::entities.push_back(floor);
}

void Player::player_update(float deltatime)
{
	if (state == WALK_LEFT || state == WALK_RIGHT)
	{
		if (state == WALK_LEFT)
		{
			velocity.x -= MOVE_SPEED * deltatime;
			flip = false;
		}

		if (velocity.x < -MAX_VELOCITY)
			velocity.x = -MAX_VELOCITY;

		if (state == WALK_RIGHT)
		{
			velocity.x += MOVE_SPEED * deltatime;
			flip = true;
		}

		if (velocity.x > MAX_VELOCITY)
			velocity.x = MAX_VELOCITY;
	}

	// jump
	if (keydown_map[ACTION] == true && state != CASTING && state != DEAD)
	{
		if (velocity.y == 0)
		{
			velocity.y = JUMP_FORCE;
			falling = true;
			animationState->setAnimation(0, "jump", false);
			state = JUMPING;
		}
	}
}

void Player::check_slide()
{
	if (abs(velocity.x) > 0.2 || state == JUMPING)
	{
		animationState->setAnimation(0, "idle", true);
		animationState->addAnimation(0, "idle_two", true, 0);
	}
	else
		animationState->setAnimation(0, "idle_two", true);
}

void Player::state_machine()
{
	if (state == IDLE && (keydown_map[LEFT] || keydown_map[RIGHT]))
	{
		if (keydown_map[LEFT])
			state = WALK_LEFT;

		if (keydown_map[RIGHT])
			state = WALK_RIGHT;

		animationState->setAnimation(0, "walk", true);
	}
	if ((state == WALK_LEFT || state == WALK_RIGHT) && (keydown_map[LEFT] == false && keydown_map[RIGHT] == false))
	{
		if (state != DEAD)
			state = IDLE;

		check_slide();
	}

	if (state == WALK_LEFT && keydown_map[LEFT] == false)
	{
		state = IDLE;
		check_slide();
	}

	if (state == WALK_RIGHT && keydown_map[RIGHT] == false)
	{
		state = IDLE;
		check_slide();
	}
}

void Player::take_input(boundinput input, bool keydown)
{
	keydown_map[input] = keydown;

	/*if (input == HAT_CLEAR)
	{
		keydown_map[LEFT] = false;
		keydown_map[RIGHT] = false;
		keydown_map[UP] = false;
		keydown_map[DOWN] = false;
	}*/

	if (input == LEFT && keydown == true)
	{
		keydown_map[RIGHT] = false;
	}
	else if (input == RIGHT && keydown == true)
	{
		keydown_map[LEFT] = false;
	}

}