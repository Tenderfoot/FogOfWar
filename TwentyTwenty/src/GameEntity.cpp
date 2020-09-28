
#include "game_entity.h"

void GameEntity::draw() {

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