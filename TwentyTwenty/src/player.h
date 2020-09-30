#pragma once

#include <math.h>
#include <map>
#include <string>

#include "common.h"
#include "spine_entity.h"
#include "audiocontroller.h"

#define MAX_FALL_SPEED -3
#define ACCELERATION_DTG 1
#define MAX_VELOCITY 0.3
#define MOVE_SPEED 0.8
#define FRICTION_COEFFICIENT 1
#define JUMP_FORCE 0.4
#define UNDER_HEAD_BOUNCE -0.001	// this is what velocity is set to when you hit your head on the top of a box

typedef enum
{
	IDLE,
	WALK_LEFT,
	WALK_RIGHT,
	JUMPING,
	CASTING,
	DEAD
} e_player_states;

class MyListener : public spine::AnimationStateListenerObject
{
public:
	void callback(spine::AnimationState* state, spine::EventType type, spine::TrackEntry* entry, spine::Event* event)
	{
		// Inspect and respond to the event here.
		if (type == spine::EventType_Event)
		{
			if (std::string(event->getData().getName().buffer()) == std::string("footstep"))
				AudioController::play_sound(std::string("data/").append(std::string(event->getStringValue().buffer())));
		}
	}
};

class Player : public SpineEntity
{
public:
	Player() : SpineEntity("rasta")
	{
		falling = true;
		flip = false;
		camera.x = 0;
		camera.y = 0;
	}

	void draw();
	void init();
	void make_floor();
	void check_slide();
	void take_input(boundinput input, bool keydown);
	virtual void update(float timedelta);
	void state_machine();
	void player_update(float deltatime);

	std::map<boundinput, bool> keydown_map;
	e_player_states state;
	bool falling;
	t_transform camera;
};