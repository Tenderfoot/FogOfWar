
#pragma once
#include <stdio.h>
#include <string>
#include <thread>
#include <SDL_net/SDL_net.h>

class FOWGatherer;
class FOWCharacter;

typedef struct
{
    int id;
    int type;
    int x;
    int y;
}t_entitymessage;

class ClientHandler
{
public:

    static void init();
    static void run();
    static int recieve_character_data(FOWCharacter* specific_character, UDPpacket* packet, int i);   // FOWCharacter
    static int recieve_gatherer_data(FOWGatherer* specific_character, UDPpacket* packet, int i);    // FOWGatherer

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