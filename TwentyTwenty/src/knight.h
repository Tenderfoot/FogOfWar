#pragma once

#include "fow_character.h"

class FOWBuilding;

class FOWKnight: public FOWCharacter
{
public:

	FOWKnight();
	FOWKnight(t_vertex initial_position);

	void clear_selection()
	{
		FOWSelectable::clear_selection();
	}

};
