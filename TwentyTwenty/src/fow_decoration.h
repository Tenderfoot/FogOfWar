#pragma once

#include "spine_entity.h"


typedef struct
{
	bool initialized;
	bool animated;
	t_VBO shared_vbo;
	spine::AnimationState* shared_animationState;
	spine::Skeleton* shared_skeleton;
}t_decoration_sharedinfo;

class FOWDecoration : public SpineEntity
{
public:

	FOWDecoration();
	FOWDecoration(std::string decoration, t_vertex position);
	void update(float delta_time);
	void draw();
	static void reset_decorations();

	static std::map<std::string, t_decoration_sharedinfo> decoration_shared_info;
};