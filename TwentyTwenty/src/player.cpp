
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

void Player::make_floor()
{
	GameEntity* floor = new GameEntity(0, -30, 30, 10);
	floor->init();
	Game::entities.push_back(floor);
}