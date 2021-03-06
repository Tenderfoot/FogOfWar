
#pragma comment(lib, "SDL2")
#pragma comment(lib, "SDL2main")
#pragma comment(lib, "SDL2_mixer")
#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32")
#pragma comment(lib, "SDL2_ttf")
#pragma comment(lib, "lua5.3.5.lib")
#pragma comment(lib, "SDL2_net.lib")

#define NO_SDL_GLEXT

#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <string>
#include <map>
#include <fstream>
#include "SOIL.h"
#include <lua/lua.hpp>
#include <SDL_net/SDL_net.h>

// GLU is deprecated and I should look into removing it - only used by gluPerspective
#include <gl/GLU.h>

#include "json.hpp"
#include "common.h"
#include "game.h"
#include "audiocontroller.h"
#include "settings.h"
#include "main_menu.h"
#include "server_handler.h"
#include "client_handler.h"

SDL_Window* window;
nlohmann::json settings_data;
bool done = false;

Settings user_settings;
lua_State* state;
MainMenu *menu;

extern std::map<boundinput, SDL_Keycode> keymap = {
	{ACTION, SDLK_SPACE},
	{UP, SDLK_UP},
	{LEFT, SDLK_LEFT},
	{RIGHT, SDLK_RIGHT},
	{DOWN, SDLK_DOWN},
	{EDIT_KEY, SDLK_F1},
	{PLAY_KEY, SDLK_F3},
	{SHIFT, SDLK_LSHIFT},
	{CTRL, SDLK_LCTRL},
	{ALT, SDLK_LALT},
	{ESCAPE, SDLK_ESCAPE},
	{SAVE, SDLK_F5},
	{PAGE_UP, SDLK_PAGEUP},
	{PAGE_DOWN, SDLK_PAGEDOWN},
	{BUILD_FARM, SDLK_f},
	{BUILD_BARRACKS, SDLK_b},
	{BUILD_TOWNHALL, SDLK_t},
	{ATTACK_MOVE_MODE, SDLK_a},
	{EDITOR_SWITCH_MODE, SDLK_SPACE},
	{FULLSCREEN, SDLK_F11},
	{TOGGLE_SOUND, SDLK_F8},
	{START_SERVER, SDLK_F5},
	{START_CLIENT, SDLK_F6},
	{DISABLE_SIDESCROLL, SDLK_F4},
	{ENTER_KEY, SDLK_RETURN},
	{BUILD_FOOTMAN, SDLK_f},
	{BUILD_ARCHER, SDLK_r},
	{BUILD_GATHERER, SDLK_g}
};

 
void from_json(const nlohmann::json& j, Settings& s) {
	j.at("width").get_to(s.width);
	j.at("height").get_to(s.height);
	j.at("fullscreen").get_to(s.fullscreen);
	j.at("use_sound").get_to(s.use_sound);
	j.at("use_scroll").get_to(s.use_scroll);
	j.at("host_name").get_to(s.host_name);
}

bool LoadSettings(std::string filename) {
	std::ifstream i(filename);
	i >> settings_data;

	// import settings
	user_settings = settings_data.get<Settings>();

	printf("%dx%d (fullscreen %d) video mode selected\n", user_settings.width, user_settings.height, user_settings.fullscreen);

	return true;
}


void init_opengl()
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glEnable(GL_COLOR_MATERIAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_MULTISAMPLE);
	//glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, user_settings.width, user_settings.height);
	glClearColor(0.05f, 0.05f, 0.05f, 0.5f);

	PaintBrush::setup_extensions();
}


void handle_sdl_event()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT)
		{
			Game::done = true;
		}

		if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
		{
			Game::take_input(event.key.keysym.sym, event.type == SDL_KEYDOWN);

			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				Game::done = true;
			}

		}

		if (event.type == SDL_MOUSEMOTION)
		{
			Game::raw_mouse_position.x = (float)event.motion.x;
			Game::raw_mouse_position.y = (float)event.motion.y;
			Game::relative_mouse_position.x = (float)event.motion.xrel;
			Game::relative_mouse_position.y = -(float)event.motion.yrel;
		}

		if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
		{
			bool keydown = event.type == SDL_MOUSEBUTTONDOWN;
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				Game::take_input(LMOUSE, keydown);
			}

			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				Game::take_input(RMOUSE, keydown);
			}

			if (event.button.button == SDL_BUTTON_MIDDLE)
			{
				Game::take_input(MIDDLEMOUSE, keydown);
			}
		}

		if (event.type == SDL_MOUSEWHEEL)
		{
			if (event.wheel.y > 0)
				Game::take_input(MWHEELUP, true);

			if (event.wheel.y < 0)
				Game::take_input(MWHEELDOWN, true);
		}
	}

	// not an SDL event but we'll put this here for now
	if (user_settings.isDirty())
	{
		if (window != nullptr)
		{
			SDL_SetWindowFullscreen(window, (SDL_WINDOW_FULLSCREEN & user_settings.fullscreen));
			user_settings.clearDirty();
			save_settings_to_file(user_settings, DEFAULT_SETTINGS_PATH);
		}
	}
}

void draw()
{
	// This initial draw is just to get mouse coordinates
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity(); 

	Game::draw_plane();
	Game::get_mouse_in_space();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();

	if (!menu->complete)
	{
		menu->draw();
	}
	else
	{
		Game::draw();
		Game::draw_ui();
	}

	SDL_GL_SwapWindow(window);
}

int main(int argc, char* argv[])
{
	printf("Version pre-alpha 0.3\n");
	printf("OpenGL version %S\n", glGetString(GL_VERSION));
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Init(SDL_INIT_AUDIO);
	SDL_Init(SDL_INIT_JOYSTICK);

	if (TTF_Init() == -1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}

	/* Initialize the network */
	if (SDLNet_Init() < 0) {
		fprintf(stderr, "Couldn't initialize net: %s\n",
			SDLNet_GetError());
		SDL_Quit();
		exit(1);
	}

	// load user settings
	LoadSettings(DEFAULT_SETTINGS_PATH);

	// Initialize LUA
	state = luaL_newstate();
	luaL_openlibs(state);

	// Initialize Audio
	AudioController::init();

	window = SDL_CreateWindow("TwentyTwenty", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, user_settings.width, user_settings.height, SDL_WINDOW_OPENGL | (SDL_WINDOW_FULLSCREEN & user_settings.fullscreen));
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);

	init_opengl();

	menu = new MainMenu();

	// menu stuff
	while(!menu->complete)
	{
		// the only reason this works right now is because
		// game, before being initialized, is acting as an input
		// passthrough for the UI elements and the menu
		// please fix
		handle_sdl_event();
		draw();
	}

	// get the menu off the screen
	UserInterface::widgets.clear();

	// If we chose to be a client, we need to get the map from the server
	if (menu->network_type == NETWORK_CLIENT)
	{
		ClientHandler::init();

		while (ClientHandler::mapname.compare("") == 0)
		{
			printf("waiting...\n");
		}

		// this is kind of a hack
		menu->selected_map = ClientHandler::mapname;
	}

	// Initialize the game with the selected map
	if (!Game::init(menu->selected_map))
	{
		exit(0);
	}

	// Start the server up if we're the server
	if (menu->network_type == NETWORK_SERVER)
	{
		ServerHandler::init();
	}

	float previous_time = SDL_GetTicks();
	while (!Game::done)
	{
		handle_sdl_event();

		float current_time = SDL_GetTicks();
		Game::run((current_time - previous_time) / 1000);
		previous_time = current_time;

		draw();
	}

	lua_close(state);

	/*if (servsock != NULL) {
		SDLNet_TCP_Close(servsock);
		servsock = NULL;
	}
	if (socketset != NULL) {
		SDLNet_FreeSocketSet(socketset);
		socketset = NULL;
	}*/

	SDLNet_Quit();
	TTF_Quit();
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	shutdown_settings_thread();

	return 1;
}