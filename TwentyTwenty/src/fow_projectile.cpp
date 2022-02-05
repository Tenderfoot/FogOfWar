
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
	scale = glm::vec3(1.0f, 1.0f, 1.0f);
	rotation = glm::vec4(0);
}

void FOWProjectile::set_target(GameEntity* new_target)
{
	target = new_target;
	travel_distance = (draw_position - new_target->position).Magnitude();
	time_spawned = SDL_GetTicks();

	t_vertex positional_angle = position - target->position;
	positional_angle.Normalize();
	float angle = t_vertex(-1,0,0).DotProduct(positional_angle);
	printf("%f\n", angle);
	rotation = glm::vec4(0.0f, 0.0f, 1.0f, angle);
}

void FOWProjectile::update(float delta_time)
{
	float time_diff = SDL_GetTicks() - time_spawned;
	float speed = 250.0f;

	draw_position.x = std::lerp(position.x, target->position.x, time_diff/(travel_distance*speed));
	draw_position.y = std::lerp(position.y, target->position.y, time_diff/(travel_distance*speed));

	if ((time_diff / (travel_distance * speed)) > 1 && !has_landed)
	{
		visible = false;
		((FOWSelectable*)target)->take_damage(15);
		has_landed = true;
	}
}