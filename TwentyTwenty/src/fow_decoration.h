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
	void update_skeleton(std::string skeleton_name, float delta_time);
	static void reset_decorations();
	t_VBO& get_vbo();

	static std::vector<float> all_verticies;
	static std::vector<float> all_texcoords;
	static std::vector<float> all_colors;

	void make_totals();
	static void assemble_megatron();
	static t_VBO megatron_vbo;
	static int total_num_faces;
	static GLuint texture;

	int tree_variation;
	t_VBO* ref_to_shared_vbo;
	static std::map<std::string, t_decoration_sharedinfo> decoration_shared_info;
};