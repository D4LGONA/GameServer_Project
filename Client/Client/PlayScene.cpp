#include "stdafx.h"
#include "PlayScene.h"

void PlayScene::ProcessReceivedData(const char* data, int len)
{
    unsigned short packet_size = 0;
    std::memcpy(&packet_size, &packets[0], sizeof(packet_size));
    // ���⼭ ���� �����͸� �������ݿ� �°� ó���մϴ�.
    char packetType = data[2];
    if (packets.size() >= len)
    {
        switch (packetType)
        {
        case SC_LOGIN_INFO:
        {
            SC_LOGIN_INFO_PACKET* packet = (SC_LOGIN_INFO_PACKET*)data;
            // �α��� ���� ó�� ����
            curx = packet->x - 10;
            cury = packet->y - 10;
            pl->setup(packet->x, packet->y, packet->exp, packet->hp, packet->level, packet->visual, packet->max_hp);
            cout << "SC_LOGIN_INFO_PACKET" << endl;
            break;
        }
        case SC_LOGIN_FAIL:
        {
            SC_LOGIN_FAIL_PACKET* p = (SC_LOGIN_FAIL_PACKET*)data;
            exit(-1);
            break;
        }
        case SC_CHAT:
        {
            SC_CHAT_PACKET* packet = (SC_CHAT_PACKET*)data;
            // ä�� �޽��� ó�� ����
            MessageBoxA(NULL, packet->mess, "Chat", MB_OK);
        }
        break;
        case SC_MOVE_OBJECT:
        {
            SC_MOVE_OBJECT_PACKET* packet = (SC_MOVE_OBJECT_PACKET*)data;
            pl->move(packet->x, packet->y);
        }
        // �߰� ��Ŷ Ÿ�� ó�� ������ ���⿡ �ۼ��մϴ�.
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
    char buffer[1024]; // �ӽ� ����
    WSABUF wsabuf;
    wsabuf.buf = buffer;
    wsabuf.len = sizeof(buffer);

    DWORD bytesReceived = 0;
    DWORD flags = 0;
    WSAOVERLAPPED overlapped = { 0 };

    int result = WSARecv(g_socket, &wsabuf, 1, &bytesReceived, &flags, &overlapped, NULL);
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING) {
            MessageBox(NULL, L"Receive failed", L"Error", MB_OK);
            exit(-1);
        }
    }
    else if (bytesReceived > 0) {
        // ���� �����͸� ���Ϳ� �߰�
        packets.insert(packets.end(), buffer, buffer + bytesReceived);
        unsigned short packet_size = 0;
        std::memcpy(&packet_size, &packets[0], sizeof(packet_size));
        ProcessReceivedData(&packets[0], packet_size);
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