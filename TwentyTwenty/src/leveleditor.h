#pragma once

#include "common.h"
#include "entity.h"

class LevelEditor
{
public:

	LevelEditor() {
		camera_transform.x = 0;
		camera_transform.y = 0;
	}

	t_transform camera_transform;
	void take_input(boundinput input, bool keydown);
};