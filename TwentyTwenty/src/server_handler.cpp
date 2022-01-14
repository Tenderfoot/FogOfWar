
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <thread>
#include "server_handler.h"

#define ERROR (0xff)
#define TIMEOUT (5000) /*five seconds */

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

int udpsend(UDPsocket sock, int channel, UDPpacket* out, UDPpacket* in, Uint32 delay, Uint8 expect, int timeout)
{
	Uint32 t, t2;
	int err;

	in->data[0] = 0;
	t = SDL_GetTicks();
	do
	{
		t2 = SDL_GetTicks();
		if (t2 - t > (Uint32)timeout)
		{
			printf("timed out\n");
			return(0);
		}
		if (!SDLNet_UDP_Send(sock, channel, out))
		{
			printf("SDLNet_UDP_Send: %s\n", SDLNet_GetError());
			exit(1);
		}
		err = SDLNet_UDP_Recv(sock, in);
		if (!err)
			SDL_Delay(delay);
	} while (!err || (in->data[0] != expect && in->data[0] != ERROR));
	if (in->data[0] == ERROR)
		printf("received error code\n");
	return(in->data[0] == ERROR ? -1 : 1);
}

int udprecv(UDPsocket sock, UDPpacket* in, Uint32 delay, Uint8 expect, int timeout)
{
	Uint32 t, t2;
	int err;

	in->data[0] = 0;
	t = SDL_GetTicks();
	do
	{
		t2 = SDL_GetTicks();
		if (t2 - t > (Uint32)timeout)
		{
			/*printf("timed out\n"); // this is commented to look nicer... */
			return(0);
		}
		err = SDLNet_UDP_Recv(sock, in);
		if (!err)
			SDL_Delay(delay);
	} while (!err || (in->data[0] != expect && in->data[0] != ERROR));
	if (in->data[0] == ERROR)
		printf("received error code\n");
	return(in->data[0] == ERROR ? -1 : 1);
}

void ServerHandler::init()
{
	port = (Uint16)strtol("127.0.0.1", NULL, 0);
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

	initialized = true;

	printf("running server...\n");
	std::thread* test = new std::thread(run);
}

void ServerHandler::run()
{
	while (1)
	{
		in->data[0] = 0;
		printf("waiting...\n");

		while (!SDLNet_UDP_Recv(sock, in))
			SDL_Delay(100); /*1/10th of a second */


		if (in->data[0] != 1 << 4)
		{
			in->data[0] = ERROR;
			in->len = 1;
			SDLNet_UDP_Send(sock, -1, in);
			continue; /* not a request... */
		}

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
		if (SDLNet_UDP_Bind(sock, 0, &ip) == -1)
		{
			printf("SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
			exit(7);
		}
	}
}