#pragma once

#include "common.h"
#include "game_entity.h"
#include "grid_manager.h"

// This stuffb belongs in UI but is being placed here for merge
class UIWidget
{
public:
	UIWidget()
	{
		visible = true;
		absorbs_mouse = false;
	}

	float x, y, width, height;
	bool visible;
	bool absorbs_mouse;

	virtual bool coords_in_ui(t_vertex mousecoords)
	{
		float res_width = 2560;
		float res_height = 1440;

		if (mousecoords.x > this->x * res_width - (0.5 * this->width * res_width) && mousecoords.x < this->x * res_width + (0.5 * this->width * res_width) &&
			mousecoords.y > this->y * res_height - (0.5 * this->height * res_height) && mousecoords.y < this->y * res_height + (0.5 * this->height * res_height) && absorbs_mouse)
		{
			return true;
		}
		return false;
	}

	virtual void click_at_location(t_vertex mousecoords)
	{

	}

	t_vertex color;
	int index;

	virtual void draw() = 0;
};

class GreenBox : public UIWidget
{
public:
	GreenBox()
	{
		visible = true;
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}

	bool visible;
	void draw()
	{
		if (visible)
		{
			glDisable(GL_TEXTURE_2D);
			glColor3f(0.5f, 1.0f, 0.5f);
			glLineWidth(2.0f);
			glBegin(GL_LINES);
			glVertex2f(x, y);
			glVertex2f(width, y);
			glVertex2f(x, y);
			glVertex2f(x, height);
			glVertex2f(width, y);
			glVertex2f(width, height);
			glVertex2f(x, height);
			glVertex2f(width, height);
			glEnd();
			glEnable(GL_TEXTURE_2D);
			glColor3f(1.0f, 1.0f, 1.0f);
		}
	}

	t_vertex mouse_in_space;
};

class FOWSelectable;

class FOWPlayer
{
public:

	FOWPlayer();

	// FOWPlayer for this one
	void get_selection(t_vertex tile_space, t_vertex tile_end);
	virtual void take_input(boundinput input, bool type);

	bool is_selectable(entity_types type)
	{
		return (type == FOW_CHARACTER || type == FOW_GATHERER || type == FOW_BUILDING || type == FOW_TOWNHALL || type == FOW_GOLDMINE);
	}

	void draw();
	virtual void update();
	void draw_selections();

	int gold;

	t_vertex gridstart_ui;
	t_vertex gridstart_world;
	t_vertex mouse_in_space;
	t_vertex camera_pos;
	float camera_distance;
	std::vector<FOWSelectable*> selection_group;
	GreenBox *green_box;

	t_transform mouse_pos;

	float last_poor_warning;

	FOWSelectable *selection;

	std::vector<GameEntity*> *entities;
	GridManager *grid_manager;

	bool queue_add_toggle;

};