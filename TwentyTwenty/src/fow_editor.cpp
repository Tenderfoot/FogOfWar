
#include "fow_character.h"
#include "fow_building.h"
#include "fow_editor.h"
#include "undead.h"
#include "knight.h"
#include "gatherer.h"
#include "settings.h"
#include "user_interface.h"
#include "game.h"

const std::vector<entity_types> FOWEditor::character_types = { FOW_KNIGHT, FOW_GATHERER, FOW_SKELETON };
const std::vector<entity_types> FOWEditor::building_types = { FOW_TOWNHALL, FOW_GOLDMINE, FOW_FARM, FOW_ENEMYSPAWNER };
int FOWEditor::character_type;
int FOWEditor::building_type;
bool FOWEditor::placing_characters;
tiletype_t FOWEditor::blobtype;
bool FOWEditor::blob_droppin;
t_editormode FOWEditor::editor_mode;
t_placemode FOWEditor::placemode;
FOWTownHall* FOWEditor::building;
FOWKnight* FOWEditor::character;
int FOWEditor::current_placed_team;

extern Settings user_settings;

FOWEditor::FOWEditor()
{
	editor_mode = MODE_PLACE;
	placemode = PLACE_BUILDING;
	building = nullptr;
	character = nullptr;

	character_type = 0;
	building_type = 0;
	placing_characters = true;
	current_placed_team = 0;
}

void FOWEditor::update(float time_delta)
{
	if (blob_droppin)
	{
		GridManager::dropblob(Game::coord_mouse_position.x, Game::coord_mouse_position.y, blobtype);
		GridManager::cull_orphans();
		GridManager::calc_all_tiles();
	}

	FOWPlayer::update(time_delta);
}

void FOWEditor::take_input(SDL_Keycode input, bool key_down)
{
	camera_input(input, key_down);

	if (input == MIDDLEMOUSE && key_down == true)
	{
		printf("Saving %d %d\n", GridManager::size.x, GridManager::size.y);
		GridManager::save_map("data/test.json");
	}

	if (keymap[EDITOR_SWITCH_MODE] == input && key_down == true)
	{
		if (editor_mode == MODE_PAINT)
		{
			editor_mode = MODE_PLACE;
			printf("Entered place mode!\n");
		}
		else
		{
			editor_mode = MODE_PAINT;
			printf("Entered paint mode!\n");
		}
	}

	if (keymap[FULLSCREEN] == input && key_down == true)
	{
		user_settings.toggleFullScreen();
	}

	if (editor_mode == MODE_PAINT)
	{
		take_paint_input(input, key_down);
	}
	else
	{
		take_place_input(input, key_down);
	}
}

void FOWEditor::take_paint_input(SDL_Keycode input, bool type)
{
	t_tile *new_tile;

	if (input == LMOUSE && type == true)
	{
		blob_droppin = true;
	}

	if (input == LMOUSE && type == false)
	{
		green_box->visible = false;
		blob_droppin = false;
	}

	/*if (keymap[ACTION] == input && type == true)
	{
		grid_manager->randomize_map();
	}*/

	if (keymap[PAGE_UP] == input && type == true)
	{
		printf("hit here in blobtype\n");
		blobtype = (tiletype_t)(((int)blobtype)+1);
		blobtype = (tiletype_t)(((int)blobtype % 5));
	}


	if (input == SDLK_2)
	{
		GridManager::size.y--;
	}

	if (input == SDLK_8)
	{
		GridManager::size.y++;
		for (int i = 0; i < GridManager::size.x; i++)
		{
			new_tile = new t_tile();
			new_tile->x = i;
			new_tile->y = GridManager::size.y - 1;
			new_tile->gscore = INFINITY;
			new_tile->fscore = INFINITY;
			new_tile->entity_on_position = nullptr;
			new_tile->type = TILE_DIRT;
			GridManager::tile_map[i][GridManager::size.y - 1] = *new_tile;
		}
	}

	if (input == SDLK_4)
	{
		GridManager::size.x--;
	}

	if (input == SDLK_6)
	{
		GridManager::size.x++;
		for (int i = 0; i < GridManager::size.y; i++)
		{
			new_tile = new t_tile();
			new_tile->x = GridManager::size.x - 1;
			new_tile->y = i;
			new_tile->gscore = INFINITY;
			new_tile->fscore = INFINITY;
			new_tile->entity_on_position = nullptr;
			new_tile->type = TILE_DIRT;
			GridManager::tile_map[GridManager::size.x - 1][i] = *new_tile;
		}
	}
}

void FOWEditor::init()
{
	if (character == nullptr)
	{
		character = new FOWKnight(t_vertex(-1, -1, 0));	// BAD (size being 0 makes it not crash, turns out)
	}
	
	if (building == nullptr)
	{
		building = new FOWTownHall(t_vertex(0,0,0));
	}
}

void FOWEditor::draw()
{
	if (editor_mode == MODE_PLACE)
	{
		if (placing_characters)
		{
			character->position = Game::coord_mouse_position;
			character->draw_position = Game::coord_mouse_position;
			character->team_id = current_placed_team;
			character->draw();
		}
		else
		{
			building->position = Game::coord_mouse_position;
			building->draw_position = Game::coord_mouse_position;
			building->team_id = current_placed_team;
			building->draw();
		}
	}
}

void FOWEditor::take_place_input(SDL_Keycode input, bool type)
{

	if (keymap[PAGE_UP] == input && type == true)
	{
		if (placing_characters)
		{
			character_type++;
			character_type = character_type % character_types.size();
			entity_types current_type = character_types.at(character_type);
			if (current_type == FOW_GATHERER)
				character->skin_name = "farm";
			if (current_type == FOW_KNIGHT)
				character->skin_name = "knight";
			if (current_type == FOW_SKELETON)
				character->skin_name = "skel";
			character->reset_skin();
			SpineManager::reset_vbo(character->skeleton, &character->VBO);
		}
		else
		{
			building_type++;
			building_type = building_type % building_types.size();
			entity_types current_type = building_types.at(building_type);
			if (current_type == FOW_TOWNHALL)
				building->skin_name = "TownHall";
			if (current_type == FOW_GOLDMINE)
				building->skin_name = "GoldMine";
			if (current_type == FOW_FARM)
				building->skin_name = "Farm";
			if (current_type == FOW_ENEMYSPAWNER)
				building->skin_name = "Barracks";
			building->reset_skin();
			SpineManager::reset_vbo(building->skeleton, &building->VBO);
		}
	}

	if (input == SDLK_HOME && type == true)
	{
		current_placed_team = (current_placed_team + 1) % 3;
	}

	if (input == SDLK_c)
	{
		placing_characters = true;
	}

	if (input == SDLK_b)
	{
		placing_characters = false;
	}
	
	if (input == LMOUSE && type == true)
	{
		FOWSelectable* new_selectable = nullptr;

		if (placing_characters)
		{
			new_selectable = ((FOWCharacter*)GridManager::build_and_add_entity(character_types.at(character_type), Game::coord_mouse_position));
		}
		else
		{
			new_selectable = ((FOWBuilding*)GridManager::build_and_add_entity(building_types.at(building_type), Game::coord_mouse_position));
		}

		new_selectable->team_id = current_placed_team;
	}
}