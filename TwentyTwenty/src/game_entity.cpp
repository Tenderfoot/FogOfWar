
#include "game_entity.h"

void GameEntity::draw() {

	glPushMatrix();
		glDisable(GL_TEXTURE_2D);
		glTranslatef(transform.x, transform.y, -50.0f);
		float width = (transform.w / 2);
		float height = (transform.h / 2);
		glBegin(GL_QUADS);
			glVertex3f(transform.x - width, transform.y - height, 0.0f);
			glVertex3f(transform.x + width, transform.y - height, 0.0f);
			glVertex3f(transform.x + width, transform.y + height, 0.0f);
			glVertex3f(transform.x - width, transform.y + height, 0.0f);
		glEnd();
		glEnable(GL_TEXTURE_2D);
	glPopMatrix();

};

bool GameEntity::check_collision(GameEntity* other)
{
	t_transform aabb1 = get_aabb();
	t_transform aabb2 = other->get_aabb();

	if (aabb1.h < aabb2.y)
		return true;

	return false;
}