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
	void set_skin(std::string skin_name);
	void add_to_skin(std::string skin_name);
	void reset_skin(std::string skin_name);
	t_VBO VBO;


	std::string skeleton_name;
	spine::Skin* current_skin;
	spine::Skeleton *skeleton;
	spine::AnimationState *animationState;
	bool flip;
	t_vertex color;
};