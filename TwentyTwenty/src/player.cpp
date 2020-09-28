
#include "player.h"
#include "game.h"

void Player::init()
{
	t_transform aabb = SpineManager::getAABB(skeleton);

	float width = abs(aabb.x - aabb.w);
	float height = abs(aabb.y - aabb.h);

	printf("dimensions: %f, %f\n", width, height);

	velocity.x = 0;
	velocity.y = 0;
}

void Player::update(float timedelta) {

	if (velocity.y > MAX_FALL_SPEED && falling)
	{
		velocity.y -= ACCELERATION_DTG * timedelta;
	}

	transform.x += velocity.x;
	transform.y += velocity.y;

	for (std::vector<Entity*>::iterator it = Game::entities.begin(); it != Game::entities.end(); ++it)
	{
		if ((*it)->entity_type == GAME_ENTITY && (*it) != this)
		{
			GameEntity* test = (GameEntity*)(*it);
			if (check_collision(test))
			{
				transform.y = (test->get_aabb().y) - (transform.w / 2);
				velocity.y = 0;
				falling = false;
			}
		}
	}

	SpineEntity::update(timedelta);
};

void Player::make_floor()
{
	GameEntity* floor = new GameEntity(0, -30, 30, 10);
	floor->init();
	Game::entities.push_back(floor);
}