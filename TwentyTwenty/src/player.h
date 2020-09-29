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
	t_transform get_aabb()
	{
		t_transform aabb;
		float x1 = transform.x - (transform.w / 2);
		float y1 = transform.y;
		float x2 = transform.x + (transform.w / 2);
		float y2 = transform.y + transform.h;

		aabb.x = std::min(x1, x2);
		aabb.w = std::max(x1, x2);
		aabb.y = std::min(y1, y2);
		aabb.h = std::max(y1, y2);

		return aabb;
	}

	std::map<boundinput, bool> keydown_map;
	e_player_states state;
	bool falling;
};