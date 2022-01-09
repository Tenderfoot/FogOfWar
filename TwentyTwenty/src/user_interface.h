#pragma once

#include <vector>
#include "grid_manager.h"

class GridManager;
class UserInterface;

class UIWidget
{
public:
	UIWidget()
	{
		visible = true;
		absorbs_mouse = false;
		draw_2d = false;
	}

	//virtual bool coords_in_ui(t_vertex mousecoords);
	//virtual void click_at_location(t_vertex mousecoords);
	virtual void draw() = 0;
	virtual void take_input(SDL_Keycode input, bool keydown) {};

	// for 2D poisitions we're just using t_vertex and the first two fields
	t_vertex position;
	t_vertex size;
	bool visible;
	bool absorbs_mouse;
	t_vertex color;
	bool draw_2d;	// use othographic projection?
};


class GreenBox : public UIWidget
{
public:
	GreenBox();

	bool visible;
	void draw();
	t_vertex mouse_in_space;
};


class MapWidget : public UIWidget
{
public:
	MapWidget();

	std::map<tiletype_t, t_vertex> type_to_color;
	GridManager* map_grid;
	bool visible;
	bool mouse_down;	// track while the mouse button is held down
	void draw();
	virtual void take_input(SDL_Keycode input, bool keydown);
};

class UserInterface
{
public:

	static std::vector<UIWidget*> widgets;
	static GridManager* grid_manager;
	t_vertex mouse_coords;

	static void add_widget(UIWidget* new_widget);
	void mouse_focus();
	static void take_input(SDL_Keycode input, bool keydown);
	static void draw();
};