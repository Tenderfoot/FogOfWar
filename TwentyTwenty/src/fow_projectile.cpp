
#include "fow_projectile.h"
#include "paintbrush.h"

FOWProjectile::FOWProjectile(t_vertex position)
{
	type = FOW_PROJECTILE;
	draw_position = position;
	texture = PaintBrush::get_texture("data/images/arrow.png");
	printf("in here\n");
}

void FOWProjectile::update(float delta_time)
{
/*	if (target->position.x > draw_position.x)
		draw_position.x += delta_time;
	else if(target->position.x < draw_position.x)
		draw_position.x -= delta_time;

	if (target->position.y > draw_position.y)
		draw_position.y += delta_time;
	else if (target->position.y < draw_position.y)
		draw_position.y -= delta_time;*/
}