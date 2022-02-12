#pragma once

#include <SDL_net/SDL_net.h>

class FOWCharacter;
class FOWGatherer;
struct data_getter;
struct data_setter;
class GameEntity;
class FOWBuilding;

typedef struct 
{
	int gold;
	int wood;
	IPaddress ip;
	int team_id;
}t_tracked_player;

class ServerHandler
{
public:

	// methods
	static void init();
	static void run();

	// network stuff
	static void handle_new_connection();
	static UDPpacket* send_tilemap();
	static UDPpacket* send_entity_data();
	static UDPpacket* send_entity_data_detailed();
	static void handle_bindme();
	static void handle_client_command();

	// recieve character stuff
	static void assemble_character_data(FOWCharacter* specific_character);
	static void assemble_gatherer_data(FOWGatherer *specific_character);
	// recieve building stuff
	static void assemble_building_data(FOWBuilding* specific_character);

	// helper
	static GameEntity* get_target(int entity_id);

	// variables
	static data_getter packet_data;
	static data_setter out_data;
	static bool initialized;
	static t_tracked_player client;
	static bool tiles_dirty; // did the grid change? so that we need to notify clients?
protected:
};