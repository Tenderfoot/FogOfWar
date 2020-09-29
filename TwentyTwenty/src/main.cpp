
#pragma comment(lib, "SDL2")
#pragma comment(lib, "SDL2main")
#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32")

// Next few goals

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

void from_json(const nlohmann::json& j, Settings& s) {
	j.at("width").get_to(s.width);
	j.at("height").get_to(s.height);
	j.at("fullscreen").get_to(s.fullscreen);
}

std::map<SDL_Keycode, boundinput> keymap ={
		{SDLK_SPACE, ACTION},
		{SDLK_w, UP},
		{SDLK_a, LEFT},
		{SDLK_d, RIGHT},
		{SDLK_s, DOWN},
};

bool LoadSettings(std::string filename) {
	std::ifstream i(filename);
	i >> settings_data;

	// import settings
	user_settings = settings_data.get<Settings>();

	printf("%dx%d (fullscreen %d) video mode selected\n", user_settings.width, user_settings.height, user_settings.fullscreen);

	return true;
}

GLuint Soil_Load_Texture(std::string filename)
{
	GLuint loaded_texture;
	int flags;

	flags = SOIL_FLAG_MIPMAPS;

	loaded_texture = SOIL_load_OGL_texture(filename.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		flags);

	// Make sure texture is set to repeat on wrap
	glBindTexture(GL_TEXTURE_2D, loaded_texture);

	// make sure it doesn't wrap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return loaded_texture;
}


void init_opengl()
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glEnable(GL_COLOR_MATERIAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, user_settings.width, user_settings.height);
	glMatrixMode(GL_PROJECTION);  // Select The Projection Matrix
	glLoadIdentity();                // Reset The Projection Matrix
	
	// This is deprecated (glu.h)
	gluPerspective(90, (float)user_settings.width / (float)user_settings.height, 1.0, 1000.0);

	glMatrixMode(GL_MODELVIEW);  // Select The Model View Matrix
	glLoadIdentity();    // Reset The Model View Matrix

	glClearColor(0.05f, 0.05f, 0.05f, 0.5f);
}


void handle_sdl_event()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
			done = true;

		if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
			witch_game.take_input(keymap[event.key.keysym.sym], event.type == SDL_KEYDOWN);
	}
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();

	witch_game.draw();

	SDL_GL_SwapWindow(window);
}

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Init(SDL_INIT_AUDIO);
	SDL_Init(SDL_INIT_JOYSTICK);

	LoadSettings("data/settings.json");

	window = SDL_CreateWindow("TwentyTwenty", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, user_settings.width, user_settings.height, SDL_WINDOW_OPENGL | (SDL_WINDOW_FULLSCREEN & user_settings.fullscreen));
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);

	init_opengl();

	if (!witch_game.init())
	{
		exit(0);
	}

	float previous_time = SDL_GetTicks();
	while (!done) {
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