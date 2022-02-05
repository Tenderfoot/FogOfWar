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

	virtual void callback(spine::AnimationState* state, spine::EventType type, spine::TrackEntry* entry, spine::Event* event);

	void char_init();

};
