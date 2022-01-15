
#pragma once
#include <stdio.h>
#include <string>
#include <thread>
#include <SDL_net/SDL_net.h>

class ClientHandler
{
public:

    static void init();
    static void run();

    static Uint16 port;
    static const char* host, * fname, * fbasename;
    static Sint32 flen, pos, p2;
    static int len, blocks, i, err;
    static Uint32 ack;
    static IPaddress ip;
    static UDPsocket sock;
    static UDPpacket* in, * out;
    static FILE* f;
    static bool initialized;
    static Uint32 ipnum;
    static SDLNet_SocketSet set;

protected:
};