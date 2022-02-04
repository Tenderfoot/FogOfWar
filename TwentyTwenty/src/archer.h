#pragma once

#include "fow_character.h"

class FOWBuilding;

class FOWArcher: public FOWCharacter
{
public:

	FOWArcher();
	FOWArcher(t_vertex initial_position);

	void clear_selection()
	{
		FOWSelectable::clear_selection();
	}

	void char_init();

};
