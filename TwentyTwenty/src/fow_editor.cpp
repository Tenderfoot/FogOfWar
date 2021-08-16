
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

	if (input == MIDDLEMOUSE && type == true)
	{
		grid_manager->save_map("test");
	}
}

void FOWEditor::init()
{
	if (character == nullptr)
		character = new FOWKnight(t_vertex(0,0,0));
	if (building == nullptr)
		building = new FOWTownHall();
}

void FOWEditor::draw()
{
	if (editor_mode == MODE_PLACE)
	{
		character->visible = true;
		character->position = t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0);
		character->draw_position = t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0);
		character->draw();
	}
	else
		character->visible = false;

}

void FOWEditor::take_place_input(SDL_Keycode input, bool type)
{

	if (keymap[PAGE_UP] == input && type == true)
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
	
	if (input == LMOUSE && type == true)
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
				new_gatherer->team_id = 1;
				grid_manager->entities->push_back(new_gatherer);
				break;
			case FOW_SKELETON:
				new_undead = new FOWUndead(t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0));
				new_undead->team_id = 1;
				grid_manager->entities->push_back(new_undead);
				break;
		}
	}
}