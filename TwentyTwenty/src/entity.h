#pragma once

typedef struct
{
	float x, y, w, h;
} t_transform;

class Entity
{
public:
	Entity()
	{
	}

	virtual void init() {};
	virtual void update(float timedelta) {};
	virtual void draw() {};
};