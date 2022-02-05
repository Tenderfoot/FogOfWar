
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
}

void FOWProjectile::set_target(GameEntity* new_target)
{
	target = new_target;
	travel_distance = (draw_position - new_target->position).Magnitude();
	time_spawned = SDL_GetTicks();
}

void FOWProjectile::update(float delta_time)
{
	float time_diff = SDL_GetTicks() - time_spawned;

	draw_position.x = std::lerp(position.x, target->position.x, time_diff/(travel_distance*1000));
	draw_position.y = std::lerp(position.y, target->position.y, time_diff/(travel_distance*1000));

	if ((time_diff / (travel_distance * 1000)) > 1 && !has_landed)
	{
		visible = false;
		((FOWSelectable*)target)->take_damage(15);
		has_landed = true;
	}
}