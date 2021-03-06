
#include "knight.h"

FOWKnight::FOWKnight()
{
	type = FOW_KNIGHT;
	skin_name = "knight";
	attack_type = ATTACK_MELEE;

	maximum_hp = 60;
	current_hp = maximum_hp;

	// audio
	ready_sounds.push_back("data/sounds/knight_sounds/Hready.wav");
	select_sounds.push_back("data/sounds/knight_sounds/Hwhat1.wav");
	select_sounds.push_back("data/sounds/knight_sounds/Hwhat2.wav");
	select_sounds.push_back("data/sounds/knight_sounds/Hwhat3.wav");
	select_sounds.push_back("data/sounds/knight_sounds/Hwhat4.wav");
	select_sounds.push_back("data/sounds/knight_sounds/Hwhat5.wav");
	select_sounds.push_back("data/sounds/knight_sounds/Hwhat6.wav");
	command_sounds.push_back("data/sounds/knight_sounds/Hyessir1.wav");
	command_sounds.push_back("data/sounds/knight_sounds/Hyessir2.wav");
	command_sounds.push_back("data/sounds/knight_sounds/Hyessir3.wav");
	command_sounds.push_back("data/sounds/knight_sounds/Hyessir4.wav");
	death_sounds.push_back("data/sounds/death.wav");
}

FOWKnight::FOWKnight(t_vertex initial_position) : FOWKnight::FOWKnight()
{
	set_position(initial_position);
}


void FOWKnight::char_init()
{
	animationState->addAnimation(0, "idle_two", true, 0);
	animationState->setListener(this);
	add_to_skin("sword");
	dirty_tile_map();
}