#include "stdafx.h"
#include "PlayScene.h"

void PlayScene::ProcessReceivedData(const char* data, int len)
{
    unsigned short packet_size = 0;
    std::memcpy(&packet_size, &packets[0], sizeof(packet_size));
    char packetType = data[2];
    if (packets.size() >= len)
    {
        switch (packetType)
        {
        case SC_LOGIN_INFO:
        {
            cout << "SC_LOGIN_INFO_PACKET" << endl;
            SC_LOGIN_INFO_PACKET* packet = (SC_LOGIN_INFO_PACKET*)data;
            curx = packet->x - 10;
            cury = packet->y - 10;
            pl->setup(packet->x, packet->y, packet->exp, packet->hp, packet->level, packet->visual, packet->max_hp);
            break;
        }
        case SC_LOGIN_FAIL:
        {
            cout << "SC_LOGIN_FAIL_PACKET" << endl;
            SC_LOGIN_FAIL_PACKET* p = (SC_LOGIN_FAIL_PACKET*)data;
            exit(-1);
            break;
        }
        case SC_CHAT:
        {
            SC_CHAT_PACKET* packet = (SC_CHAT_PACKET*)data;
            MessageBoxA(NULL, packet->mess, "Chat", MB_OK);
        }
        break;
        case SC_MOVE_OBJECT:
        {
            SC_MOVE_OBJECT_PACKET* packet = (SC_MOVE_OBJECT_PACKET*)data;
            pl->move(packet->x, packet->y);
            curx = packet->x - 10;
            cury = packet->y - 10;
            break;
        }
        default:
            MessageBoxA(NULL, "Unknown packet received", "Error", MB_OK);
            break;
        }
        packets.erase(packets.begin(), packets.begin() + packet_size);
    }
    ReceiveFromServer();
}

void PlayScene::SendToServer(const char* data, int len)
{
    WSABUF wsabuf;
    wsabuf.buf = const_cast<char*>(data);
    wsabuf.len = len;

    DWORD bytesSent;
    int result = WSASend(g_socket, &wsabuf, 1, &bytesSent, 0, NULL, NULL);
    if (result == SOCKET_ERROR) {
        MessageBox(NULL, L"Send failed", L"Error", MB_OK);
        exit(-1);
    }
}

void PlayScene::ReceiveFromServer() {
    char buffer[1024];
    int result;

    while (true) {
        result = recv(g_socket, buffer, sizeof(buffer), 0);
        if (result > 0) {
            std::cout << "Received " << result << " bytes from server." << std::endl;
            // Copy received data to packets vector
            packets.insert(packets.end(), buffer, buffer + result);
            // Process the received data stored in the buffer
            ProcessReceivedData(buffer, result);
        }
        else if (result == 0) {
            std::cerr << "Connection closed by server." << std::endl;
            closesocket(g_socket);
            exit(-1);
        }
        else {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK) {
                // No data available, return and try again later
                return;
            }
            else {
                std::cerr << "recv failed with error: " << error << std::endl;
                closesocket(g_socket);
                exit(-1);
            }
        }
    }
}

void PlayScene::SendLoginPacket(const char* name)
{
    CS_LOGIN_PACKET packet;
    packet.size = sizeof(packet);
    packet.type = CS_LOGIN;
    memcpy(packet.name, name, NAME_SIZE);

    SendToServer((const char*)&packet, packet.size);
}

void PlayScene::SendMovePacket(char direction)
{
    CS_MOVE_PACKET packet;
    packet.size = sizeof(packet);
    packet.type = CS_MOVE;
    packet.direction = direction;

    SendToServer((const char*)&packet, packet.size);
}

void PlayScene::SendChatPacket(const char* message)
{
    CS_CHAT_PACKET packet;
    packet.size = sizeof(packet) - CHAT_SIZE + strlen(message) + 1;
    packet.type = CS_CHAT;
    memcpy(packet.mess, message, CHAT_SIZE);

    SendToServer((const char*)&packet, packet.size);
}
