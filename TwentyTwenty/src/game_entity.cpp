
#include "game_entity.h"
#include "common.h"

int Entity::entity_count = 0;

GameEntity::GameEntity() : GameEntity::GameEntity(0, 0, 0, 0) {
}

GameEntity::GameEntity(float x, float y, float w, float h) {
	transform.x = x;
	transform.y = y;
	transform.w = w;
	transform.h = h;
	type = GAME_ENTITY;
	texture_coordinates.x = 0;
	texture_coordinates.y = 0;
	texture_coordinates.w = 1;
	texture_coordinates.h = 1;
}

void GameEntity::draw() 
{
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
		float width = (transform.w / 2);
		float height = (transform.h / 2);
		glColor4f(r, g, b, a);
		glBegin(GL_QUADS);
			glTexCoord2f(texture_coordinates.x, texture_coordinates.y); glVertex3f(transform.x - width, transform.y - height, GAME_PLANE);
			glTexCoord2f(texture_coordinates.w, texture_coordinates.y); glVertex3f(transform.x + width, transform.y - height, GAME_PLANE);
			glTexCoord2f(texture_coordinates.w, texture_coordinates.h); glVertex3f(transform.x + width, transform.y + height, GAME_PLANE);
			glTexCoord2f(texture_coordinates.x, texture_coordinates.h); glVertex3f(transform.x - width, transform.y + height, GAME_PLANE);
		glEnd();
		glColor3f(1.0f, 1.0f, 1.0f);
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
	if (other->collision_enabled == false)
		return false;

	t_transform aabb1 = get_aabb();
	t_transform aabb2 = other->get_aabb();

	if (aabb1.w > aabb2.x && aabb1.x < aabb2.w && aabb1.h > aabb2.y && aabb1.y < aabb2.h)
		return true;

	return false;
}