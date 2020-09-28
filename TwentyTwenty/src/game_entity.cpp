
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

t_transform GameEntity::get_aabb()
{
	t_transform aabb;
	float x1 = transform.x - (transform.w / 2);
	float y1 = transform.y - (transform.h / 2);
	float x2 = transform.x + (transform.w / 2);
	float y2 = transform.y + (transform.h / 2);

	aabb.x = std::min(x1, x2);
	aabb.w = std::max(x1, x2);
	aabb.y = std::min(y1, y2);
	aabb.h = std::max(y1, y2);

	return aabb;
}

bool GameEntity::check_collision(GameEntity* other)
{
	t_transform aabb1 = get_aabb();
	t_transform aabb2 = other->get_aabb();

	if (aabb1.h < aabb2.y)
		return true;

	return false;
}