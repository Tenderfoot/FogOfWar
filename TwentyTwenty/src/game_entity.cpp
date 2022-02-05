
#include "game_entity.h"
#include "common.h"
#include "paintbrush.h"

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
	if (visible)
	{
		glDisable(GL_DEPTH_TEST);
		PaintBrush::reset_model_matrix();
		PaintBrush::transform_model_matrix(glm::vec3(draw_position.x, -draw_position.y, 0.0f), glm::vec4(0), glm::vec3(1));
		PaintBrush::quad_vbo.texture = texture;		// not great
		PaintBrush::draw_quad_vao();
		PaintBrush::reset_model_matrix();
		glEnable(GL_DEPTH_TEST);
	}
}

t_transform GameEntity::get_aabb()
{
	t_transform aabb;
	float x1 = draw_position.x - (transform.w / 2);
	float y1 = draw_position.y - (transform.h / 2);
	float x2 = draw_position.x + (transform.w / 2);
	float y2 = draw_position.y + (transform.h / 2);

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