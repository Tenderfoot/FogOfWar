
#include <cmath>
#include <iostream>
#include <filesystem>
#include "main_menu.h"
#include "Settings.h"
#include "fow_player.h"

extern Settings user_settings;
std::string MainMenu::current_menu;

MainMenu::MainMenu()
{
	populate_maps();
	menu_map["Main Menu"] = UIMenu(std::vector<std::string>({"Single Player", "Multiplayer", "Settings", "Quit"}), select_callback);
	menu_map["Single Player"] =  UIMenu(std::vector<std::string>({ "Maplist", "Main Menu"}), select_callback);
	menu_map["Multiplayer"] =  UIMenu(std::vector<std::string>({ "Host Game", "Join Game", "Main Menu" }), select_callback);
	menu_map["Settings"] =  UIMenu(std::vector<std::string>({ "Resolution", "Full Screen", "Music Volume", "SFX Volume", "Main Menu" }), select_callback);
	current_menu = "Main Menu";

	populate_maps();

	UserInterface::add_widget(&menu_map[current_menu]);
	complete = false;
	selected = 0;
}

void MainMenu::populate_maps()
{
	std::string path = "./data/maps";
	menu_map["Single Player"].menu_options.clear();

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		menu_map["Single Player"].menu_options.push_back(entry.path().filename().string());
	}

	menu_map["Single Player"].menu_options.push_back("Main Menu");
}

void MainMenu::select_callback(std::string selected)
{
	if(selected.compare("Quit") == 0)
	{
		exit(1);
	}
	if (selected.compare("Single Player") == 0 || selected.compare("Multiplayer") == 0 || selected.compare("Settings") == 0 || selected.compare("Main Menu") == 0)
	{
		current_menu = selected;
	}
}

void MainMenu::draw()
{

	UserInterface::widgets.clear();
	UserInterface::add_widget(&menu_map[current_menu]);
	UserInterface::draw();
}

void MainMenu::take_input(SDL_Keycode input, bool keydown)
{
}