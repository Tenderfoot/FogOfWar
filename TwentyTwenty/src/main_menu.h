
#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include <string>
#include <map>
#include "user_interface.h"

typedef enum
{
	NETWORK_NONE,
	NETWORK_CLIENT,
	NETWORK_SERVER
}t_networktype;

class MainMenu
{
public:
	// constructor / destructor
	MainMenu();
	
	// methods
	void populate_maps();
	void draw();
	void take_input(SDL_Keycode input, bool keydown);
	static void select_callback(std::string selected);

	// variables
	UIMenu *main_menu;
	std::map<std::string, UIMenu> menu_map;
	static bool complete;
	int selected;
	static std::string current_menu;
	static std::string selected_map;
	static t_networktype network_type;
};