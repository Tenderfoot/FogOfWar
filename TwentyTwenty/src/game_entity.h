#pragma once

#include "entity.h"

class GameEntity : public Entity
{
public:
	GameEntity() : Entity() {
		transform.x = 0;
		transform.y = 0;
		transform.w = 0;
		transform.h = 0;
	}

	GameEntity(float x, float y, float w, float h) : Entity() {
		transform.x = x;
		transform.y = y;
		transform.w = w;
		transform.h = h;
	}

	t_transform transform;
	t_transform velocity;

	virtual void update(float timedelta) {};
	virtual void draw() {
		
		glPushMatrix();
			glDisable(GL_TEXTURE_2D);
			glTranslatef(transform.x, transform.y, -50.0f);
			float width = (transform.w / 2);
			float height = (transform.h / 2);
			glBegin(GL_QUADS);
				glVertex3f(transform.x - width, transform.y - height, -50.0f);
				glVertex3f(transform.x + width, transform.y - height, -50.0f);
				glVertex3f(transform.x + width, transform.y + height, -50.0f);
				glVertex3f(transform.x - width, transform.y + height, -50.0f);
			glEnd();
			glEnable(GL_TEXTURE_2D);
		glPopMatrix();
	
	};
};