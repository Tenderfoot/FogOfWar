
#include "fow_character.h"
#include "fow_building.h"
#include "fow_editor.h"
#include "undead.h"
#include "knight.h"
#include "gatherer.h"

FOWEditor::FOWEditor()
{
	editor_mode = MODE_PLACE;
	placemode = PLACE_BUILDING;
	building = nullptr;
	character = nullptr;

	character_type = 0;
	character_types = { FOW_KNIGHT, FOW_GATHERER, FOW_SKELETON };
	building_type = 0;
	building_types = { FOW_TOWNHALL, FOW_GOLDMINE, FOW_FARM };

	placing_characters = true;
}

void FOWEditor::update(float time_delta)
{
	if (blob_droppin)
	{
		grid_manager->dropblob(grid_manager->mouse_x, grid_manager->mouse_y, blobtype);
		grid_manager->cull_orphans();
		grid_manager->calc_all_tiles();
	}
	FOWPlayer::update(time_delta);
}

void FOWEditor::take_input(SDL_Keycode input, bool type)
{
	camera_input(input, type);

	if (input == MIDDLEMOUSE && type == true)
	{
		grid_manager->save_map("data/test.json");
	}

	if (keymap[EDITOR_SWITCH_MODE] == input && type == true)
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

	if (editor_mode == MODE_PAINT)
		take_paint_input(input, type);
	else
		take_place_input(input, type);

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

	if (keymap[PAGE_DOWN] == input && type == true)
	{
		grid_manager->use_tex = !grid_manager->use_tex;
	}

	/*if (keymap[ACTION] == input && type == true)
	{
		grid_manager->randomize_map();
	}*/

	if (keymap[PAGE_UP] == input && type == true)
	{
		printf("hit here in blobtype\n");
		blobtype++;
		blobtype = blobtype % 5;
	}


	if (input == SDLK_2)
	{
		grid_manager->height--;
	}

	if (input == SDLK_8)
	{
		grid_manager->height++;
		for (int i = 0; i < grid_manager->width; i++)
		{
			new_tile = new t_tile();
			new_tile->x = i;
			new_tile->y = grid_manager->height - 1;
			new_tile->gscore = INFINITY;
			new_tile->fscore = INFINITY;
			new_tile->entity_on_position = nullptr;
			new_tile->type = 0;
			grid_manager->tile_map[i][grid_manager->height - 1] = *new_tile;
		}
	}

	if (input == SDLK_4)
	{
		grid_manager->width--;
	}

	if (input == SDLK_6)
	{
		grid_manager->width++;
		for (int i = 0; i < grid_manager->height; i++)
		{
			new_tile = new t_tile();
			new_tile->x = grid_manager->width - 1;
			new_tile->y = i;
			new_tile->gscore = INFINITY;
			new_tile->fscore = INFINITY;
			new_tile->entity_on_position = nullptr;
			new_tile->type = 0;
			grid_manager->tile_map[grid_manager->width - 1][i] = *new_tile;
		}
	}

}

void FOWEditor::init()
{
	if (character == nullptr)
		character = new FOWKnight(t_vertex(0,0,0));
	if (building == nullptr)
		building = new FOWTownHall(0,0,3);
}

void FOWEditor::draw()
{
	if (editor_mode == MODE_PLACE)
	{
		if (placing_characters)
		{
			character->position = t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0);
			character->draw_position = t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0);
			character->draw();
		}
		else
		{
			building->position = t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0);
			building->draw_position = t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0);
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
			building->reset_skin();
		}
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
		if (placing_characters)
		{
			FOWKnight* new_knight = nullptr;
			FOWGatherer* new_gatherer = nullptr;
			FOWUndead* new_undead = nullptr;

			switch (character_types.at(character_type))
			{
			case FOW_KNIGHT:
				new_knight = new FOWKnight(t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0));
				new_knight->team_id = 0;
				grid_manager->entities->push_back(new_knight);
				break;
			case FOW_GATHERER:
				new_gatherer = new FOWGatherer(t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0));
				new_gatherer->team_id = 0;
				grid_manager->entities->push_back(new_gatherer);
				break;
			case FOW_SKELETON:
				new_undead = new FOWUndead(t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0));
				new_undead->team_id = 1;
				grid_manager->entities->push_back(new_undead);
				break;
			}
		}
		else
		{
			FOWTownHall* new_townhall = nullptr;
			FOWGoldMine* new_goldmine = nullptr;
			FOWFarm* new_farm = nullptr;

			switch (building_types.at(building_type))
			{
			case FOW_TOWNHALL:
				new_townhall = new FOWTownHall(grid_manager->mouse_x, grid_manager->mouse_y, 3);
				new_townhall->team_id = 0;
				new_townhall->dirty_tile_map();
				grid_manager->entities->push_back(new_townhall);
				break;
			case FOW_GOLDMINE:
				new_goldmine = new FOWGoldMine(grid_manager->mouse_x, grid_manager->mouse_y, 3);
				new_goldmine->team_id = 0;
				new_goldmine->dirty_tile_map();
				grid_manager->entities->push_back(new_goldmine);
				break;
			case FOW_FARM:
				new_farm = new FOWFarm(grid_manager->mouse_x, grid_manager->mouse_y, 2);
				new_farm->team_id = 0;
				new_farm->dirty_tile_map();
				grid_manager->entities->push_back(new_farm);
				break;
			}
		}
	}
}