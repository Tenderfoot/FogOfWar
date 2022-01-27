#pragma once

#include "fow_decoration.h"

bool FOWDecoration::initialized = false;
bool FOWDecoration::animated = false;

FOWDecoration::FOWDecoration()
{
	FOWDecoration("tree", t_vertex(0, 0, 0));
}

FOWDecoration::FOWDecoration(std::string decoration, t_vertex position)
{
	skeleton_name = decoration;
	skin_name = "";
	build_spine();
	animationState->addAnimation(0, "animation", true, 0);
	draw_offset = t_vertex(0.0, 0.0, 0.0);
	draw_position = position;
}