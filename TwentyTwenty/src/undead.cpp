
#include "undead.h"

FOWUndead::FOWUndead()
{
	type = FOW_SKELETON;
	skin_name = "skel";
	char_init();
	add_to_skin("sword");

	// audio
	death_sounds.push_back("data/sounds/skeleton_death.wav");
}

FOWUndead::FOWUndead(t_vertex initial_position) : FOWUndead::FOWUndead()
{
	set_position(initial_position);
}