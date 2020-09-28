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

	t_transform get_aabb()
	{
		t_transform aabb;
		float x1 = transform.x - (transform.w / 2);
		float y1 = transform.y - (transform.h / 2);
		float x2 = transform.x + (transform.w / 2);
		float y2 = transform.y + (transform.h / 2);

		aabb.x = std::min(x1, x2);
		aabb.w = std::max(x1, x2);
		aabb.y = std::min(y1, y2);
		aabb.h = std::max(y1, y2);

		return aabb;
	}

	t_transform transform;
	t_transform velocity;

	virtual void update(float timedelta) {};
	virtual void draw();
	bool check_collision(GameEntity *other);
};