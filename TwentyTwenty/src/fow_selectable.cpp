
#include "fow_selectable.h"
#include "audiocontroller.h"
#include "fow_player.h"
#include "user_interface.h"

float FOWSelectable::last_command_sound = 0;
UIProgressBar* FOWSelectable::hp_bar;

void FOWSelectable::process_command(FOWCommand next_command)
{
};

void FOWSelectable::clear_selection()
{
	selected = false;
	hp_bar->visible = false;
};

// references to future classes... should just have flags on this class
bool FOWSelectable::is_selectable(entity_types type)
{
	return (type == FOW_CHARACTER || type == FOW_GATHERER || type == FOW_BUILDING || type == FOW_TOWNHALL || type == FOW_GOLDMINE);
}

bool FOWSelectable::is_unit(entity_types type)
{
	return (type == FOW_GATHERER || type == FOW_KNIGHT || type == FOW_SKELETON || type == FOW_ARCHER);
}

bool FOWSelectable::is_unit()
{
	return is_unit(type);
}

void FOWSelectable::draw()
{
	if (selected)
		draw_selection_box();

	// this is for lazy loading the VBO
	// you can't to OpenGL stuff in a thread
	if (spine_initialized == false)
	{
		build_spine();
	}

	PaintBrush::set_uniform(VBO.shader, "team_id", team_id);
	
	SpineEntity::draw();
}


void FOWSelectable::update(float deltatime)
{
	if (selected)
	{
		hp_bar->set_current(current_hp);
	}

	SpineEntity::update(deltatime);
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
	//if (team_id == FOWPlayer::team_id)
	//	glColor3f(0.5f, 1.0f, 0.5f);
	//else
	//	glColor3f(1.0f, 0.0f, 0.0f);

	float minx =  0.5;
	float maxx =  0.5;
	float miny = 0.5;
	float maxy = 0.5;

	PaintBrush::reset_model_matrix();
	PaintBrush::transform_model_matrix(PaintBrush::get_shader("spine"), glm::vec3(draw_position.x - 1, -draw_position.y, 0.0f), glm::vec4(0), glm::vec4(1));

	PaintBrush::use_shader(PaintBrush::get_shader("spine"));
	glDisable(GL_TEXTURE_2D);
	glLineWidth(1.0f);
	glBegin(GL_LINES);
		glVertex3f(minx, miny - size, 1);
		glVertex3f(minx, maxy, 1);
		glVertex3f(minx, miny - size, 1);
		glVertex3f(maxx + size, miny - size, 1);
		glVertex3f(minx, maxy, 1);
		glVertex3f(maxx + size, maxy, 1);
		glVertex3f(maxx + size, miny - size, 1);
		glVertex3f(maxx + size, maxy, 1);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	PaintBrush::reset_model_matrix();
	PaintBrush::stop_shader();
}

// this is probably cacheable if it becomes a problem
std::vector<t_tile> FOWSelectable::get_adjacent_tiles(bool position_empty, bool dont_check_passable)
{
	std::vector<t_tile> adjacent_tiles;
	for (int widthItr = position.x - 1; widthItr < position.x + (size + 1); widthItr++)
	{
		for (int heightItr = position.y - 1; heightItr < position.y + (size + 1); heightItr++)
		{
			if (((widthItr == position.x - 1 || widthItr == position.x + size) || heightItr == position.y - 1 || heightItr == position.y + size) && (GridManager::tile_map[widthItr][heightItr].entity_on_position == nullptr || position_empty == false) && (GridManager::tile_map[widthItr][heightItr].wall == 0 || dont_check_passable))
			{
				adjacent_tiles.push_back(GridManager::tile_map[widthItr][heightItr]);
			}
		}
	}

	return adjacent_tiles;
}

std::vector<t_tile> FOWSelectable::get_adjacent_tiles_from_center(int buffer_size, bool position_empty, bool dont_check_passable)
{
	std::vector<t_tile> adjacent_tiles;
	for (int widthItr = position.x - buffer_size; widthItr <= position.x + (buffer_size); widthItr++)
	{
		for (int heightItr = position.y - buffer_size; heightItr <= position.y + (buffer_size); heightItr++)
		{
			if (widthItr < 0 || heightItr < 0)
				continue;

			if (((widthItr == position.x - buffer_size || widthItr == position.x + buffer_size || heightItr == position.y - buffer_size || heightItr == position.y + buffer_size)) && (GridManager::tile_map[widthItr][heightItr].entity_on_position == nullptr || position_empty == false) && (GridManager::tile_map[widthItr][heightItr].wall == 0 || dont_check_passable))
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
	hp_bar->visible = true;
	hp_bar->current = current_hp;
	hp_bar->maximum = maximum_hp;
}

void FOWSelectable::dirty_tile_map()
{
	int widthItr = 0, heightItr = 0;

	for (widthItr = position.x; widthItr < position.x + (size); widthItr++)
	{
		for (heightItr = position.y; heightItr < position.y + (size); heightItr++)
		{
			GridManager::tile_map[widthItr][heightItr].entity_on_position = this;
		}
	}
}

void FOWSelectable::take_damage(int amount) 
{
	printf("selectable called\n");
};