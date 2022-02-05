#pragma once

#include "game_entity.h"

class FOWProjectile : public GameEntity
{
public:

	FOWProjectile(t_vertex position)
	{	
		draw_position = t_vertex(position.x, position.y, position.z);
	}
	
};