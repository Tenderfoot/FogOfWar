#pragma once

typedef struct
{
	float x, y, w, h;
} t_transform;

typedef enum
{
	ENTITY,
	GAME_ENTITY,
	PLAYER_ENTITY
}e_entity_types;

class Entity
{
public:
	Entity()
	{
		entity_type = ENTITY;
	}

	e_entity_types entity_type;

	virtual void init() {};
	virtual void update(float timedelta) {};
	virtual void draw() {};
};