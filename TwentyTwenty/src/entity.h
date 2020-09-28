#pragma once

class Entity
{
public:
	Entity()
	{
	}

	virtual void update(float timedelta) {};
	virtual void draw() {};
};