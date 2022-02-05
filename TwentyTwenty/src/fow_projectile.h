#pragma once

#include "game_entity.h"

class FOWProjectile : public GameEntity
{
public:

	// constructor
	FOWProjectile(t_vertex position);

	// inherited methods
	void update(float delta_time);

	// variables
	GameEntity* target;
};