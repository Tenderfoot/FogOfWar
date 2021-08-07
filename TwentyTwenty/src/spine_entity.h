#pragma once

#include <string>
#include "game_entity.h"
#include "spine_manager.h"

class SpineEntity : public GameEntity
{
public:

	SpineEntity();
	SpineEntity(std::string skin_name);

	virtual void update(float timedelta);
	t_transform get_aabb();
	virtual void draw();
	void build_vbo();

	t_VBO VBO;

	spine::Skin* current_skin;
	spine::Skeleton *skeleton;
	spine::AnimationState *animationState;
	bool flip;
	t_vertex color;
};