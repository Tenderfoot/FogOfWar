#pragma once

#include "common.h"
#include "spine_entity.h"
#include <math.h>
#include <map>

#define MAX_FALL_SPEED -3
#define ACCELERATION_DTG 1
#define MAX_VELOCITY 0.3
#define MOVE_SPEED 0.8
#define FRICTION_COEFFICIENT 1
#define JUMP_FORCE 0.5

typedef enum
{
	IDLE,
	WALK_LEFT,
	WALK_RIGHT,
	JUMPING,
	CASTING,
	DEAD
} e_player_states;

class Player : public SpineEntity
{
public:
	Player() : SpineEntity("witch")
	{
		falling = true;
		flip = false;
	}


	void draw();
	void init();
	void make_floor();
	void take_input(boundinput input, bool keydown);
	virtual void update(float timedelta);
	void state_machine();
	void player_update(float deltatime);

	std::map<boundinput, bool> keydown_map;
	e_player_states state;
	bool falling;
};