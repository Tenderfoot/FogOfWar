#pragma once

#include <math.h>

class t_vertex
{
public:
	t_vertex(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	t_vertex()
	{
	}

	t_vertex operator* (float multiplier)
	{
		this->x = this->x * multiplier;
		this->y = this->y * multiplier;
		this->z = this->z * multiplier;

		return *this;
	}

	t_vertex operator- (t_vertex& other)
	{
		return t_vertex(this->x - other.x, this->y - other.y, this->z - other.z);
	}


	t_vertex operator+= (t_vertex& other)
	{
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;

		return *this;
	}

	float DotProduct(t_vertex other) {
		return x * other.x + y * other.y + z * other.z;
	}

	float Magnitude()
	{
		return (float)sqrt(x * x + y * y);
	}

	void Normalize() {
		float magnitude = Magnitude();
		x = x / magnitude;
		y = y / magnitude;
	}

	float texcoord_x, texcoord_y;
	float x, y, z;
	float r, g, b;
};

class t_transform
{
public:
	t_transform()
	{
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}
	t_transform(float x, float y, float w, float h)
	{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}
	float x, y, w, h;
};

// Entity - a textured QUAD
// Game_Entity - a quad with physics
enum entity_types
{
	ENTITY,
	GAME_ENTITY,
	GRASS_ENTITY,
	PLAYER_ENTITY,
	EMITTER_ENTITY,
	SKELETON_ENTITY,
	ARCHER_ENTITY,
	ARROW_ENTITY,
	BUTTON_ENTITY,
	PORTCULLIS_ENTITY,
	SWORDSMAN_ENTITY,
	ENEMY_SIDEFIRE_ENTITY,
	GRID_CHARACTER,
	GRID_ENEMYCHARACTER,
	GRID_SPAWNPOINT,
	GRID_ENEMYSPAWNPOINT,
	FOW_CHARACTER,
	FOW_BUILDING,
	FOW_TOWNHALL,
	FOW_GOLDMINE,
	FOW_GATHERER
};

class Entity
{
public:
	Entity()
	{
		type = ENTITY;
	}

	entity_types type;
	t_vertex position;

	virtual void init() {};
	virtual void update(float timedelta) {};
	virtual void draw() {};
};