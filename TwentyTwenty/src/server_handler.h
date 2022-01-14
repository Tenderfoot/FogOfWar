#pragma once

#include <SDL_net/SDL_net.h>

class ServerHandler
{
public:

	static void init();
	static void run();
	
	static bool initialized;
protected:
};