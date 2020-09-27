
#pragma comment(lib, "SDL2")
#pragma comment(lib, "SDL2main")

// some globals
#define USE_FULLSCREEN 1
#define res_width 2560
#define res_height  1440

#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>

SDL_Window* window;

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Init(SDL_INIT_AUDIO);
	SDL_Init(SDL_INIT_JOYSTICK);

	window = SDL_CreateWindow("TwentySixteen", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, res_width, res_height, SDL_WINDOW_OPENGL);

	SDL_Quit();
	return 1;
}