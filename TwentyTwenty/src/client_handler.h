
#pragma once
#include <stdio.h>
#include <string>
#include <thread>
#include <mutex>
#include <SDL_net/SDL_net.h>
#include "entity.h"

class FOWGatherer;
class FOWCharacter;
class FOWCommand;
class FOWBuilding;

bool is_unit(entity_types type);
bool is_building(entity_types type);

typedef struct
{
    int id;
    int type;
    int x;
    int y;
}t_entitymessage;

// This thing keeps our position in the data pointer
// every time we get something out it increments
typedef struct data_getter
{
    int i=0;
    UDPpacket* packet;
    void clear()
    {
        i = 0;
    }

    int get_data()
    {
        int to_return = SDLNet_Read32(&packet->data[i]);
        i+=4;
        return to_return;
    }
};

typedef struct data_setter
{
    int i = 0;
    UDPpacket* packet;
    void clear()
    {
        i = 0;
    }

    void push_back(int data)
    {
        SDLNet_Write32(data, &packet->data[i]);
        i+=4;
    }
};

class ClientHandler
{
public:

    // main functions
    static void init();
    static void run();
    static UDPpacket* send_command_queue();    // Send the clients commands to the server
    static void ask_for_bind();
    static void ask_for_map_info();
    static void recieve_character_data(FOWCharacter* specific_character);   // FOWCharacter
    static void recieve_gatherer_data(FOWGatherer* specific_character);    // FOWGatherer
    static void recieve_building_data(FOWBuilding* specific_building);   // FOWCharacter
    static void handle_message_tiles();
    static void handle_entity_data();
    static void handle_entity_detailed();


    // stuff from SDL_Net
    static Uint16 port;
    static const char* host, * fname, * fbasename;
    static Sint32 flen, pos, p2;
    static int len, blocks, err;
    static Uint32 ack;
    static IPaddress ip;
    static UDPsocket sock;
    static UDPpacket* in, * out;
    static FILE* f;
    static bool initialized;
    static Uint32 ipnum;
    static SDLNet_SocketSet set;
    static data_getter packet_data;
    static data_setter out_data;
    static std::string mapname;

    // sending commands
    static std::vector<FOWCommand> command_queue;

protected:
};