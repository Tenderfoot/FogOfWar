#pragma once

#include "fow_player.h"

class FOWEditor : public FOWPlayer
{
public:
	FOWEditor();

	void take_input(SDL_Keycode input, bool type);
	void update(float time_delta);

	int blobtype;
	bool blob_droppin;	// I like to have fun sometimes
};