
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <thread>
#include "server_handler.h"

const int ServerHandler::MAX_PACKET_SIZE;
IPaddress ServerHandler::serverIP;
UDPsocket ServerHandler::udpsock;
UDPpacket* ServerHandler::recv_packet;
SDLNet_SocketSet ServerHandler::socketset;
int ServerHandler::numused;

// https://gist.github.com/jsaak/f563e60201a71197b6c7b00d369aa4ce
void ServerHandler::init()
{
	socketset = nullptr;

    udpsock = SDLNet_UDP_Open(3333);
    if (!udpsock) {
        printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        exit(2);
    }
    else {
        printf("listening on 0.0.0.0:3333\n");
    }

    socketset = SDLNet_AllocSocketSet(2);
    if (socketset == NULL) {
        fprintf(stderr, "Couldn't create socket set: %s\n", SDLNet_GetError());
        exit(2);
    }

    numused = SDLNet_UDP_AddSocket(socketset, udpsock);
    if (numused == -1) {
        printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
        exit(2);
    }
    
    recv_packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!recv_packet) {
        printf("Could not allocate packet\n");
        exit(2);
    }
}

void ServerHandler::start_server()
{
    printf("running server...\n");
    std::thread(run);
}

void ServerHandler::run()
{
    while (1) {
        SDLNet_CheckSockets(socketset, ~0);
        if (SDLNet_SocketReady(udpsock)) {
            if (SDLNet_UDP_Recv(udpsock, recv_packet)) {
                SDLNet_UDP_Send(udpsock, -1, recv_packet);

                //format log
                int len = recv_packet->len;
                char temp[MAX_PACKET_SIZE + 2];
                memcpy(temp, recv_packet->data, len);

                if (temp[len - 1] == '\n') {
                    temp[len - 1] = '\\';
                    temp[len] = 'n';
                    temp[len + 1] = '\0';
                }
                else {
                    temp[len] = '\0';
                }

                char hoststring[128];
                printf("data: %s\n", recv_packet->data);

            }
        }
    }
}