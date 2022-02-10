
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <thread>
#include "common.h"
#include "server_handler.h"
#include "grid_manager.h"
#include "game.h"
#include "gatherer.h"
#include "client_handler.h"

#define ERROR (0xff)
#define TIMEOUT (5000) /*five seconds */
#define TICK_RATE 30

const char* host = NULL;
char fname[65535];
int len = 1400, plen, err = 0;
Sint32 flen;
Uint16 port;
Uint32 ipnum;
IPaddress ip;
UDPsocket sock;
UDPpacket* out, * in, ** packets, * outs[32];
Sint32 p, p2;
bool ServerHandler::initialized = false;
data_getter ServerHandler::packet_data;
data_setter ServerHandler::out_data;
t_tracked_player ServerHandler::client;
SDLNet_SocketSet set;

int udpsend(UDPsocket sock, int channel, UDPpacket* out)
{
	if (!SDLNet_UDP_Send(sock, channel, out))
	{
		printf("SDLNet_UDP_Send: %s\n", SDLNet_GetError());
		exit(1);
	}
	return 1;
}

void ServerHandler::init()
{
	port = (Uint16)strtol("1227", NULL, 0);
	if (!port)
	{
		printf("a server port cannot be 0.\n");
		exit(3);
	}

	len = strtol("127.0.0.1", NULL, 0);

	/* open udp server socket */
	if (!(sock = SDLNet_UDP_Open(port)))
	{
		printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		exit(4);
	}
	printf("port %hd opened\n", port);

	/* allocate max packet */
	if (!(out = SDLNet_AllocPacket(65535)))
	{
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		exit(5);
	}
	if (!(in = SDLNet_AllocPacket(65535)))
	{
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		exit(5);
	}

	/* allocate 32 packets of size len */
	if (!(packets = SDLNet_AllocPacketV(32, len)))
	{
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		exit(6);
	}

	set = SDLNet_AllocSocketSet(16);
	if (!set) {
		printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
		exit(1); //most of the time this is a major error, but do what you want.
	}

	int numused;
	// add socket to a socket set
	numused = SDLNet_UDP_AddSocket(set, sock);
	if (numused == -1) {
		printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
	}

	initialized = true;

	printf("running server...\n");
	std::thread* test = new std::thread(run);
}

UDPpacket *ServerHandler::send_tilemap()
{
	UDPpacket* packet = SDLNet_AllocPacket(sizeof(int) + sizeof(int) + sizeof(int) + (sizeof(int) * GridManager::size.x * GridManager::size.y));

	// set up our setter
	out_data.clear();
	out_data.packet = packet;
	
	// assemble the data to send the tile map information over
	// this sends the tile type - from this we can infer whether or not the tile is blocking (wall)
	// the data packet will have a byte specifying that this is a grid map update,
	// followed by two integers - the width and height, followed by 
	// width*height integer tiletypes, organized by row leading
	// should be easy, right?

	// message type
	out_data.push_back(MESSAGE_TILES);
	out_data.push_back(GridManager::size.x);
	out_data.push_back(GridManager::size.y);
	for (int widthItr = 0; widthItr < GridManager::size.x; widthItr++)
		for (int heightItr = 0; heightItr < GridManager::size.y; heightItr++)
			out_data.push_back(GridManager::tile_map[widthItr][heightItr].type);	// not even 100% sure this works, real line below
			
	//packet->data[(int)(heightItr * GridManager::size.x) + widthItr + 3] = GridManager::tile_map[widthItr][heightItr].type;

	packet->len = out_data.i;

	return packet;
}

UDPpacket* ServerHandler::send_entity_data()
{
	UDPpacket* packet = SDLNet_AllocPacket(2 + Game::entities.size()*4);

	// set up our setter
	out_data.clear();
	out_data.packet = packet;

	// assemble the data to send the entity information over
	// the data packet will have a byte specifying that this is an enitity update,
	// followed by the number of entities in the update
	// each entity includes its id, type, and position (4 integers)
	// if the entity already exists, update it,
	// otherwise bake it fresh?

	out_data.push_back(MESSAGE_ENTITY_DATA);
	out_data.push_back(Game::entities.size());
	for (auto entity : Game::entities)
	{
		out_data.push_back(entity->id);
		out_data.push_back(entity->type);
		out_data.push_back(entity->position.x);
		out_data.push_back(entity->position.y);
	}

	packet->len = out_data.i;

	return packet;
}

void ServerHandler::assemble_character_data(FOWCharacter* specific_character)
{
	out_data.push_back(specific_character->flip);
	out_data.push_back(specific_character->state);

	// we need to add the path to the message if they are in moving state
	if (specific_character->state == GRID_MOVING)
	{
		// since the client read moving state, they know to look for:
		// # of stops in the path
			// x of path entry
			// y of path entry
		//
		// this will allow me to reconstruct current_path client side
		// with both (current_path.size() > 0) and state = GRID_MOVING, everything should be there
		// to do client-side movement prediction
		out_data.push_back(specific_character->current_path.size());

		for (int j = 0; j < specific_character->current_path.size(); j++)
		{
			auto tile = specific_character->current_path.at(j);
			out_data.push_back(tile->x);
			out_data.push_back(tile->y);
		}
	}
	if (specific_character->state == GRID_ATTACKING)
	{
		// add the attack target to the data packet
		out_data.push_back(specific_character->get_attack_target()->id);
	}
	if (specific_character->state == GRID_CHOPPING)
	{
		// add the attack target to the data packet
		out_data.push_back(((FOWGatherer*)specific_character)->current_tree.x);
		out_data.push_back(((FOWGatherer*)specific_character)->current_tree.y);
	}

	if (specific_character->type == FOW_GATHERER)
	{
		assemble_gatherer_data((FOWGatherer*)specific_character);
	}
}

void ServerHandler::assemble_gatherer_data(FOWGatherer *specific_character)
{
	out_data.push_back(specific_character->has_gold);
}

void ServerHandler::assemble_building_data(FOWBuilding* specific_building)
{
	out_data.push_back(specific_building->currently_making_unit);
	out_data.push_back(specific_building->unit_start_time);
	out_data.push_back(specific_building->destroyed);
}

UDPpacket* ServerHandler::send_entity_data_detailed()
{
	UDPpacket* packet = SDLNet_AllocPacket(65535);

	// set up our setter
	out_data.clear();
	out_data.packet = packet;

	// send some entity data
	out_data.push_back(MESSAGE_ENTITY_DETAILED);
	out_data.push_back(client.gold);
	out_data.push_back(client.wood);
	out_data.push_back(Game::entities.size());

	for (auto entity : Game::entities)
	{
		// don't send decoration information
		if (entity->type == FOW_DECORATION)
			continue;

		out_data.push_back(entity->id);
		out_data.push_back(entity->type);
		out_data.push_back(entity->position.x);
		out_data.push_back(entity->position.y);
		out_data.push_back(entity->visible);
		out_data.push_back(((FOWSelectable*)entity)->team_id);	// I wish I didn't have to cast here

		if (((FOWSelectable*)entity)->is_unit())
		{
			assemble_character_data((FOWCharacter*)entity);
		}
		else
		{
			assemble_building_data((FOWBuilding*)entity);
		}
	}

	packet->len = out_data.i;

	return packet;
}

void ServerHandler::handle_bindme()
{
	printf("Recieved bind request\n");
	// handle initial connection
	memcpy(&ip, &in->address, sizeof(IPaddress));
	host = SDLNet_ResolveIP(&ip);
	ipnum = SDL_SwapBE32(ip.host);
	port = SDL_SwapBE16(ip.port);

	if (host)
		printf("request from host=%s port=%hd\n", host, port);
	else
		printf("request from host=%d.%d.%d.%d port=%hd\n",
			ipnum >> 24,
			(ipnum >> 16) & 0xff,
			(ipnum >> 8) & 0xff,
			ipnum & 0xff,
			port);

	strcpy(fname, (char*)in->data + 1);
	printf("fname=%s\n", fname);

	if (SDLNet_UDP_Bind(sock, 0, &ip) == -1)
	{
		printf("SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
		exit(7);
	}

	client.ip = in->address;
	client.gold = 2000;
	client.wood = 1000;
	client.team_id = 1;

	out = SDLNet_AllocPacket(65535);
	SDLNet_Write32(MESSAGE_BINDME, &out->data[0]);
	strcpy((char*)out->data + 4, "you have been bound!");
	out->len = strlen("you have been bound!") + 4;
	out->address = client.ip;
	udpsend(sock, -1, out);
	SDLNet_FreePacket(out);
}

void ServerHandler::handle_client_command()
{
	// if the command has a target
	GameEntity* target = nullptr;

	int num_commands = packet_data.get_data();
	printf("%d Client Command Recieved!!\n", num_commands);
	for (int j = 0; j < num_commands; j++)
	{
		int entity_id = packet_data.get_data();
		int command_type = packet_data.get_data();

		GameEntity* command_entity = nullptr;
		for (auto entity : Game::entities)
		{
			if (entity->id == entity_id)
			{
				command_entity = entity;
			}
		}

		if ((t_ability_enum)command_type == MOVE)
		{
			int x_pos = packet_data.get_data();
			int y_pos = packet_data.get_data();
			printf("send %d to %d, %d\n", entity_id, x_pos, y_pos);
			((FOWCharacter*)command_entity)->give_command(FOWCommand((t_ability_enum)command_type, t_vertex(x_pos, y_pos, 0.0f)));
		}
		if ((t_ability_enum)command_type == GATHER)
		{
			int target_id = packet_data.get_data();
			printf("gather %d to %d\n", entity_id, target_id);
			((FOWCharacter*)command_entity)->give_command(FOWCommand((t_ability_enum)command_type, (FOWSelectable*)get_target(target_id)));
		}
		if ((t_ability_enum)command_type == BUILD_BUILDING)
		{
			int building_type = packet_data.get_data();
			int x_pos = packet_data.get_data();
			int y_pos = packet_data.get_data();
			printf("build a building at %d %d\n", x_pos, y_pos);
			((FOWGatherer*)command_entity)->building_type = (entity_types)building_type;	// maybe try to find a way to bake this into the command instead
			((FOWCharacter*)command_entity)->give_command(FOWCommand((t_ability_enum)command_type, t_vertex(x_pos, y_pos, 0.0f)));
		}
		if ((t_ability_enum)command_type == BUILD_UNIT)
		{
			int unit_type = packet_data.get_data();
			printf("build a unit!!!!\n");
			((FOWBuilding*)command_entity)->process_command(FOWCommand(BUILD_UNIT, (entity_types)unit_type));
		}
		if ((t_ability_enum)command_type == ATTACK_MOVE)
		{
			int x_pos = packet_data.get_data();
			int y_pos = packet_data.get_data();
			printf("attack move %d to %d, %d\n", entity_id, x_pos, y_pos);
			((FOWCharacter*)command_entity)->give_command(FOWCommand((t_ability_enum)command_type, t_vertex(x_pos, y_pos, 0.0f)));
		}
		if ((t_ability_enum)command_type == ATTACK)
		{
			int target_id = packet_data.get_data();
			((FOWCharacter*)command_entity)->give_command(FOWCommand((t_ability_enum)command_type, (FOWSelectable*)get_target(target_id)));
		}
		if ((t_ability_enum)command_type == CHOP)
		{
			int x_pos = packet_data.get_data();
			int y_pos = packet_data.get_data();
			printf("chop command\n", entity_id, x_pos, y_pos);
			((FOWCharacter*)command_entity)->give_command(FOWCommand((t_ability_enum)command_type, t_vertex(x_pos, y_pos, 0.0f)));
		}
	}
}

GameEntity* ServerHandler::get_target(int entity_id)
{
	GameEntity* target = nullptr;

	for (auto entity : Game::entities)
	{
		if (entity->id == entity_id)
		{
			target = entity;
		}
	}
	return target;
}

void ServerHandler::run()
{
	int numready;

	in->data[0] = 0;
	printf("waiting for connection...\n");

	// loop the server - check for packets incoming, send outgoing
	float last_tick = 0;
	while (1)
	{
		numready = SDLNet_CheckSockets(set, 0);
		if (numready == -1) {
			printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
			//most of the time this is a system error, where perror might help you.
			perror("SDLNet_CheckSockets");
		}
		else if (numready) {
			//printf("There are %d sockets with activity!\n", numready);
			// check all sockets with SDLNet_SocketReady and handle the active ones.
			if (SDLNet_SocketReady(sock)) {
				int numpkts = SDLNet_UDP_Recv(sock, in);
				if (numpkts) {

					packet_data.clear();
					packet_data.packet = in;

					t_messagetype recieved_message = (t_messagetype)packet_data.get_data();

					if (recieved_message == MESSAGE_TILES)	// client is requesting the tiles
					{
						out = send_tilemap();
						out->address = in->address;
						udpsend(sock, -1, out);
					}
					else if (recieved_message == MESSAGE_ENTITY_DATA)	// client is requesting basic entity data (id, type, position)
					{
						out = send_entity_data();
						out->address = in->address;
						udpsend(sock, -1, out);
					}
					else if (recieved_message == MESSAGE_ENTITY_DETAILED)	// client is requesting detailed entity data (entity type specifics included)
					{
						out = send_entity_data_detailed();
						out->address = in->address;
						udpsend(sock, -1, out);
					}
					else if(recieved_message == MESSAGE_HELLO)	// client is saying hello!
					{
						strcpy(fname, (char*)in->data + 1);
						printf("fname=%s\n", fname);
					}
					else if (recieved_message == MESSAGE_BINDME)	// client is saying hello!
					{
						handle_bindme();
					}
					else if (recieved_message == MESSAGE_CLIENT_COMMAND)		// client sent a command intended for a FOWSelectable
					{
						handle_client_command();
					}
					else if (recieved_message == MESSAGE_MAP_INFO)	// client is saying hello!
					{
						strcpy(fname, (char*)in->data + 4);
						printf("fname=%s\n", fname);
						out = SDLNet_AllocPacket(65535);
						SDLNet_Write32(MESSAGE_MAP_INFO, &out->data[0]);
						strcpy((char*)out->data + 4, Game::mapname.c_str());
						out->len = strlen(Game::mapname.c_str()) + 4;
						out->address = in->address;
						udpsend(sock, -1, out);
						SDLNet_FreePacket(out);
					}
					else
					{
						printf("Network request type not recognized\n");
						printf("Message type was: %d\n", in->data[0]);
					}
				}
			}
		}
		
		if (SDL_GetTicks() - last_tick > TICK_RATE)
		{
			/* Lets stop doing this for now
			*  so as of now, I guess
			*  the server is only responding to requests
			*  and the client is requesting all entity data
			* every tick.
			out = SDLNet_AllocPacket(65535);
			out->data[0] = MESSAGE_HELLO;
			strcpy((char*)out->data + 1, "Server to Client");
			out->len = strlen("Server to Client") + 2;
			out->address = in->address;
			udpsend(sock, -1, out, in, 0, 1, TIMEOUT);
			last_tick = SDL_GetTicks();
			SDLNet_FreePacket(out);
			*/
		}
	}
}