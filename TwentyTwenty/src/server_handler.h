#pragma once

#include <SDL_net/SDL_net.h>

class ServerHandler
{
public:

	static void init();
    static void start_server();
	static void run();

    static IPaddress serverIP;
    static UDPsocket udpsock;
    static UDPpacket* recv_packet;
    static SDLNet_SocketSet socketset;
    static int numused;
    static const int MAX_PACKET_SIZE = 512;

protected:
};