
#include "common.h"
#include "client_handler.h"
#include "grid_manager.h"
#include "gatherer.h"

#define TIMEOUT (5000) /*five seconds */
#define ERROR (0xff)
#define TICK_RATE 500

Uint16 ClientHandler::port;
const char* ClientHandler::host, * ClientHandler::fname, * ClientHandler::fbasename;
Sint32 ClientHandler::flen, ClientHandler::pos, ClientHandler::p2;
int ClientHandler::len, ClientHandler::blocks, ClientHandler::i, ClientHandler::err;
Uint32 ClientHandler::ack;
IPaddress ClientHandler::ip;
UDPsocket ClientHandler::sock;
UDPpacket* ClientHandler::in, * ClientHandler::out;
FILE* ClientHandler::f;
bool ClientHandler::initialized = false;
Uint32 ClientHandler::ipnum;
SDLNet_SocketSet ClientHandler::set;
extern int udpsend(UDPsocket sock, int channel, UDPpacket* out, UDPpacket* in, Uint32 delay, Uint8 expect, int timeout);

typedef struct
{
	int id;
	int type;
	int x;
	int y;
}t_entitymessage;

void ClientHandler::init()
{
	printf("Initializing Client...\n");

	/* get the host from the commandline */
	host = "127.0.0.1";
	/* get the port from the commandline */
	port = (Uint16)strtol("1227", NULL, 0);
	if (!port)
	{
		printf("a server port cannot be 0.\n");
		exit(3);
	}

	if (SDLNet_ResolveHost(&ip, host, port) == -1)
	{
		printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		exit(4);
	}

	/* open udp client socket */
	if (!(sock = SDLNet_UDP_Open(0)))
	{
		printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		exit(5);
	}

	/* allocate max packet */
	if (!(out = SDLNet_AllocPacket(65535)))
	{
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		exit(6);
	}
	if (!(in = SDLNet_AllocPacket(65535)))
	{
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		exit(6);
	}

	/* bind server address to channel 0 */
	if (SDLNet_UDP_Bind(sock, 0, &ip) == -1)
	{
		printf("SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
		exit(7);
	}

	set = SDLNet_AllocSocketSet(16);
	if (!set) {
		printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
		exit(1); //most of the time this is a major error, but do what you want.
	}

	// add socket to a socket set
	SDLNet_UDP_AddSocket(set, sock);

	initialized = true;
    std::thread *new_thread = new std::thread(run);
}

int ClientHandler::recieve_gatherer_data(FOWGatherer *specific_character, UDPpacket* packet, int i)
{
	printf("We've got a gatherer!\n");
	int character_state = packet->data[i];
	int has_gold = packet->data[i+1];
	printf("state was %d and has_gold was %d\n", character_state, has_gold);
	return i + 2;
}


void ClientHandler::run()
{
    printf("running client\n");
	/* open output file */
	float last_tick = 0;
	char fname[65535];
	int numready;

	// loop the server - check for packets incoming, send outgoing
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
				in = SDLNet_AllocPacket(65535);
				int numpkts = SDLNet_UDP_Recv(sock, in);
				
				if (numpkts) 
				{
					if (in->data[0] == MESSAGE_TILES)
					{
						printf("Received tile update data\n");
						for (int i = 3; i < in->len; i++)
						{
							int tile_index = i - 3;
							int x_pos = ((tile_index / ((int)GridManager::size.x)));
							int y_pos = (tile_index % ((int)GridManager::size.x));
							GridManager::tile_map[y_pos][x_pos].type = (tiletype_t)in->data[i];
						}
						// this line below murders memory if hit a lot
						// need to move genbuffers
						GridManager::calc_all_tiles();
					}

					if (in->data[0] == MESSAGE_ENTITY_DATA)
					{
						printf("Received entity update data\n");
						int num_entities = in->data[1];
						for (int i = 2; i < num_entities * 4; i = i + 4)
						{
							t_entitymessage new_message;
							new_message.id = in->data[i];
							new_message.type = in->data[i + 1];
							new_message.x = in->data[i + 2];
							new_message.y = in->data[i + 3];

							if ((entity_types)new_message.type == FOW_GATHERER)
							{
								auto  net_entities = GridManager::get_entities_of_type(FOW_GATHERER);
								for (auto entity : net_entities)
								{
									if (entity->id == new_message.id)
									{
										entity->position = t_vertex(new_message.x, new_message.y, 0.0f);
									}
								}

							}
						}
					}
					if (in->data[0] == MESSAGE_ENTITY_DETAILED)
					{
						printf("Received entity update data\n");
						int num_entities = in->data[1];
						for (int i = 2; i < in->len; i = i)
						{
							t_entitymessage new_message;
							new_message.id = in->data[i];
							new_message.type = in->data[i + 1];
							new_message.x = in->data[i + 2];
							new_message.y = in->data[i + 3];

							if ((entity_types)new_message.type == FOW_GATHERER)
							{
								auto  net_entities = GridManager::get_entities_of_type(FOW_GATHERER);
								// if a new gatherer was created on server this will break
								for (auto entity : net_entities)
								{
									if (entity->id == new_message.id)
									{
										i = recieve_gatherer_data((FOWGatherer*)entity, in, i + 4);
									}
								}
							}
							else
							{
								i = i + 4;
							}
						}
					}
					else if(in->data[0] == MESSAGE_HELLO)
					{
						strcpy(fname, (char*)in->data + 1);
						printf("fname=%s\n", fname);
					}
					else
					{
						printf("Network request type not recognized\n");
						printf("message type: %d\n", in->data[0]);
					}
				}
				SDLNet_FreePacket(in);
			}
		}

		if (SDL_GetTicks() - last_tick > TICK_RATE)
		{
			out = SDLNet_AllocPacket(65535);
			out->data[0] = MESSAGE_ENTITY_DETAILED;
			strcpy((char*)out->data + 1, "Client to Server");
			out->len = strlen("Client to Server") + 2;
			udpsend(sock, 0, out, in, 0, 1, TIMEOUT);
			last_tick = SDL_GetTicks();
			SDLNet_FreePacket(out);
		}
	}

}