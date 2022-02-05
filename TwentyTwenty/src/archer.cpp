
#include "archer.h"
#include "client_handler.h"
#include "audiocontroller.h"
#include "grid_manager.h"
#include "fow_projectile.h"
#include "game.h"

FOWArcher::FOWArcher()
{
	type = FOW_ARCHER;
	skin_name = "matt";
	attack_type = ATTACK_RANGED;

	// audio
	ready_sounds.push_back("data/sounds/archer_sounds/Eready.wav");
	select_sounds.push_back("data/sounds/archer_sounds/Ewhat1.wav");
	select_sounds.push_back("data/sounds/archer_sounds/Ewhat2.wav");
	select_sounds.push_back("data/sounds/archer_sounds/Ewhat3.wav");
	select_sounds.push_back("data/sounds/archer_sounds/Ewhat4.wav");
	command_sounds.push_back("data/sounds/archer_sounds/Eyessir1.wav");
	command_sounds.push_back("data/sounds/archer_sounds/Eyessir2.wav");
	command_sounds.push_back("data/sounds/archer_sounds/Eyessir3.wav");
	command_sounds.push_back("data/sounds/archer_sounds/Eyessir4.wav");
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
		// spine has its own string class that doesn't work with std::string
		// on second thought this should probably be .compare() == 0
		if (std::string(event->getData().getName().buffer()) == std::string("fire_arrow"))
		{
			GameEntity *new_projectile = GridManager::create_entity(FOW_PROJECTILE, position);
			((FOWProjectile*)new_projectile)->set_target(get_attack_target());
			Game::entities.push_back(new_projectile);
		}
	}
}