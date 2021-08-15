#pragma once

#include "fow_player.h"

typedef enum
{
	MODE_PAINT,
	MODE_PLACE
} t_editormode;

class FOWEditor : public FOWPlayer
{
public:
	FOWEditor();

	void take_input(SDL_Keycode input, bool type);
	void update(float time_delta);
	void take_paint_input(SDL_Keycode input, bool type);
	void take_place_input(SDL_Keycode input, bool type);

	t_editormode editor_mode;

	int blobtype;
	bool blob_droppin;	// I like to have fun sometimes
};