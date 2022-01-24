#pragma once

#include "entity.h"
#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include <math.h>
#include <mutex>

class GameEntity : public Entity
{
public:

	GameEntity();
	GameEntity(float x, float y, float w, float h);

	virtual t_transform get_aabb();

	int layer;
	float r, g, b, a;

	// this overlap is the project merge
	t_transform transform;
	bool visible;
	t_vertex draw_position; // grid position
	t_vertex draw_offset; // fix for buildings

	// moving on...
	t_transform texture_coordinates;
	bool collision_enabled;
	GLuint texture;
	std::mutex entity_mutex;

	virtual void update(float timedelta) {};
	virtual void draw();
	bool check_collision(GameEntity *other);
};