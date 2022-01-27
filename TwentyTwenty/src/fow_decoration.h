#pragma once

#include "spine_entity.h"

class FOWDecoration : public SpineEntity
{
public:

	FOWDecoration();
	FOWDecoration(std::string decoration, t_vertex position);

	static bool initialized;
	static bool animated;
};