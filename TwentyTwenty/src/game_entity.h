#pragma once
#include "entity.h"
#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include <math.h>
#include <mutex>
#include <glm/glm/vec3.hpp> // glm::vec3
#include <glm/glm/vec4.hpp> // glm::vec4

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
	glm::vec3 scale;	// maintian scale for model matrix unifrom
	glm::vec4 rotation;

	// moving on...
	t_transform texture_coordinates;
	bool collision_enabled;
	GLuint texture;
	std::mutex entity_mutex;

	virtual void update(float timedelta) {};
	virtual void draw();
	bool check_collision(GameEntity *other);
};