#pragma once

#include "fow_decoration.h"
#include "grid_manager.h"

std::map<std::string, t_decoration_sharedinfo> FOWDecoration::decoration_shared_info;
std::map<std::string, std::vector<float>> FOWDecoration::all_verticies;
std::map<std::string, std::vector<float>> FOWDecoration::all_texcoords;
std::map<std::string, std::vector<float>> FOWDecoration::all_colors;
std::map<std::string, t_VBO> FOWDecoration::megatron_vbo;
std::map<std::string, int> FOWDecoration::total_num_faces;
std::map<std::string, GLuint> FOWDecoration::texture;

extern PFNGLBUFFERDATAARBPROC      glBufferData;
extern PFNGLBINDBUFFERARBPROC      glBindBuffer;
extern PFNGLBINDVERTEXARRAYPROC	glBindVertexArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

std::map<std::string, int> FOWDecoration::megatron_vertex_pointer;

FOWDecoration::FOWDecoration()
{
}

FOWDecoration::FOWDecoration(std::string decoration, t_vertex position, t_tile *tile_ref)
{
	skeleton_name = decoration;
	skin_name = "";
	load_spine_data(skeleton_name, skin_name);	// this creates a new spine skeleton for each - maybe memory issue here?
	reference_to_tile = tile_ref;

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

		if (decoration.compare("tree") == 0)
		{
			set_skin("plain");
			SpineManager::reset_vbo(skeleton, &decoration_shared_info[decoration].shared_vbo[0]);
			SpineManager::reset_vbo(skeleton, &decoration_shared_info[decoration].shared_vbo[1]);
			SpineManager::reset_vbo(skeleton, &decoration_shared_info[decoration].shared_vbo[2]);
		}

		decoration_shared_info[decoration].initialized = true;
	}

	deleted = false;
	tree_variation = rand() % 3;
	megatron_vertex_pointer[decoration] = 0;
	megatron_vbo[decoration] = t_VBO();
	ref_to_megatron = &megatron_vbo[decoration];
	ref_to_megatron_vertex_pointer = &megatron_vertex_pointer[decoration];
	ref_to_shared_vbo = &decoration_shared_info[decoration].shared_vbo[tree_variation];
	ref_to_vertex_map = &all_verticies[skeleton_name];
	draw_offset = t_vertex(0.0, 0.0, 0.0);
	draw_position = position;
	this->position = position;
	texture[skeleton_name] = ref_to_shared_vbo->texture;
	spine_initialized = true;
	make_all_totals();
}

void FOWDecoration::make_totals()
{
	if (!deleted)
	{
		for (int i = 0; i < ref_to_shared_vbo->num_faces * 3; i += 3)
		{
			ref_to_megatron->verticies[*ref_to_megatron_vertex_pointer] = ref_to_shared_vbo->verticies[i] + draw_position.x;
			ref_to_megatron->verticies[*ref_to_megatron_vertex_pointer + 1] = ref_to_shared_vbo->verticies[i + 1] - draw_position.y;
			ref_to_megatron->verticies[*ref_to_megatron_vertex_pointer + 2] = 1 - (draw_position.y / GridManager::size.y);
			*ref_to_megatron_vertex_pointer += 3;
		}
	}
}

void FOWDecoration::delete_decoration()
{
	deleted = true;
	total_num_faces[skeleton_name] -= ref_to_shared_vbo->num_faces;
}

void FOWDecoration::make_all_totals()
{
	t_VBO the_vbo = get_vbo();
	total_num_faces[skeleton_name] += get_vbo().num_faces;
	for (int i = 0; i < get_vbo().num_faces * 3; i += 3)
	{
		all_verticies[skeleton_name].push_back(the_vbo.verticies[i] + draw_position.x);
		all_verticies[skeleton_name].push_back(the_vbo.verticies[i + 1] - draw_position.y);
		all_verticies[skeleton_name].push_back(the_vbo.verticies[i + 2]);
	}
	for (int i = 0; i < get_vbo().num_faces * 2; i++)
	{
		all_texcoords[skeleton_name].push_back(the_vbo.texcoords[i]);
	}
	for (int i = 0; i < get_vbo().num_faces * 3; i++)
	{
		all_colors[skeleton_name].push_back(the_vbo.colors[i]);
	}
}

void FOWDecoration::clear_totals(std::string decoration_name)
{
	all_verticies[decoration_name].clear();
	all_texcoords[decoration_name].clear();
	all_colors[decoration_name].clear();
	megatron_vertex_pointer[decoration_name] = 0;
}

void FOWDecoration::assemble_megatron(std::string decoration_name)
{
	megatron_vbo[decoration_name].num_faces = total_num_faces[decoration_name];
	megatron_vbo[decoration_name].texture = texture[decoration_name];

	megatron_vbo[decoration_name].verticies = std::shared_ptr<float[]>(new float[megatron_vbo[decoration_name].num_faces * 3]);
	megatron_vbo[decoration_name].colors = std::shared_ptr<float[]>(new float[megatron_vbo[decoration_name].num_faces * 3]);
	megatron_vbo[decoration_name].texcoords = std::shared_ptr<float[]>(new float[megatron_vbo[decoration_name].num_faces * 2]);

	int i = 0;
	for (auto vertex : all_verticies[decoration_name])
	{
		megatron_vbo[decoration_name].verticies[i] = vertex;
		i++;
	}
	i = 0;
	for (auto color : all_colors[decoration_name])
	{
		megatron_vbo[decoration_name].colors[i] = color;
		i++;
	}
	i = 0;
	for (auto texcoord : all_texcoords[decoration_name])
	{
		megatron_vbo[decoration_name].texcoords[i] = texcoord;
		i++;
	}

	PaintBrush::generate_vbo(megatron_vbo[decoration_name]);
	PaintBrush::bind_vbo(megatron_vbo[decoration_name]);
	megatron_vbo[decoration_name].shader = PaintBrush::get_shader("decorations");
}

void FOWDecoration::update_megatron(std::string decoration_name)
{
	int i = 0;
	
	t_VBO* ref_to_vbo = &megatron_vbo[decoration_name];

	glBindBuffer(GL_ARRAY_BUFFER, ref_to_vbo->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ref_to_vbo->num_faces * 3, ref_to_vbo->verticies.get(), GL_DYNAMIC_DRAW);
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
		PaintBrush::transform_model_matrix(ref_to_shared_vbo->shader, glm::vec3(draw_position.x + draw_offset.x, -draw_position.y + draw_offset.y, 0.0), glm::vec4(0), glm::vec3(1));
		PaintBrush::draw_vao(*ref_to_shared_vbo);
		PaintBrush::reset_model_matrix();
	}
}