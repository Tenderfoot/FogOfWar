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

	if (decoration_shared_info.find(decoration) == decoration_shared_info.end())
	{
		decoration_shared_info[decoration] = t_decoration_sharedinfo();
	}

	if (decoration_shared_info[decoration].initialized == false)
	{
		build_spine();
		decoration_shared_info[decoration].shared_vbo = VBO;
		decoration_shared_info[decoration].shared_animationState = animationState;
		decoration_shared_info[decoration].shared_skeleton = skeleton;
		decoration_shared_info[decoration].initialized = true;
	}
	skeleton = decoration_shared_info[decoration].shared_skeleton;
	decoration_shared_info[decoration].shared_animationState->addAnimation(0, "animation", true, 0);
	draw_offset = t_vertex(0.0, 0.0, 0.0);
	draw_position = position;
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
		printf("hit here\n");
		// this dirty_vbo stuff works when I put it here
		// but failed where I intended to put it which was draw
		// I'm not 100% sure why
		if (dirty_vbo == true)
		{
			SpineManager::reset_vbo(decoration_shared_info[skeleton_name].shared_skeleton, &decoration_shared_info[skeleton_name].shared_vbo);
			dirty_vbo = false;
		}

		decoration_shared_info[skeleton_name].shared_animationState->update(delta_time);
		decoration_shared_info[skeleton_name].shared_animationState->apply(*skeleton);

		SpineManager::update_vbo(skeleton, &decoration_shared_info[skeleton_name].shared_vbo);
		decoration_shared_info[skeleton_name].animated = true;
	}
}

void FOWDecoration::draw()
{
	if (visible)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glPushMatrix();
		glTranslatef(draw_position.x + draw_offset.x, -draw_position.y + draw_offset.y, 0.1f);
		if (flip)
			glRotatef(180, 0.0f, 1.0f, 0.0f);
		PaintBrush::draw_vbo(decoration_shared_info[skeleton_name].shared_vbo);
		glPopMatrix();
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
}