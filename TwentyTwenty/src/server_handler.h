#pragma once

#include <SDL_net/SDL_net.h>

class FOWGatherer;

class ServerHandler
{
public:

	static void init();
	static void run();
	static void handle_new_connection();
	static UDPpacket* send_tilemap();
	static UDPpacket* send_entity_data();
	static UDPpacket* send_entity_data_detailed();
	static int assemble_character_data(FOWGatherer* specific_character, UDPpacket* packet, int i);
	static int assemble_gatherer_data(FOWGatherer *specific_character, UDPpacket* packet, int i);

	static bool initialized;
protected:
};