
#include "fow_character.h"
#include "fow_building.h"
#include "fow_editor.h"
#include "undead.h"

FOWEditor::FOWEditor()
{
	editor_mode = MODE_PLACE;
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


void FOWEditor::take_place_input(SDL_Keycode input, bool type)
{
	if (input == LMOUSE && type == true)
	{
		// place a skeleton
		FOWUndead* new_undead = new FOWUndead(t_vertex(grid_manager->mouse_x, grid_manager->mouse_y, 0));
		new_undead->team_id = 1;
		grid_manager->entities->push_back(new_undead);
	}

}