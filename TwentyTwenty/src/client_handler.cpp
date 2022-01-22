#include "common.h"
#include "client_handler.h"
#include "grid_manager.h"
#include "gatherer.h"
#include "game.h"

#define TIMEOUT (5000) /*five seconds */
#define ERROR (0xff)
#define TICK_RATE 30

// from SDL_Net
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

// for commands
std::vector<FOWCommand> ClientHandler::command_queue;

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
	FOWPlayer::team_id = 1;
    std::thread *new_thread = new std::thread(run);
}

UDPpacket* ClientHandler::send_command_queue()
{
	UDPpacket* packet = SDLNet_AllocPacket(65535);
	
	// So Here we're going to send commands from the server to the client
	// a command has a type
	packet->data[0] = MESSAGE_CLIENT_COMMAND;
	packet->data[1] = command_queue.size();
	int i = 2;
	for (auto command : command_queue)
	{	
		packet->data[i] = command.self_ref->id;
		packet->data[i + 1] = command.type;
		i += 2;
		switch (command.type)
		{
			case MOVE:
				packet->data[i] = command.position.x;
				packet->data[i + 1] = command.position.y;
				i += 2;
				break;
			case GATHER:
				packet->data[i] = command.target->id;
				i += 1;
				break;
			case BUILD_BUILDING:
				// building type and position
				packet->data[i] = ((FOWGatherer*)command.self_ref)->building_type;
				packet->data[i + 1] = command.position.x;
				packet->data[i + 2] = command.position.y;
				i += 3;
				break;
			case BUILD_UNIT:
				// unit type
				packet->data[i] = ((FOWBuilding*)command.self_ref)->entity_to_build;
				i += 1;
				break;
			case ATTACK_MOVE:
				packet->data[i] = command.position.x;
				packet->data[i + 1] = command.position.y;
				i += 2;
				break;
		}
	}

	command_queue.clear();
	packet->len = i;	// is this the size like above?

	return packet;
}

int ClientHandler::recieve_gatherer_data(FOWGatherer* specific_character, UDPpacket* packet, int i)
{
	// we're going to hack in getting gold until discrete players are in
	auto holding_gold = specific_character->has_gold;
	int has_gold = packet->data[i];
	specific_character->has_gold = has_gold;
	if (holding_gold == 0 && has_gold == 1)
	{
		specific_character->add_to_skin("moneybag");
	}
	if (holding_gold == 1 && has_gold == 0)
	{
		FOWPlayer::gold++;
		specific_character->reset_skin();
	}
	return i + 1;
}

int ClientHandler::recieve_character_data(FOWCharacter *specific_character, UDPpacket* packet, int i)
{
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
			specific_character->draw_position = specific_character->position;
			specific_character->set_animation("walk_two");
		}

		// and get their current path
		int num_stops = packet->data[i];
		i++;
		// so we're going to assume for now, if the path size matches,
		// nothing has changed. this isn't a perfect assumption but its pretty close
		// edit:
		// we made a change - this used to be num_stops == specific_character->current_path.size()
		// the idea is that just because the server hit the next spot and we haven't yet doens't mean
		// the entire path needs to be rewritten
		//int check = std::abs((int)(num_stops - specific_character->current_path.size()));
		
		if (num_stops == specific_character->current_path.size())
		{
			i += num_stops*2;
		}
		else // otherwise lets repopulate current_path
		{
			specific_character->draw_position = specific_character->position;
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
	else
	{
		// Every state other than moving we just want to draw them where they actually are
		specific_character->draw_position = specific_character->position;
	}

	if ((GridCharacterState)character_state == GRID_IDLE)
	{
		// if the character just started moving, boot up the walk animation
		if (previous_state != GRID_IDLE)
		{
			specific_character->set_animation("idle_two");
		}
	}
	if ((GridCharacterState)character_state == GRID_DYING)
	{
		// if the character just started moving, boot up the walk animation
		if (previous_state != GRID_DYING)
		{
			if (specific_character->spine_initialized)
			{
				specific_character->die();
			}
		}
	}
	if ((GridCharacterState)character_state == GRID_ATTACKING)
	{
		// Get the attack target from the packet data
		int attack_target = packet->data[i];
		i++;
		for (auto entity : Game::entities)
		{
			if (entity->id == attack_target)
			{
				specific_character->network_target = (FOWSelectable*)entity;
			}
		}
		// if the character just started moving, boot up the walk animation
		if (previous_state != GRID_ATTACKING)
		{
			specific_character->attack();
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
						int num_entities = in->data[1];
						for (int i = 2; i < in->len; i = i)
						{
							t_entitymessage new_message;
							new_message.id = in->data[i];
							new_message.type = in->data[i + 1];
							new_message.x = in->data[i + 2];
							new_message.y = in->data[i + 3];
							bool visible = in->data[i + 4];
							int team_id = in->data[i + 5];
							i = i + 6;

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
								if (((entity_types)new_message.type == FOW_GATHERER) ||
									((entity_types)new_message.type == FOW_KNIGHT) ||
									((entity_types)new_message.type == FOW_SKELETON))
								{
									((FOWCharacter*)the_entity)->draw_position.x = new_message.x;
									((FOWCharacter*)the_entity)->draw_position.y = new_message.y;
								}
								((FOWSelectable*)the_entity)->team_id = team_id;
							}

							// if its a unit, handle unit
							if (((entity_types)new_message.type == FOW_GATHERER) ||
								((entity_types)new_message.type == FOW_KNIGHT) ||
								((entity_types)new_message.type == FOW_SKELETON))
							{
								the_entity->position.x = new_message.x;
								the_entity->position.y = new_message.y;
								the_entity->visible = visible;
								// recieve character data
								i = recieve_character_data((FOWCharacter*)the_entity, in, i);
							}
							else if (((entity_types)new_message.type == FOW_TOWNHALL) ||
								((entity_types)new_message.type == FOW_BARRACKS) ||
								((entity_types)new_message.type == FOW_FARM))
							{
								((FOWSelectable*)the_entity)->team_id = team_id;
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

		// Check to see if there are any commands to send to the server
		if (SDL_GetTicks() - last_tick > TICK_RATE)
		{
			// if the client has any commands to send to the server, do so now
			if (command_queue.size() > 0)
			{
				out = send_command_queue();
				udpsend(sock, 0, out, in, 0, 1, TIMEOUT);
				SDLNet_FreePacket(out);
				last_tick = SDL_GetTicks();
			}
			else    // otherwise just request everything for now I guess
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
}