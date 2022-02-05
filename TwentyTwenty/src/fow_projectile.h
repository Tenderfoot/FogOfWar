#pragma once

#include "game_entity.h"

class FOWProjectile : public GameEntity
{
public:

	// constructor
	FOWProjectile(t_vertex position);

	// inherited methods
	void update(float delta_time);
	void set_target(GameEntity* target);
	
	// variables
	GameEntity* target;
	float time_spawned;
	float travel_distance;
	bool has_landed;
};