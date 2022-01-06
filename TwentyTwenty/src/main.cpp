
#pragma comment(lib, "SDL2")
#pragma comment(lib, "SDL2main")
#pragma comment(lib, "SDL2_mixer")
#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32")

// Next few goals
// the 0.5 offset problem
// debugging mining

#define NO_SDL_GLEXT

#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <string>
#include <map>
#include <fstream>
#include "SOIL.h"
// GLU is deprecated and I should look into removing it - only used by gluPerspective
#include <gl/GLU.h>

#include "json.hpp"
#include "common.h"
#include "game.h"
#include "audiocontroller.h"

SDL_Window* window;
Game witch_game;
nlohmann::json settings_data;
bool done = false;

class Settings
{
public:
	int width, height;
	int fullscreen;
};

Settings user_settings;

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
	{EDITOR_SWITCH_MODE, SDLK_SPACE}

};

 
void from_json(const nlohmann::json& j, Settings& s) {
	j.at("width").get_to(s.width);
	j.at("height").get_to(s.height);
	j.at("fullscreen").get_to(s.fullscreen);
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
	glMatrixMode(GL_PROJECTION);  // Select The Projection Matrix
	glLoadIdentity();                // Reset The Projection Matrix
	
	// This is deprecated (glu.h)
	gluPerspective(90, (float)user_settings.width / (float)user_settings.height, 0.1, 1000.0);

	glMatrixMode(GL_MODELVIEW);  // Select The Model View Matrix
	glLoadIdentity();    // Reset The Model View Matrix

	glClearColor(0.05f, 0.05f, 0.05f, 0.5f);

}


void handle_sdl_event()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT)
		{
			done = true;
		}

		if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
		{
			witch_game.take_input(event.key.keysym.sym, event.type == SDL_KEYDOWN);
		}

		if (event.type == SDL_MOUSEMOTION)
		{
			witch_game.raw_mouse_position.x = (float)event.motion.x;
			witch_game.raw_mouse_position.y = (float)event.motion.y;
			witch_game.player.raw_mouse_position = witch_game.raw_mouse_position;
			witch_game.player.screen = t_transform(user_settings.width, user_settings.height, 0, 0);
			witch_game.relative_mouse_position.x = (float)event.motion.xrel;
			witch_game.relative_mouse_position.y = -(float)event.motion.yrel;
		}

		if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
		{
			bool keydown = event.type == SDL_MOUSEBUTTONDOWN;
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				witch_game.take_input(LMOUSE, keydown);
			}

			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				witch_game.take_input(RMOUSE, keydown);
			}

			if (event.button.button == SDL_BUTTON_MIDDLE)
			{
				witch_game.take_input(MIDDLEMOUSE, keydown);
			}
		}

		if (event.type == SDL_MOUSEWHEEL)
		{
			if (event.wheel.y > 0)
				witch_game.take_input(MWHEELUP, true);

			if (event.wheel.y < 0)
				witch_game.take_input(MWHEELDOWN, true);
		}
	}
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();

	witch_game.draw();
	witch_game.get_mouse_in_space();

	SDL_GL_SwapWindow(window);
}

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Init(SDL_INIT_AUDIO);
	SDL_Init(SDL_INIT_JOYSTICK);

	LoadSettings("data/settings.json");

	AudioController::init();

	window = SDL_CreateWindow("TwentyTwenty", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, user_settings.width, user_settings.height, SDL_WINDOW_OPENGL | (SDL_WINDOW_FULLSCREEN & user_settings.fullscreen));
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);

	init_opengl();

	if (!witch_game.init())
	{
		exit(0);
	}

	float previous_time = SDL_GetTicks();
	while (!done) 
	{
		// SDL Events
		handle_sdl_event();
		// Run
		float current_time = SDL_GetTicks();
		witch_game.run((current_time - previous_time)/1000);
		previous_time = current_time;
		// Draw
		draw();
	}

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 1;
}