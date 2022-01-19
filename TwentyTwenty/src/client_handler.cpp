
#include "common.h"
#include "client_handler.h"
#include "grid_manager.h"
#include "gatherer.h"
#include "game.h"

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
	int has_gold = packet->data[i];
	printf("has gold was %d\n", has_gold);
	return i + 1;
}

int ClientHandler::recieve_character_data(FOWCharacter *specific_character, UDPpacket* packet, int i)
{
	printf("We've got a character!\n");

	// Get and set the characters state
	int character_flip = packet->data[i];
	i++;
	int character_state = packet->data[i];
	i++;

	// set stuff
	auto previous_state = specific_character->state;
	specific_character->state = (GridCharacterState)character_state;
	specific_character->flip = character_flip;

	// there is extra data if the state is moving
	if ((GridCharacterState)character_state == GRID_MOVING)
	{
		// if the character just started moving, boot up the walk animation
		if (previous_state != GRID_MOVING)
		{
			specific_character->animationState->setAnimation(0, "walk_two", true);
		}

		// and get their current path
		int num_stops = packet->data[i];
		i++;
		// so we're going to assume for now, if the path size matches,
		// nothing has changed. this isn't a perfect assumption but its pretty close
		if (num_stops == specific_character->current_path.size())
		{
			i += num_stops*2;
		}
		else // otherwise lets repopulate current_path
		{
			specific_character->current_path.clear();
			for (int j = 0; j < num_stops; j++)
			{
				int x = packet->data[i];
				int y = packet->data[i + 1];
				t_tile* tile_ref = &GridManager::tile_map[x][y];
				specific_character->current_path.push_back(tile_ref);
				i += 2;
			}
		}
	}

	if (specific_character->type == FOW_GATHERER)
	{
		i = recieve_gatherer_data((FOWGatherer*)specific_character, packet, i);
	}
	return i;
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
						printf("len was %d\n", in->len);
						for (int i = 2; i < in->len; i = i)
						{
							t_entitymessage new_message;
							new_message.id = in->data[i];
							new_message.type = in->data[i + 1];
							new_message.x = in->data[i + 2];
							new_message.y = in->data[i + 3];
							i = i + 4;

							// does this entity exist client side already?
							GameEntity* the_entity = nullptr;

							for (auto entity : Game::entities)
							{
								if (entity->id == new_message.id)
								{
									the_entity = entity;
								}
							}

							// if not, create it
							if (the_entity == nullptr)
							{
								the_entity = GridManager::build_and_add_entity((entity_types)new_message.type, t_vertex(new_message.x, new_message.y, 0.0f));
							}

							// if its a unit, handle unit
							if (((entity_types)new_message.type == FOW_GATHERER) ||
								((entity_types)new_message.type == FOW_KNIGHT) ||
								((entity_types)new_message.type == FOW_SKELETON))
							{
								the_entity->position.x = new_message.x;
								the_entity->position.y = new_message.y;
								// recieve character data
								i = recieve_character_data((FOWCharacter*)the_entity, in, i);
							}
							else if (((entity_types)new_message.type == FOW_TOWNHALL) ||
								((entity_types)new_message.type == FOW_BARRACKS) ||
								((entity_types)new_message.type == FOW_FARM))
							{
									// do building stuff
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