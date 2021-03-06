#pragma once

#include "spine_entity.h"
#include "grid_manager.h"

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
	FOWDecoration(std::string decoration, t_vertex position, t_tile* tile_ref);

	// overloading this makes it so we can ignore entities for ids
	void increment_entity() {};

	void update(float delta_time);
	void update_skeleton(std::string skeleton_name, float delta_time);
	static void reset_decorations();
	t_VBO& get_vbo();

	static std::map<std::string, std::vector<float>> all_verticies;
	static std::map<std::string, std::vector<float>> all_texcoords;
	static std::map<std::string, std::vector<float>> all_colors;

	void make_totals();
	void delete_decoration();
	void make_all_totals();
	static void clear_totals(std::string decoration_name);
	static void assemble_megatron(std::string decoration_name);
	static void update_megatron(std::string decoration_name);
	static std::map<std::string, t_VBO> megatron_vbo;
	static std::map<std::string, int> total_num_faces;
	static std::map<std::string, GLuint> texture;

	bool deleted;
	t_tile* reference_to_tile;
	int tree_variation;
	t_VBO* ref_to_shared_vbo;
	t_VBO* ref_to_megatron;
	std::vector<float>* ref_to_vertex_map;
	int* ref_to_total_num_faces;
	static std::map<std::string, int> megatron_vertex_pointer;
	static std::vector<std::string> decoration_types;
	int *ref_to_megatron_vertex_pointer;
	static std::map<std::string, t_decoration_sharedinfo> decoration_shared_info;
};