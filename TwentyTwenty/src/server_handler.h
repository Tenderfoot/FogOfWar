#pragma once

#include <SDL_net/SDL_net.h>

class ServerHandler
{
public:

	static void init();
	static void run();
	static void handle_new_connection();

	static bool initialized;
protected:
};