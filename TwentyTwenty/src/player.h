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

class Player : public SpineEntity
{
public:
	Player() : SpineEntity("witch")
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