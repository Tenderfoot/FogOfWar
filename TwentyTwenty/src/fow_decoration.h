#pragma once

#include "spine_entity.h"


typedef struct
{
	bool initialized;
	bool animated;
	t_VBO shared_vbo[3];
	spine::AnimationState* shared_animationState[3];
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

	int tree_variation;
	t_VBO* ref_to_shared_vbo;
	static std::map<std::string, t_decoration_sharedinfo> decoration_shared_info;
};