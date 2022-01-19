
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <thread>
#include "common.h"
#include "server_handler.h"
#include "grid_manager.h"
#include "game.h"
#include "gatherer.h"

#define ERROR (0xff)
#define TIMEOUT (5000) /*five seconds */
#define TICK_RATE 500

const char* host = NULL;
char fname[65535];
int len = 1400, plen, err = 0;
Sint32 flen;
Uint16 port;
Uint32 ipnum;
IPaddress ip;
UDPsocket sock;
UDPpacket* out, * in, ** packets, * outs[32];
Sint32 p, p2, i;
bool ServerHandler::initialized = false;
SDLNet_SocketSet set;

int udpsend(UDPsocket sock, int channel, UDPpacket* out, UDPpacket* in, Uint32 delay, Uint8 expect, int timeout)
{
	if (!SDLNet_UDP_Send(sock, channel, out))
	{
		printf("SDLNet_UDP_Send: %s\n", SDLNet_GetError());
		exit(1);
	}
	return(in->data[0] == ERROR ? -1 : 1);
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

	initialized = true;

	printf("running server...\n");
	std::thread* test = new std::thread(run);
}

UDPpacket *ServerHandler::send_tilemap()
{
	UDPpacket* packet = SDLNet_AllocPacket(sizeof(int) + sizeof(int) + sizeof(int) + (sizeof(int) * GridManager::size.x * GridManager::size.y));

	// assemble the data to send the tile map information over
	// this sends the tile type - from this we can infer whether or not the tile is blocking (wall)
	// the data packet will have a byte specifying that this is a grid map update,
	// followed by two integers - the width and height, followed by 
	// width*height integer tiletypes, organized by row leading
	// should be easy, right?

	// message type
	packet->len = (GridManager::size.x * GridManager::size.y) + 3;
	packet->data[0] = MESSAGE_TILES;
	packet->data[1] = GridManager::size.x;
	packet->data[2] = GridManager::size.y;
	for (int widthItr = 0; widthItr < GridManager::size.x; widthItr++)
		for (int heightItr = 0; heightItr < GridManager::size.y; heightItr++)
			packet->data[(int)(heightItr * GridManager::size.x) + widthItr + 3] = GridManager::tile_map[widthItr][heightItr].type;

	return packet;
}

UDPpacket* ServerHandler::send_entity_data()
{
	UDPpacket* packet = SDLNet_AllocPacket(2 + Game::entities.size()*4);
	// assemble the data to send the entity information over
	// the data packet will have a byte specifying that this is an enitity update,
	// followed by the number of entities in the update
	// each entity includes its id, type, and position (4 integers)
	// if the entity already exists, update it,
	// otherwise bake it fresh?

	packet->len = (Game::entities.size()*4) + 2;	// is this the size like above?
	packet->data[0] = MESSAGE_ENTITY_DATA;
	packet->data[1] = Game::entities.size();
	int i = 2;
	for (auto entity : Game::entities)
	{
		packet->data[i] = entity->id;
		packet->data[i+1] = entity->type;
		packet->data[i+2] = entity->position.x;
		packet->data[i+3] = entity->position.y;
		i += 4;
	}

	return packet;
}

int ServerHandler::assemble_gatherer_data(FOWGatherer *specific_character, UDPpacket* packet, int i)
{
	packet->data[i] = specific_character->state;
	packet->data[i+1] = specific_character->has_gold;

	return i + 2;
}

UDPpacket* ServerHandler::send_entity_data_detailed()
{
	UDPpacket* packet = SDLNet_AllocPacket(65535);
	
	// I didn't want to mess up the above function
	// this one is going to send state and current_command if applicable
	// if moving it will send the current_path

	packet->data[0] = MESSAGE_ENTITY_DETAILED;
	packet->data[1] = Game::entities.size();
	int i = 2;
	for (auto entity : Game::entities)
	{
		packet->data[i] = entity->id;
		packet->data[i + 1] = entity->type;
		packet->data[i + 2] = entity->position.x;
		packet->data[i + 3] = entity->position.y;
		if ((entity_types)entity->type == FOW_GATHERER)
		{
			i = assemble_gatherer_data((FOWGatherer*)entity, packet, i + 4);
		}
		else
		{
			i += 4;
		}
	}

	packet->len = i;

	return packet;
}

void ServerHandler::run()
{
	int numready;

	in->data[0] = 0;
	printf("waiting for connection...\n");

	while (!SDLNet_UDP_Recv(sock, in))
		SDL_Delay(100); /*1/10th of a second */

	// connection didn't give us what we expected
	if (in->data[0] != 1 << 4)
	{
		in->data[0] = ERROR;
		in->len = 1;
		SDLNet_UDP_Send(sock, -1, in);
	}

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

	int numused;

	// add socket to a socket set
	numused = SDLNet_UDP_AddSocket(set, sock);
	if (numused == -1) {
		printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
		// perhaps you need to restart the set and make it bigger...
	}

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

					if (in->data[0] == MESSAGE_TILES)	// client is requesting the tiles
					{
						out = send_tilemap();
						out->address = in->address;
						udpsend(sock, -1, out, in, 0, 1, TIMEOUT);
					}
					if (in->data[0] == MESSAGE_ENTITY_DATA)	// client is requesting basic entity data (id, type, position)
					{
						out = send_entity_data();
						out->address = in->address;
						udpsend(sock, -1, out, in, 0, 1, TIMEOUT);
					}
					if (in->data[0] == MESSAGE_ENTITY_DETAILED)	// client is requesting detailed entity data (entity type specifics included)
					{
						out = send_entity_data_detailed();
						out->address = in->address;
						udpsend(sock, -1, out, in, 0, 1, TIMEOUT);
					}
					else if(in->data[0] == MESSAGE_HELLO)
					{
						strcpy(fname, (char*)in->data + 1);
						printf("fname=%s\n", fname);
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
			out = SDLNet_AllocPacket(65535);
			out->data[0] = MESSAGE_HELLO;
			strcpy((char*)out->data + 1, "Server to Client");
			out->len = strlen("Server to Client") + 2;
			out->address = in->address;
			udpsend(sock, -1, out, in, 0, 1, TIMEOUT);
			last_tick = SDL_GetTicks();
			SDLNet_FreePacket(out);
		}
	}
}