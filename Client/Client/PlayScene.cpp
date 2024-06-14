#include "stdafx.h"
#include "PlayScene.h"

void PlayScene::ProcessReceivedData(const char* data, int len) {
    char packetType = data[2]; // Assuming the packet type is at the third byte
    switch (packetType) {
    case SC_LOGIN_INFO: {
        std::cout << "SC_LOGIN_INFO_PACKET" << std::endl;
        SC_LOGIN_INFO_PACKET* packet = (SC_LOGIN_INFO_PACKET*)data;
        curx = packet->x - 10;
        cury = packet->y - 10;
        pl->setup(packet->x, packet->y, packet->exp, packet->hp, packet->level, packet->visual, packet->max_hp);
        pl->id = packet->id;
        break;
    }
    case SC_LOGIN_FAIL: {
        std::cout << "SC_LOGIN_FAIL_PACKET" << std::endl;
        SC_LOGIN_FAIL_PACKET* p = (SC_LOGIN_FAIL_PACKET*)data;
        std::cout << "로그인이 정상적으로 종료되지 않았거나 이미 접속중인 플레이어입니다." << std::endl;
        Sleep(10);
        exit(-1);
        break;
    }
    case SC_CHAT: {
        std::cout << "SC_CHAT_PACKET" << std::endl;
        SC_CHAT_PACKET* packet = (SC_CHAT_PACKET*)data;
        MessageBoxA(NULL, packet->mess, "Chat", MB_OK);
        break;
    }
    case SC_MOVE_OBJECT: {
        std::cout << "SC_MOVE_OBJECT_PACKET" << std::endl;
        SC_MOVE_OBJECT_PACKET* packet = (SC_MOVE_OBJECT_PACKET*)data;
        if (packet->id == pl->id) {
            pl->move(packet->x, packet->y);
            curx = packet->x - 10;
            cury = packet->y - 10;
        }
        else {
            objs[packet->id]->move(packet->x, packet->y);
        }
        break;
    }
    case SC_ADD_OBJECT: {
        std::cout << "SC_ADD_OBJECT_PACKET" << std::endl;
        SC_ADD_OBJECT_PACKET* packet = (SC_ADD_OBJECT_PACKET*)data;
        if (pl->id == packet->id) break;
        objs[packet->id] = new Object();
        objs[packet->id]->setup(packet->x, packet->y, 0, 100, 10, packet->visual, 100, packet->id);
        break;
    }
    case SC_REMOVE_OBJECT: {
        SC_REMOVE_OBJECT_PACKET* packet = (SC_REMOVE_OBJECT_PACKET*)data;
        delete(objs[packet->id]);
        objs.erase(packet->id);
        break;
    }
    default:
        MessageBoxA(NULL, "Unknown packet received", "Error", MB_OK);
        break;
    }
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

void PlayScene::keydown(WPARAM wparam)
{
    POINT player_p = { curx + 10, cury + 10 };
    switch (wparam)
    {
    case VK_UP:
        if (chrono::high_resolution_clock::now() - last_move_time > 1s)
            if (canMove({ player_p.x, player_p.y - 1 }))
                SendMovePacket(0);
        break;
    case VK_DOWN:
        if (chrono::high_resolution_clock::now() - last_move_time > 1s)
            if (canMove({ player_p.x, player_p.y + 1 }))
                SendMovePacket(1);
        break;
    case VK_LEFT:
        if (chrono::high_resolution_clock::now() - last_move_time > 1s)
            if (canMove({ player_p.x - 1, player_p.y }))
                SendMovePacket(2);
        break;
    case VK_RIGHT:
        if (chrono::high_resolution_clock::now() - last_move_time > 1s)
            if (canMove({ player_p.x + 1, player_p.y }))
                SendMovePacket(3);
        break;
    case 'a': // 4방향 공격
    case 'A':
        if (chrono::high_resolution_clock::now() - last_atk_time > 1s)
        {
            last_atk_time = chrono::high_resolution_clock::now();
            auto tm = chrono::high_resolution_clock::now() + 1s; 
            POINT p1 = { player_p.x + 1, player_p.y };
            POINT p2 = { player_p.x - 1, player_p.y };
            POINT p3 = { player_p.x, player_p.y + 1 };
            POINT p4 = { player_p.x, player_p.y - 1 };
            eft_objs.emplace_back(p1, tm);
            eft_objs.emplace_back(p2, tm);
            eft_objs.emplace_back(p3, tm);
            eft_objs.emplace_back(p4, tm);
            SendAtkPacket(0);
        }
        
        break;
    case VK_ESCAPE:
        SendLogoutPacket();
        exit(0);
        break;
    }
}

bool PlayScene::ProcessPackets() {
    if (packets.size() < sizeof(unsigned short)) {
        return false; // Not enough data to even read the size
    }

    unsigned short packet_size = 0;
    std::memcpy(&packet_size, &packets[0], sizeof(packet_size));

    if (packets.size() < packet_size) {
        return false; // Not enough data for a full packet
    }

    std::vector<char> packet_data(packets.begin(), packets.begin() + packet_size);
    packets.erase(packets.begin(), packets.begin() + packet_size);
    ProcessReceivedData(packet_data.data(), packet_size);

    return true;
}

void PlayScene::ReceiveFromServer() {
    char buffer[1024];
    int result;

    while (true) {
        result = recv(g_socket, buffer, sizeof(buffer), 0);
        if (result > 0) {
            std::cout << "Received " << result << " bytes from server." << std::endl;
            packets.insert(packets.end(), buffer, buffer + result);
            while (ProcessPackets()) {
                // Process all complete packets
            }
        }
        else if (result == 0) {
            std::cerr << "Connection closed by server." << std::endl;
            closesocket(g_socket);
            exit(-1);
        }
        else {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK) {
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
    last_move_time = chrono::high_resolution_clock::now();
    CS_MOVE_PACKET packet;
    packet.size = sizeof(packet);
    packet.type = CS_MOVE;
    packet.direction = direction;
    packet.move_time = chrono::duration_cast<std::chrono::seconds>(last_move_time.time_since_epoch()).count();

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

void PlayScene::SendAtkPacket(char atk_type)
{
    CS_ATTACK_PACKET packet;
    packet.size = sizeof(packet);
    packet.type = CS_ATTACK;
    packet.atk_type = atk_type;

    SendToServer((const char*)&packet, packet.size);
}

void PlayScene::SendLogoutPacket()
{
    CS_LOGOUT_PACKET packet;
    packet.size = sizeof(CS_LOGOUT_PACKET);
    packet.type = CS_LOGOUT;

    SendToServer((const char*)&packet, packet.size);
}