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

	static void init();
	static void take_input(SDL_Keycode input, bool type);
	static void update(float time_delta);
	static void draw();
	static void take_paint_input(SDL_Keycode input, bool type);
	static void take_place_input(SDL_Keycode input, bool type);

	static t_editormode editor_mode;
	static t_placemode placemode;

	// for choosing in editor place mode
	static int character_type;
	static const std::vector<entity_types> character_types;
	static int building_type;
	static const std::vector<entity_types> building_types;
	static bool placing_characters;
	static int current_placed_team;

	// these two are just skin swapped for the different placement options
	static FOWTownHall *building;
	static FOWKnight *character;

	static tiletype_t blobtype;
	static bool blob_droppin;
};