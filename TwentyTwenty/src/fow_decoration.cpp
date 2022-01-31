#pragma once

#include "fow_decoration.h"
std::map<std::string, t_decoration_sharedinfo> FOWDecoration::decoration_shared_info;
std::vector<float> FOWDecoration::all_verticies;
std::vector<float> FOWDecoration::all_texcoords;
std::vector<float> FOWDecoration::all_colors;
int FOWDecoration::total_num_faces;
t_VBO FOWDecoration::megatron_vbo;
GLuint FOWDecoration::texture;

FOWDecoration::FOWDecoration()
{
	FOWDecoration("tree", t_vertex(0, 0, 0));
}

FOWDecoration::FOWDecoration(std::string decoration, t_vertex position)
{
	skeleton_name = decoration;
	skin_name = "";
	load_spine_data(skeleton_name, skin_name);

	if (decoration_shared_info.find(decoration) == decoration_shared_info.end())
	{
		decoration_shared_info[decoration] = t_decoration_sharedinfo();
	}

	if (decoration_shared_info[decoration].initialized == false)
	{
		decoration_shared_info[decoration].shared_vbo[0] = SpineManager::make_vbo(skeleton);
		decoration_shared_info[decoration].shared_vbo[1] = SpineManager::make_vbo(skeleton);
		decoration_shared_info[decoration].shared_vbo[2] = SpineManager::make_vbo(skeleton);
		decoration_shared_info[decoration].shared_animationState[0] = new spine::AnimationState(SpineManager::stateData[skeleton_name]);
		decoration_shared_info[decoration].shared_animationState[1] = new spine::AnimationState(SpineManager::stateData[skeleton_name]);
		decoration_shared_info[decoration].shared_animationState[2] = new spine::AnimationState(SpineManager::stateData[skeleton_name]);

		decoration_shared_info[decoration].shared_animationState[0]->addAnimation(0, "animation", true, 0);
		decoration_shared_info[decoration].shared_animationState[1]->addAnimation(0, "animation", true, 1);
		decoration_shared_info[decoration].shared_animationState[2]->addAnimation(0, "animation", true, 2);

		decoration_shared_info[decoration].shared_skeleton = skeleton;
		decoration_shared_info[decoration].initialized = true;
	}

	tree_variation = rand() % 3;

	ref_to_shared_vbo = &decoration_shared_info[decoration].shared_vbo[tree_variation];
	draw_offset = t_vertex(0.0, 0.0, 0.0);
	draw_position = position;
	this->position = position;
	texture = ref_to_shared_vbo->texture;
	spine_initialized = true;
	make_totals();
}

void FOWDecoration::make_totals()
{
	t_VBO the_vbo = get_vbo();
	total_num_faces += get_vbo().num_faces;
	for (int i = 0; i < get_vbo().num_faces * 3; i++)
	{
		all_verticies.push_back(the_vbo.verticies[i]);
	}
	for (int i = 0; i < get_vbo().num_faces * 2; i++)
	{
		all_texcoords.push_back(the_vbo.texcoords[i]);
	}
	for (int i = 0; i < get_vbo().num_faces * 3; i++)
	{
		all_colors.push_back(the_vbo.colors[i]);
	}
}

void FOWDecoration::assemble_megatron()
{
	megatron_vbo = t_VBO();

	megatron_vbo.num_faces = total_num_faces;
	megatron_vbo.texture = texture;

	megatron_vbo.verticies = std::shared_ptr<float[]>(new float[megatron_vbo.num_faces * 3]);
	megatron_vbo.colors = std::shared_ptr<float[]>(new float[megatron_vbo.num_faces * 3]);
	megatron_vbo.texcoords = std::shared_ptr<float[]>(new float[megatron_vbo.num_faces * 2]);

	int i = 0;
	for (auto vertex : all_verticies)
	{
		megatron_vbo.verticies[i] = vertex;
		i++;
	}
	i = 0;
	for (auto color : all_colors)
	{
		megatron_vbo.colors[i] = color;
		i++;
	}
	i = 0;
	for (auto texcoord : all_texcoords)
	{
		megatron_vbo.texcoords[i] = texcoord;
		i++;
	}

	PaintBrush::generate_vbo(megatron_vbo);
	PaintBrush::bind_vbo(megatron_vbo);

	total_num_faces = 0;
}

t_VBO& FOWDecoration::get_vbo()
{
	return decoration_shared_info[skeleton_name].shared_vbo[tree_variation];
}

void FOWDecoration::reset_decorations()
{
	for (auto decoration : decoration_shared_info)
	{
		decoration_shared_info[decoration.first].animated = false;
	}
}

void FOWDecoration::update(float delta_time)
{
	if (decoration_shared_info[skeleton_name].animated == false)
	{
		decoration_shared_info[skeleton_name].shared_animationState[0]->update(delta_time);
		decoration_shared_info[skeleton_name].shared_animationState[0]->apply(*skeleton);
		SpineManager::update_vbo(skeleton, &decoration_shared_info[skeleton_name].shared_vbo[0]);

		decoration_shared_info[skeleton_name].shared_animationState[1]->update(delta_time);
		decoration_shared_info[skeleton_name].shared_animationState[1]->apply(*skeleton);
		SpineManager::update_vbo(skeleton, &decoration_shared_info[skeleton_name].shared_vbo[1]);

		decoration_shared_info[skeleton_name].shared_animationState[2]->update(delta_time);
		decoration_shared_info[skeleton_name].shared_animationState[2]->apply(*skeleton);
		SpineManager::update_vbo(skeleton, &decoration_shared_info[skeleton_name].shared_vbo[2]);

		decoration_shared_info[skeleton_name].animated = true;
	}
}

void FOWDecoration::update_skeleton(std::string new_skeleton_name, float delta_time)
{
	decoration_shared_info[new_skeleton_name].shared_animationState[0]->update(delta_time);
	decoration_shared_info[new_skeleton_name].shared_animationState[0]->apply(*decoration_shared_info[new_skeleton_name].shared_skeleton);
	SpineManager::update_vbo(decoration_shared_info[new_skeleton_name].shared_skeleton, &decoration_shared_info[new_skeleton_name].shared_vbo[0]);

	decoration_shared_info[new_skeleton_name].shared_animationState[1]->update(delta_time);
	decoration_shared_info[new_skeleton_name].shared_animationState[1]->apply(*decoration_shared_info[new_skeleton_name].shared_skeleton);
	SpineManager::update_vbo(decoration_shared_info[new_skeleton_name].shared_skeleton, &decoration_shared_info[new_skeleton_name].shared_vbo[1]);

	decoration_shared_info[new_skeleton_name].shared_animationState[2]->update(delta_time);
	decoration_shared_info[new_skeleton_name].shared_animationState[2]->apply(*decoration_shared_info[new_skeleton_name].shared_skeleton);
	SpineManager::update_vbo(decoration_shared_info[new_skeleton_name].shared_skeleton, &decoration_shared_info[new_skeleton_name].shared_vbo[2]);
}

void FOWDecoration::draw()
{
	if (visible)
	{
		PaintBrush::transform_model_matrix(glm::vec3(draw_position.x + draw_offset.x, -draw_position.y + draw_offset.y, 0.0), glm::vec3(1), glm::vec3(1));
		PaintBrush::draw_vao(*ref_to_shared_vbo);
		PaintBrush::reset_model_matrix();
	}
}