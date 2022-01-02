
#include "fow_selectable.h"

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
	return (type == FOW_GATHERER || type == FOW_KNIGHT || type == FOW_SKELETON);
}

void FOWSelectable::draw_selection_box()
{
	glPushMatrix();
	if (team_id == 0)
		glColor3f(0.5f, 1.0f, 0.5f);
	else
		glColor3f(1.0f, 0.0f, 0.0f);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glVertex3f((draw_position.x) - 0.5, -(draw_position.y) - 0.5 - size + 1, 0.01f);
	glVertex3f((draw_position.x) - 0.5, -(draw_position.y) + 0.5, 0.01f);
	glVertex3f((draw_position.x) - 0.5, -(draw_position.y) - 0.5 - size + 1, 0.01f);
	glVertex3f((draw_position.x) + 0.5 + size - 1, -(draw_position.y) - 0.5 - size + 1, 0.01f);
	glVertex3f((draw_position.x) - 0.5, -(draw_position.y) + 0.5, 0.01f);
	glVertex3f((draw_position.x) + 0.5 + size - 1, -(draw_position.y) + 0.5, 0.01f);
	glVertex3f((draw_position.x) + 0.5 + size - 1, -(draw_position.y) - 0.5 - size + 1, 0.01f);
	glVertex3f((draw_position.x) + 0.5 + size - 1, -(draw_position.y) + 0.5, 0.01f);
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glPopMatrix();
}

// this is probably cacheable if it becomes a problem
std::vector<t_tile> FOWSelectable::get_adjacent_tiles(bool position_empty)
{
	std::vector<t_tile> adjacent_tiles;
	int i, j;
	for (i = position.x - 1; i < position.x + (size + 1); i++)
		for (j = position.y - 1; j < position.y + (size + 1); j++)
			if ((i == position.x - 1 || i == position.x + (size + 1) || j == position.y - 1 || position.y + (size + 1)) && (grid_manager->tile_map[i][j].entity_on_position == nullptr || position_empty == false))
				adjacent_tiles.push_back(grid_manager->tile_map[i][j]);

	return adjacent_tiles;
}


void FOWSelectable::dirty_tile_map()
{
	int i, j;
	for (i = position.x; i < position.x + (size); i++)
		for (j = position.y; j < position.y + (size); j++)
			grid_manager->tile_map[i][j].entity_on_position = this;
}

void FOWSelectable::take_damage(int amount) {

	printf("selectable called\n");
};