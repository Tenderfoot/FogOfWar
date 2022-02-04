
#include "archer.h"
#include "client_handler.h"
#include "audiocontroller.h"

FOWArcher::FOWArcher()
{
	type = FOW_KNIGHT;
	skin_name = "matt";
	attack_type = ATTACK_RANGED;

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

FOWArcher::FOWArcher(t_vertex initial_position) : FOWArcher::FOWArcher()
{
	set_position(initial_position);
}

void FOWArcher::char_init()
{
	animationState->addAnimation(0, "idle_two", true, 0);
	animationState->setListener(this);
	add_to_skin("bow");
}

void FOWArcher::callback(spine::AnimationState* state, spine::EventType type, spine::TrackEntry* entry, spine::Event* event)
{
	// Inspect and respond to the event here.
	if (type == spine::EventType_Event)
	{
		printf("buffer was %s\n", event->getData().getName().buffer());
		// spine has its own string class that doesn't work with std::string
		// on second thought this should probably be .compare() == 0
		if (std::string(event->getData().getName().buffer()) == std::string("fire_arrow"))
		{
			
		}
	}
}