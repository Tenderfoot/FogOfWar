
#pragma once
#include <stdio.h>
#include <string>
#include <thread>
#include <SDL_net/SDL_net.h>

class FOWGatherer;
class FOWCharacter;
class FOWCommand;

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

    // main functions
    static void init();
    static void run();
    static UDPpacket* send_command_queue();    // Send the clients commands to the server
    static int recieve_character_data(FOWCharacter* specific_character, UDPpacket* packet, int i);   // FOWCharacter
    static int recieve_gatherer_data(FOWGatherer* specific_character, UDPpacket* packet, int i);    // FOWGatherer

    // stuff from SDL_Net
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

    // sending commands
    static std::vector<FOWCommand> command_queue;

protected:
};