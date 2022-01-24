
#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include <string>
#include <map>
#include "user_interface.h"

class MainMenu
{
public:
	// constructor / destructor
	MainMenu();
	
	// methods
	void draw();
	void take_input(SDL_Keycode input, bool keydown);
	static void select_callback(std::string selected);

	// variables
	UIMenu *main_menu;
	std::map<std::string, UIMenu> menu_map;
	bool complete;
	int selected;
	static std::string current_menu;
};