#pragma once

#include "fow_player.h"
#include "fow_building.h"
#include "knight.h"

typedef enum
{
	MODE_PAINT,
	MODE_PLACE
} t_editormode;

typedef enum
{
	PLACE_BUILDING,
	PLACE_CHARACTERS
} t_placemode;

class FOWEditor : public FOWPlayer
{
public:
	FOWEditor();

	void init();
	void take_input(SDL_Keycode input, bool type);
	void update(float time_delta);
	void draw();
	void take_paint_input(SDL_Keycode input, bool type);
	void take_place_input(SDL_Keycode input, bool type);

	t_editormode editor_mode;
	t_placemode placemode;

	// for choosing in editor place mode
	int character_type;
	std::vector<entity_types> character_types;
	int building_type;
	std::vector<entity_types> building_types;
	bool placing_characters;

	// these two are just skin swapped for the different placement options
	FOWTownHall *building;
	FOWKnight *character;

	int blobtype;
	bool blob_droppin;	// I like to have fun sometimes
};