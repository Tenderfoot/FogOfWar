#pragma once

#include "fow_decoration.h"
std::map<std::string, t_decoration_sharedinfo> FOWDecoration::decoration_shared_info;

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

	spine_initialized = true;
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

void FOWDecoration::draw()
{
	if (visible)
	{
		glPushMatrix();
		glTranslatef(draw_position.x + draw_offset.x, -draw_position.y + draw_offset.y, 0.1f);
		PaintBrush::draw_vbo(*ref_to_shared_vbo);
		glPopMatrix();
	}
}