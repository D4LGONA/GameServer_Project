#pragma once

#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <random>
#include <thread>
#include <unordered_set>
#include <concurrent_priority_queue.h>
#include <sqlext.h> 
#include <chrono>
#include <array>
#include <mutex>
#include <string>
#include <sstream>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;

#include "protocol.h"

constexpr int EYESIGHT = 5;
constexpr int BUFSIZE = 256;
constexpr int SECTOR_SIZE = 50;
atomic_int id = 0;
atomic_int npcid = 1;
constexpr POINT Nears[] =
{
    {-1, -1}, {-1, 0}, {-1, 1},
    {0, -1}, {0, 0}, {0, 1},
    {1, -1}, {1, 0}, {1, 1}
};


enum class TASK_TYPE
{
    // OV_TYPE
    ACCEPT,
    RECV,
    SEND,
    RANDOM_MOVE,
    FOLLOW_MOVE,
    DB_UPDATE,

    // EVENTS
    EV_DB_UPDATE = DB_UPDATE,
    EV_RANDOM_MOVE = RANDOM_MOVE,
    EV_FOLLOW_MOVE = FOLLOW_MOVE
};

enum PK_TYPE
{
    LOGIN_INFO = 2,
    LOGIN_FAIL = 3,
    ADD_OBJECT = 4,
    REMOVE_OBJECT = 5,
    MOVE_OBJECT = 6,
    CHAT = 7,
    STAT_CHANGE = 8
};

class EVENT
{
public:
    TASK_TYPE evt_type;
    int to_id; // 누구에게
    int from_id; // 누가
    chrono::system_clock::time_point do_time;

    EVENT() {}

    void setup(TASK_TYPE evt, int s_time, int from = -1, int to = -1)// time->s
    {
        evt_type = evt;
        from_id = from;
        to_id = to;
        do_time = chrono::system_clock::now() + chrono::seconds(s_time);
    }

    chrono::system_clock::time_point& GETTIME() { return do_time; }

    bool operator<(const EVENT& other) const
    {
        return do_time > other.do_time;
    }
};

class EXT_OVER // overlapped, packet size, type
{
public:
    WSAOVERLAPPED over;
    WSABUF wsabuf;
    char wb_buf[BUFSIZE];
    TASK_TYPE ov;
    int from;
    int to;

    EXT_OVER() // recv
    {
        wsabuf.len = BUFSIZE;
        wsabuf.buf = wb_buf;
        ov = TASK_TYPE::RECV;
        ZeroMemory(&over, sizeof(over));
    }

    ~EXT_OVER() {}

    void setup_send(char* pk) // send
    {
        wsabuf.len = pk[0];
        wsabuf.buf = wb_buf;
        ZeroMemory(&over, sizeof(over));
        ov = TASK_TYPE::SEND;
        memcpy(wb_buf, pk, pk[0]);
    }
};


int setid();
int setid_npc();
std::wstring strtowstr(const std::string& str);
void server_error(const char* msg);

#include "Object.h"
#include "Monster.h"
#include "Player.h"

extern array<Monster, MAX_NPC> npcs;
extern array<Player, MAX_USER> players;
extern array<array<unordered_set<int>, W_HEIGHT / SECTOR_SIZE + 1>, W_WIDTH / SECTOR_SIZE + 1> g_SectorList;
extern mutex g_SectorLock;