
#include "fow_character.h"
#include "fow_building.h"
#include "fow_editor.h"

FOWEditor::FOWEditor()
{
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

	if (keymap[ACTION] == input && type == true)
	{
		grid_manager->randomize_map();
	}
	
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