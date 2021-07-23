
#include "fow_character.h"
#include "fow_building.h"
#include "fow_editor.h"

FOWEditor::FOWEditor()
{
}

void FOWEditor::update()
{
	if (blob_droppin)
	{
		grid_manager->dropblob(grid_manager->mouse_x, grid_manager->mouse_y, blobtype);
		grid_manager->cull_orphans();
		grid_manager->calc_all_tiles();
	}
	FOWPlayer::update();
}

void FOWEditor::take_input(boundinput input, bool type)
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

	if (input == PAGE_DOWN && type == true)
	{
		grid_manager->use_tex = !grid_manager->use_tex;
	}

	if (input == ACTION && type == true)
	{
		grid_manager->randomize_map();
	}
	
	if (input == PAGE_UP && type == true)
	{
		blobtype++;
		blobtype = blobtype % 5;
	}
}