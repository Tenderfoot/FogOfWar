
#include "knight.h"

FOWKnight::FOWKnight()
{
	type = FOW_KNIGHT;
	skin_name = "knight";
	char_init();
	add_to_skin("sword");
}

FOWKnight::FOWKnight(t_vertex initial_position) : FOWKnight::FOWKnight()
{
	this->position = initial_position;
	this->entity_position = initial_position;
	dirty_tile_map();
}