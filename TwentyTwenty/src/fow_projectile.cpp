
#include "fow_projectile.h"
#include "paintbrush.h"
#include "fow_selectable.h"

FOWProjectile::FOWProjectile(t_vertex position)
{
	type = FOW_PROJECTILE;
	this->position = position;
	this->draw_position = position;
	visible = true;
	texture = PaintBrush::get_texture("data/images/arrow.png");
	has_landed = false;
	scale = glm::vec3((0.125f)*344/90, 0.125f, 1.0f);	// 344 / 90 is the aspect ratio of the arrow image
	rotation = glm::vec4(0);
}

void FOWProjectile::set_target(GameEntity* new_target)
{
	target = new_target;
	travel_distance = (draw_position - new_target->position).Magnitude();
	time_spawned = SDL_GetTicks();
	t_vertex positional_angle;

	if(position.y < target->position.y)
		positional_angle = position - target->position;
	else
		positional_angle = target->position-position;

	positional_angle.Normalize();
	float angle = t_vertex(1,0,0).DotProduct(positional_angle);
	float extra_angle = position.y < target->position.y ? 0.0f : glm::radians(180.0f);
	rotation = glm::vec4(0.0f, 0.0f, 1.0f, std::acos(angle) + extra_angle);
}

void FOWProjectile::update(float delta_time)
{
	float time_diff = SDL_GetTicks() - time_spawned;
	float speed = 125.0f;

	draw_position.x = std::lerp(position.x, target->position.x, time_diff/(travel_distance*speed));
	draw_position.y = std::lerp(position.y, target->position.y, time_diff/(travel_distance*speed));

	if ((time_diff / (travel_distance * speed)) > 1 && !has_landed)
	{
		visible = false;
		((FOWSelectable*)target)->take_damage(15);
		has_landed = true;
	}
}