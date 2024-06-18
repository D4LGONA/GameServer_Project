#include "stdafx.h"
#include "PlayScene.h"

void PlayScene::SendChatPacket(const std::wstring& message)
{
    CS_CHAT_PACKET packet;
    ZeroMemory(&packet, sizeof(packet));
    size_t message_len = min(message.length(), static_cast<size_t>(CHAT_SIZE - 1)); // Ensure the message length does not exceed CHAT_SIZE - 1

    // 패킷 크기를 설정합니다.
    packet.size = 3 + message_len * sizeof(wchar_t);
    packet.type = CS_CHAT;

    // 메시지를 패킷에 복사합니다.
    wmemcpy(packet.mess, message.c_str(), message_len);
    packet.mess[message_len] = L'\0'; // 널 종료

    char buffer[sizeof(CS_CHAT_PACKET)];
    memcpy(buffer, &packet, packet.size);

    SendToServer(buffer, packet.size);
}

void PlayScene::keydown(WPARAM wparam) {
    if (wparam == VK_RETURN) {
        if (in_chat_mode) {
            cout << "글자 수: " << chat_message.size() << endl;
            if (chat_message.size() != 0) 
                SendChatPacket(chat_message);
            chat_message.clear();
            in_chat_mode = false;
        }
        else {
            in_chat_mode = true;
            chat_message.clear();
        }
    }
    else if (!in_chat_mode and !death) {
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
        case 'A': // 4방향 공격
        case 'a':
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
}

void PlayScene::ProcessReceivedData(const char* data, int len) {
    const char* packet_end = data + len;
    const char* current_packet = data;

    while (current_packet < packet_end) {
        unsigned short packet_size = *reinterpret_cast<const unsigned short*>(current_packet);
        if (current_packet + packet_size > packet_end) {
            // Not enough data for a full packet, wait for more data
            break;
        }

        char packetType = current_packet[2];

        switch (packetType) {
        case SC_LOGIN_INFO: {
            std::cout << "SC_LOGIN_INFO_PACKET" << std::endl;
            SC_LOGIN_INFO_PACKET* packet = (SC_LOGIN_INFO_PACKET*)current_packet;
            curx = packet->x - 10;
            cury = packet->y - 10;
            pl->setup(packet->x, packet->y, packet->exp, packet->hp, packet->level, packet->visual, packet->max_hp);
            pl->id = packet->id;
            
            break;
        }
        case SC_LOGIN_FAIL: {
            std::cout << "SC_LOGIN_FAIL_PACKET" << std::endl;
            SC_LOGIN_FAIL_PACKET* p = (SC_LOGIN_FAIL_PACKET*)current_packet;
            std::cout << "로그인이 정상적으로 종료되지 않았거나 이미 접속중인 플레이어입니다." << std::endl;
            Sleep(1000);
            exit(-1);
            break;
        }
        case SC_CHAT: {
            std::cout << "SC_CHAT_PACKET" << std::endl;
            SC_CHAT_PACKET* packet = (SC_CHAT_PACKET*)current_packet;

            // 채팅 메시지를 chat_messages 벡터에 추가합니다.
            wstring message{ packet->mess, (packet->size - 23) / sizeof(wchar_t) };
            string s{ packet->name };
            wstring chating = L"[" + strtowstr(s) + L"]: " + message;
            chat_messages.push_back(chating);

            break;
        }
        case SC_MOVE_OBJECT: {
            std::cout << "SC_MOVE_OBJECT_PACKET" << std::endl;
            SC_MOVE_OBJECT_PACKET* packet = (SC_MOVE_OBJECT_PACKET*)current_packet;
            if (packet->id == pl->id) {
                pl->move(packet->x, packet->y);
                curx = packet->x - 10;
                cury = packet->y - 10;
            }
            else {
                if (objs[packet->id] == nullptr) break;
                objs[packet->id]->move(packet->x, packet->y);
            }
            break;
        }
        case SC_ADD_OBJECT: {
            std::cout << "SC_ADD_OBJECT_PACKET" << std::endl;
            SC_ADD_OBJECT_PACKET* packet = (SC_ADD_OBJECT_PACKET*)current_packet;
            if (pl->id == packet->id) break;
            objs[packet->id] = new Object();
            objs[packet->id]->setup(packet->x, packet->y, 0, 100, 10, packet->visual, 100, packet->id, packet->name);
            break;
        }
        case SC_REMOVE_OBJECT: {
            SC_REMOVE_OBJECT_PACKET* packet = (SC_REMOVE_OBJECT_PACKET*)current_packet;
            if (objs.find(packet->id) != objs.end()) {
                delete objs[packet->id];
                objs.erase(packet->id);
            }
            break;
        }
        case SC_STAT_CHANGE: {
            SC_STAT_CHANGE_PACKET* packet = (SC_STAT_CHANGE_PACKET*)current_packet;
            if (packet->id == pl->id)
            {
                pl->level = packet->level;
                pl->hp = packet->hp;
                pl->exp = packet->exp;
                pl->maxhp = packet->max_hp;
                if (pl->hp <= 0)
                    death = true;
                else
                    death = false;
            }
            else
            {
                if (objs.find(packet->id) == objs.end() or objs[packet->id] == nullptr) break;
                objs[packet->id]->level = packet->level;
                objs[packet->id]->hp = packet->hp;
                objs[packet->id]->exp = packet->exp;
                objs[packet->id]->maxhp = packet->max_hp;
            }
            break;
        }
        default:
            break;
        }
        current_packet += packet_size;
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

void PlayScene::ReceiveFromServer() {
    char buffer[1024];
    int result;

    while (true) {
        result = recv(g_socket, buffer, sizeof(buffer), 0);
        if (result > 0) {
            std::cout << "Received " << result << " bytes from server." << std::endl;
            packets.insert(packets.end(), buffer, buffer + result);

            // Process as many packets as possible from the buffer
            while (packets.size() > sizeof(unsigned short)) {
                unsigned short packet_size = *reinterpret_cast<const unsigned short*>(&packets[0]);
                if (packets.size() < packet_size) {
                    // Not enough data for a full packet
                    break;
                }
                ProcessReceivedData(packets.data(), packet_size);
                packets.erase(packets.begin(), packets.begin() + packet_size);
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
    strncpy_s(packet.name, name, NAME_SIZE - 1);
    packet.name[NAME_SIZE - 1] = '\0'; // 널 종료

    SendToServer(reinterpret_cast<const char*>(&packet), packet.size);
}

void PlayScene::SendMovePacket(char direction)
{
    last_move_time = chrono::high_resolution_clock::now();
    CS_MOVE_PACKET packet;
    packet.size = sizeof(packet);
    packet.type = CS_MOVE;
    packet.direction = direction;
    packet.move_time = chrono::duration_cast<std::chrono::seconds>(last_move_time.time_since_epoch()).count();

    SendToServer(reinterpret_cast<const char*>(&packet), packet.size);
}

void PlayScene::SendAtkPacket(char atk_type)
{
    CS_ATTACK_PACKET packet;
    packet.size = sizeof(packet);
    packet.type = CS_ATTACK;
    packet.atk_type = atk_type;

    SendToServer(reinterpret_cast<const char*>(&packet), packet.size);
}

void PlayScene::SendLogoutPacket()
{
    CS_LOGOUT_PACKET packet;
    packet.size = sizeof(CS_LOGOUT_PACKET);
    packet.type = CS_LOGOUT;

    SendToServer(reinterpret_cast<const char*>(&packet), packet.size);
}
