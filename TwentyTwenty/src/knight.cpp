
#include "knight.h"

FOWKnight::FOWKnight()
{
	type = FOW_KNIGHT;

	load_spine_data("spine", "knight");
	add_to_skin("sword");
	VBO = SpineManager::make_vbo(skeleton);

	animationState = new spine::AnimationState(SpineManager::stateData["spine"]);
	animationState->addAnimation(0, "idle_two", true, 0);
}

FOWKnight::FOWKnight(t_vertex initial_position) : FOWKnight::FOWKnight()
{
	this->position = initial_position;
	this->entity_position = initial_position;
	dirty_tile_map();
}