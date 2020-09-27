
#pragma comment(lib, "SDL2")
#pragma comment(lib, "SDL2main")
#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32")

// To be replaced with settings file
#define USE_FULLSCREEN 1
#define res_width 1920
#define res_height  1080

#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>

// GLU is deprecated and I should look into removing it - only used by gluPerspective
#include <gl/GLU.h>

SDL_Window* window;
bool done = false;


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

	glViewport(0, 0, res_width, res_height);
	glMatrixMode(GL_PROJECTION);  // Select The Projection Matrix
	glLoadIdentity();                // Reset The Projection Matrix
	
	gluPerspective(90, (float)res_width / (float)res_height, 1.0, 1000.0);
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
	}
}


void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();

	glBegin(GL_QUADS);
		glVertex3f(-0.5f, -0.5f, -10.0f);
		glVertex3f(0.5f, -0.5f, -10.0f);
		glVertex3f(0.5f, 0.5f, -10.0f);
		glVertex3f(-0.5f, 0.5f, -10.0f);
	glEnd();

	SDL_GL_SwapWindow(window);
}


int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Init(SDL_INIT_AUDIO);
	SDL_Init(SDL_INIT_JOYSTICK);

	window = SDL_CreateWindow("TwentyTwenty", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, res_width, res_height, SDL_WINDOW_OPENGL);
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);

	init_opengl();

	float previous_time = SDL_GetTicks();
	while (!done) {
		
		handle_sdl_event();

		// Run
		float current_time = SDL_GetTicks();
		// run(current_time - previous_time);
		previous_time = current_time;

		draw();
	}

	SDL_Quit();
	return 1;
}