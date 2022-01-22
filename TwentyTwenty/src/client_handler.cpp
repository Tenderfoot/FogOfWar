#include "common.h"
#include "client_handler.h"
#include "grid_manager.h"
#include "gatherer.h"
#include "game.h"
#include "Settings.h"

#define TIMEOUT (5000) /*five seconds */
#define ERROR (0xff)
#define TICK_RATE 30

// from SDL_Net
Uint16 ClientHandler::port;
const char* ClientHandler::host, * ClientHandler::fname, * ClientHandler::fbasename;
Sint32 ClientHandler::flen, ClientHandler::pos, ClientHandler::p2;
int ClientHandler::len, ClientHandler::blocks, ClientHandler::err;
Uint32 ClientHandler::ack;
IPaddress ClientHandler::ip;
UDPsocket ClientHandler::sock;
UDPpacket* ClientHandler::in, * ClientHandler::out;
FILE* ClientHandler::f;
bool ClientHandler::initialized = false;
Uint32 ClientHandler::ipnum;
SDLNet_SocketSet ClientHandler::set;
data_getter ClientHandler::packet_data;
data_setter ClientHandler::out_data;
extern int udpsend(UDPsocket sock, int channel, UDPpacket* out, UDPpacket* in, Uint32 delay, Uint8 expect, int timeout);

// for commands
std::vector<FOWCommand> ClientHandler::command_queue;

extern Settings user_settings;

void ClientHandler::init()
{
	printf("Initializing Client...\n");

	/* get the host from the commandline */
	host = user_settings.host_name.c_str();
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

	// set up our setter
	out_data.clear();
	out_data.packet = packet;

	// So Here we're going to send commands from the server to the client
	// a command has a type
	out_data.push_back(MESSAGE_CLIENT_COMMAND);
	out_data.push_back(command_queue.size());

	for (auto command : command_queue)
	{	
		out_data.push_back(command.self_ref->id);
		out_data.push_back(command.type);
		switch (command.type)
		{
			case MOVE:
				out_data.push_back(command.position.x);
				out_data.push_back(command.position.y);
				break;
			case GATHER:
				out_data.push_back(command.target->id);
				break;
			case BUILD_BUILDING:
				out_data.push_back(((FOWGatherer*)command.self_ref)->building_type);
				out_data.push_back(command.position.x);
				out_data.push_back(command.position.y);
				break;
			case BUILD_UNIT:
				out_data.push_back(((FOWBuilding*)command.self_ref)->entity_to_build);
				break;
			case ATTACK_MOVE:
				out_data.push_back(command.position.x);
				out_data.push_back(command.position.y);
				break;
		}
	}
	command_queue.clear();
	packet->len = out_data.i;	// is this the size like above?

	return packet;
}

int ClientHandler::recieve_gatherer_data(FOWGatherer* specific_character)
{
	// we're going to hack in getting gold until discrete players are in
	auto holding_gold = specific_character->has_gold;
	int has_gold = packet_data.get_data();
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
	return 0;	// this is no longer necissary
}

int ClientHandler::recieve_character_data(FOWCharacter *specific_character)
{
	// Get and set the characters state
	int character_flip = packet_data.get_data();
	int character_state = packet_data.get_data();

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
		int num_stops = packet_data.get_data();
		// so we're going to assume for now, if the path size matches,
		// nothing has changed. this isn't a perfect assumption but its pretty close
		// edit:
		// we made a change - this used to be num_stops == specific_character->current_path.size()
		// the idea is that just because the server hit the next spot and we haven't yet doens't mean
		// the entire path needs to be rewritten
		//int check = std::abs((int)(num_stops - specific_character->current_path.size()));
		
		if (num_stops == specific_character->current_path.size())
		{
			packet_data.i += num_stops*2;	// push forward the pointer a bunch to skip data
		}
		else // otherwise lets repopulate current_path
		{
			specific_character->draw_position = specific_character->position;
			specific_character->current_path.clear();
			for (int j = 0; j < num_stops; j++)
			{
				int x = packet_data.get_data();
				int y = packet_data.get_data();
				t_tile* tile_ref = &GridManager::tile_map[x][y];
				specific_character->current_path.push_back(tile_ref);
			}
		}
	}
	else
	{
		// Every state other than moving we just want to draw them where they actually are
		specific_character->draw_position = specific_character->position;
	}

	if ((GridCharacterState)character_state == GRID_IDLE || (GridCharacterState)character_state == GRID_BLOCKED)
	{
		// if the character just started moving, boot up the walk animation
		if (previous_state != GRID_IDLE && previous_state != GRID_BLOCKED)
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
		int attack_target = packet_data.get_data();
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
		recieve_gatherer_data((FOWGatherer*)specific_character);
	}

	return 0;
}

void ClientHandler::ask_for_bind()
{
	out = SDLNet_AllocPacket(65535);
	out->data[0] = MESSAGE_BINDME;
	strcpy((char*)out->data + 1, "Asking for bind");
	out->len = strlen("Asking for bind") + 2;
	udpsend(sock, 0, out, in, 0, 1, TIMEOUT);
	SDLNet_FreePacket(out);
}

void ClientHandler::run()
{
    printf("running client\n");

	// ask the server to bind us
	ask_for_bind();

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
					// this will keep track of the data pointer index for us
					packet_data.clear();
					packet_data.packet = in;

					t_messagetype next_message = (t_messagetype)packet_data.get_data();

					if (next_message == MESSAGE_TILES)
					{
						packet_data.get_data(); // size_x
						packet_data.get_data(); // size_y
						for (int i = 3; i < in->len; i++)
						{
							int tile_index = i - 3;
							int x_pos = ((tile_index / ((int)GridManager::size.x)));
							int y_pos = (tile_index % ((int)GridManager::size.x));
							GridManager::tile_map[y_pos][x_pos].type = (tiletype_t)packet_data.get_data();
						}
						// this line below murders memory if hit a lot
						// need to move genbuffers
						GridManager::calc_all_tiles();
					}

					if (next_message == MESSAGE_ENTITY_DATA)
					{
						int num_entities = packet_data.get_data();
						for (int i = 0; i < num_entities ; i++)
						{
							t_entitymessage new_message;
							new_message.id = packet_data.get_data();
							new_message.type = packet_data.get_data();
							new_message.x = packet_data.get_data();
							new_message.y = packet_data.get_data();

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
					if (next_message == MESSAGE_ENTITY_DETAILED)
					{
						int num_entities = packet_data.get_data();
						for (int i = 2; i < in->len; i = packet_data.i)
						{
							t_entitymessage new_message;
							new_message.id = packet_data.get_data();
							new_message.type = packet_data.get_data();
							new_message.x = packet_data.get_data();
							new_message.y = packet_data.get_data();
							bool visible = packet_data.get_data();
							int team_id = packet_data.get_data();

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
								recieve_character_data((FOWCharacter*)the_entity);
							}
							else if (((entity_types)new_message.type == FOW_TOWNHALL) ||
								((entity_types)new_message.type == FOW_BARRACKS) ||
								((entity_types)new_message.type == FOW_FARM))
							{
								((FOWSelectable*)the_entity)->team_id = team_id;
							}
						}
					}
					else if(next_message == MESSAGE_HELLO)
					{
						strcpy(fname, (char*)in->data + 1);
						printf("fname=%s\n", fname);
					}
					else if (next_message == MESSAGE_BINDME)
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