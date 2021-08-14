
#include "undead.h"

FOWUndead::FOWUndead()
{
	type = FOW_KNIGHT;

	load_spine_data("spine", "skel");
	add_to_skin("sword");
	VBO = SpineManager::make_vbo(skeleton);

	animationState = new spine::AnimationState(SpineManager::stateData["spine"]);
	animationState->addAnimation(0, "idle_two", true, 0);
	animationState->setListener(this);
}

FOWUndead::FOWUndead(t_vertex initial_position) : FOWUndead::FOWUndead()
{
	this->position = initial_position;
	this->entity_position = initial_position;
	dirty_tile_map();
}