
#include "fow_selectable.h"
#include "audiocontroller.h"

float FOWSelectable::last_command_sound = 0;

// not clear on why FOWCharacter 
void FOWSelectable::load_spine_data(std::string spine_file, std::string skin_name)
{
	skeleton_name = spine_file;
	skeleton = new spine::Skeleton(SpineManager::skeletonData[spine_file.c_str()]);
	skeleton->setToSetupPose();
	skeleton->updateWorldTransform();
	this->skin_name = skin_name;
	set_skin(skin_name.c_str());
}

void FOWSelectable::process_command(FOWCommand next_command)
{
};

void FOWSelectable::clear_selection()
{
	selected = false;
};

// references to future classes... should just have flags on this class
bool FOWSelectable::is_selectable(entity_types type)
{
	return (type == FOW_CHARACTER || type == FOW_GATHERER || type == FOW_BUILDING || type == FOW_TOWNHALL || type == FOW_GOLDMINE);
}

bool FOWSelectable::is_unit(entity_types type)
{
	return (type == FOW_GATHERER || type == FOW_KNIGHT || type == FOW_SKELETON);
}

bool FOWSelectable::is_unit()
{
	return is_unit(type);
}

void FOWSelectable::draw()
{
	if (selected)
		draw_selection_box();

	SpineEntity::draw();
}

void FOWSelectable::play_audio_queue(t_audiocue audio_cue_type)
{
	std::vector<std::string> *cue_library = nullptr;

	// part two of hack (last_command_sound)
	// again, selection happens on FOWPlayer, and thats where
	// command and selection sounds should happen based on whose selected
	// and who is getting the command
	if (SDL_GetTicks() - last_command_sound < 1000 && audio_cue_type == SOUND_COMMAND)
		return;

	if (audio_cue_type == SOUND_COMMAND)
	{
		last_command_sound = SDL_GetTicks();
	}

	switch (audio_cue_type)
	{
	case SOUND_READY:
		cue_library = &ready_sounds;
		break;
	case SOUND_SELECT:
		cue_library = &select_sounds;
		break;
	case SOUND_COMMAND:
		cue_library = &command_sounds;
		break;
	case SOUND_DEATH:
		cue_library = &death_sounds;
		break;
	}

	if (cue_library->size() > 0)
	{
		AudioController::play_sound(cue_library->at(rand() % cue_library->size()));
	}
}

void FOWSelectable::draw_selection_box()
{
	glPushMatrix();
	if (team_id == 0)
		glColor3f(0.5f, 1.0f, 0.5f);
	else
		glColor3f(1.0f, 0.0f, 0.0f);

	float minx = (draw_position.x) - 0.5;
	float maxx = (draw_position.x) - 0.5;
	float miny = -(draw_position.y) + 0.5;
	float maxy = -(draw_position.y) + 0.5;

	glDisable(GL_TEXTURE_2D);
	glLineWidth(1.0f);
	glBegin(GL_LINES);
		glVertex3f(minx, miny - size, 0.01f);
		glVertex3f(minx, maxy, 0.01f);
		glVertex3f(minx, miny - size, 0.01f);
		glVertex3f(maxx + size, miny - size, 0.01f);
		glVertex3f(minx, maxy, 0.01f);
		glVertex3f(maxx + size, maxy, 0.01f);
		glVertex3f(maxx + size, miny - size, 0.01f);
		glVertex3f(maxx + size, maxy, 0.01f);
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glPopMatrix();
}

// this is probably cacheable if it becomes a problem
std::vector<t_tile> FOWSelectable::get_adjacent_tiles(bool position_empty)
{
	std::vector<t_tile> adjacent_tiles;
	for (int widthItr = position.x - 1; widthItr < position.x + (size + 1); widthItr++)
	{
		for (int heightItr = position.y - 1; heightItr < position.y + (size + 1); heightItr++)
		{
			if ((widthItr == position.x - 1 || widthItr == position.x + (size + 1) || heightItr == position.y - 1 || position.y + (size + 1)) && (GridManager::tile_map[widthItr][heightItr].entity_on_position == nullptr || position_empty == false) && GridManager::tile_map[widthItr][heightItr].wall == 0)
			{
				adjacent_tiles.push_back(GridManager::tile_map[widthItr][heightItr]);
			}
		}
	}

	return adjacent_tiles;
}

void FOWSelectable::select_unit()
{
	selected = true;
}

void FOWSelectable::dirty_tile_map()
{
	for (int widthItr = position.x; widthItr < position.x + (size); widthItr++)
	{
		for (int heightItr = position.y; heightItr < position.y + (size); heightItr++)
		{
			GridManager::tile_map[widthItr][heightItr].entity_on_position = this;
		}
	}
}

void FOWSelectable::take_damage(int amount) 
{
	printf("selectable called\n");
};