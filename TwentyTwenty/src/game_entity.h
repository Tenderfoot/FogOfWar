#pragma once

#include "entity.h"
#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include <math.h>

class GameEntity : public Entity
{
public:
	GameEntity() : Entity() {
		transform.x = 0;
		transform.y = 0;
		transform.w = 0;
		transform.h = 0;
		entity_type = GAME_ENTITY;
	}

	GameEntity(float x, float y, float w, float h) {
		transform.x = x;
		transform.y = y;
		transform.w = w;
		transform.h = h;
		entity_type = GAME_ENTITY;
	}

	t_transform get_aabb();

	t_transform transform;
	t_transform velocity;

	virtual void update(float timedelta) {};
	virtual void draw();
	bool check_collision(GameEntity *other);
};