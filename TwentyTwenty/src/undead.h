#pragma once

#include "fow_character.h"

class FOWBuilding;

class FOWUndead : public FOWCharacter
{
public:

	FOWUndead();
	FOWUndead(t_vertex initial_position);

	void clear_selection()
	{
		FOWSelectable::clear_selection();
	}

};
