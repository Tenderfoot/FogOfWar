
#include "common.h"
#include "spine_entity.h"

void SpineEntity::update(float timedelta) {
	animationState->update(timedelta);
	animationState->apply(*skeleton);
};


t_transform SpineEntity::get_aabb()
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

void SpineEntity::draw() {
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glPushMatrix();
	glTranslatef(transform.x, transform.y, GAME_PLANE);
	if (flip)
		glRotatef(180, 0.0f, 1.0f, 0.0f);
	SpineManager::drawSkeleton(skeleton);
	glPopMatrix();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
};