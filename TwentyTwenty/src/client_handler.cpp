
#include "common.h"
#include "client_handler.h"
#include "grid_manager.h"

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
				
				if (numpkts) {
					if (in->data[0] == MESSAGE_SEND_TILES)
					{
						printf("Received tile update data\n");
						for (int i = 0; i < in->len; i++)
						{
							if (i > 3)
							{
								int tile_index = i - 3;
								int x_pos = ((tile_index / ((int)GridManager::size.x)));
								int y_pos = (tile_index % ((int)GridManager::size.x));
								GridManager::tile_map[y_pos][x_pos].type = (tiletype_t)in->data[i];
							}
						}
						GridManager::calc_all_tiles();
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
			out->data[0] = MESSAGE_HELLO;
			strcpy((char*)out->data + 1, "Client to Server");
			out->len = strlen("Client to Server") + 2;
			udpsend(sock, 0, out, in, 0, 1, TIMEOUT);
			last_tick = SDL_GetTicks();
			SDLNet_FreePacket(out);
		}
	}

}