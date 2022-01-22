#pragma once

#include <SDL_net/SDL_net.h>

class FOWCharacter;
class FOWGatherer;
struct data_getter;
struct data_setter;

class ServerHandler
{
public:

	static void init();
	static void run();
	static void handle_new_connection();
	static UDPpacket* send_tilemap();
	static UDPpacket* send_entity_data();
	static UDPpacket* send_entity_data_detailed();
	static void assemble_character_data(FOWCharacter* specific_character);
	static void assemble_gatherer_data(FOWGatherer *specific_character);
	static void handle_bindme();
	static void handle_client_command();


	static data_getter packet_data;
	static data_setter out_data;
	static bool initialized;
protected:
};