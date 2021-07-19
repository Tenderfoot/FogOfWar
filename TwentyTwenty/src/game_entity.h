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
		texture_coordinates.x = 0;
		texture_coordinates.y = 0;
		texture_coordinates.w = 1;
		texture_coordinates.h = 1;
		type = GAME_ENTITY;
	}

	GameEntity(float x, float y, float w, float h) {
		transform.x = x;
		transform.y = y;
		transform.w = w;
		transform.h = h;
		type = GAME_ENTITY;
		texture_coordinates.x = 0;
		texture_coordinates.y = 0;
		texture_coordinates.w = 1;
		texture_coordinates.h = 1;
	}

	virtual t_transform get_aabb();

	int layer;
	float r, g, b, a;

	// this overlap is the project merge
	t_transform transform;
	t_vertex position;

	// moving on...
	t_transform velocity;
	t_transform texture_coordinates;
	bool collision_enabled;
	GLuint texture;

	virtual void update(float timedelta) {};
	virtual void draw();
	bool check_collision(GameEntity *other);
};