#pragma once

#include "spine_entity.h"
#include "grid_manager.h"

class FOWDecorationManager
{
public:

	FOWDecorationManager();

	static void make_decorations();

	static std::vector<GameEntity*> decorations;

};