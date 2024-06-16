#include "stdafx.h"
#include "DB_access.h"
#include "Object.h"
#include "Player.h"
#include "Monster.h"

// iocp에 관련된 전역 변수들 //
HANDLE g_iocp_handle;
SOCKET g_server;
SOCKET g_client;

// DB관련 //
thread_local SQLHDBC hdbc = SQL_NULL_HDBC;
thread_local SQLHSTMT hstmt = SQL_NULL_HSTMT;
concurrency::concurrent_priority_queue<EVENT> g_evt_queue;

// 그 외 //
array<Player, MAX_USER> players;
array<Monster, MAX_NPC> npcs;
array<array<unordered_set<int>, W_HEIGHT / SECTOR_SIZE + 1>, W_WIDTH / SECTOR_SIZE + 1> g_SectorList;
mutex g_SectorLock;

// 함수 전방선언 //
void push_evt_queue(int, int, TASK_TYPE, int);
void initialize_server();
void check_evt(HANDLE);
void initialize_monster();

void wk_thread(HANDLE iocp_hd)
{
    while (true)
        if (DB_connect(hdbc, hstmt)) break;

    while (true)
    {
        DWORD num_bytes;
        ULONG_PTR key;
        WSAOVERLAPPED* over;
        BOOL ret;
        ret = GetQueuedCompletionStatus(iocp_hd, &num_bytes, &key, &over, INFINITE);
        if (ret == FALSE)
        {
            if (over == nullptr)
            {
                printf("GetQueuedCompletionStatus failed with error: %d\n", GetLastError());
                
                continue;
            }

            // 요기를 굳이 해야 할까
            int player_id = static_cast<int>(key);
            Player& player = players[player_id];
            int error = WSAGetLastError();
            
            DB_user_logout(string(player.getName()), hstmt);

            printf("Client [%d] encountered an error: %d\n", player_id, error);
            players[player_id].state = NONE; // todo: state lock?
            continue;
        }

        int player_id = static_cast<int>(key);
        EXT_OVER* ext_over = reinterpret_cast<EXT_OVER*>(over);

        if (ext_over->ov == TASK_TYPE::ACCEPT)
        {
            cout << "ACCEPT: " << g_client << endl;
            int client_id = setid(); 
            cout << client_id << endl;
            if (client_id != -1) {
                Player& player = players[client_id];
                // lock을 여기 해야 할까
                player.setup(client_id, g_client);
                CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_client), iocp_hd, client_id, 0);
                player.recv();
                g_client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
            }
            else 
                cout << "Max user exceeded.\n";

            EXT_OVER ac_over; // 괜찮을까?
            ac_over.ov = TASK_TYPE::ACCEPT;
            AcceptEx(g_server, g_client, ac_over.wb_buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, 0, &ac_over.over);
        }
        else if (ext_over->ov == TASK_TYPE::RECV)
        {
            Player& player = players[player_id];
            player.update_packet(ext_over, num_bytes);
            player.process_buffer(hstmt);

            player.recv();
        }
        else if (ext_over->ov == TASK_TYPE::SEND)
        {
            delete ext_over;
        }
        else if (ext_over->ov == TASK_TYPE::DB_UPDATE) // DB에 user 값들 업데이트.
        {
            for (Player& p : players)
            {
                if (p.get_state() != PLAYING) continue;
                const char* user_name = p.getName();
                short user_x = p.getX();
                short user_y = p.getY();
                int user_exp = p.getEXP();

                wchar_t wquery[512];
                swprintf_s(wquery, sizeof(wquery) / sizeof(wquery[0]),
                    L"EXEC UpdateUserDataByName @user_name = '%S', @user_x = %d, @user_y = %d, @user_exp = %d;",
                    user_name, user_x, user_y, user_exp);

                ret = SQLExecDirect(hstmt, wquery, SQL_NTS);
                if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
                    printf("Query executed successfully.\n");
                else
                    printf("Error executing query.\n");
            }
            push_evt_queue(-1, -1, TASK_TYPE::EV_DB_UPDATE, 5); // 300으로 고쳐야함
        }
        else if (ext_over->ov == TASK_TYPE::RANDOM_MOVE)
        {

        }
        else if (ext_over->ov == TASK_TYPE::FOLLOW_MOVE)
        {

        }
    }

    DB_disconnect(hdbc, hstmt);
}

int main()
{
    initialize_server();
    initialize_monster();

    push_evt_queue(-1, -1, TASK_TYPE::EV_DB_UPDATE, 5);

    // doing acceptEX
    g_client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    EXT_OVER ac_over;
    ac_over.ov = TASK_TYPE::ACCEPT;
    AcceptEx(g_server, g_client, ac_over.wb_buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, 0, &ac_over.over);

    // add threads
    thread evt_thread{ check_evt, g_iocp_handle }; // PostQueuedCompletionStatus 하는 애
    vector <thread> worker_threads;
    for (int i = 0; i < int(thread::hardware_concurrency()); ++i)
        worker_threads.emplace_back(wk_thread, g_iocp_handle);
    for (auto& th : worker_threads)
        th.join();
    evt_thread.join();
    closesocket(g_server);
    WSACleanup();
}

void check_evt(HANDLE iocp_hd)
{
    while (true)
    {
        EVENT ev;
        bool event_processed = false;

        if (g_evt_queue.try_pop(ev))
        {
            if (ev.GETTIME() <= chrono::system_clock::now())
            {
                EXT_OVER* ex_over = new EXT_OVER();
                ex_over->ov = ev.evt_type; 
                ex_over->from = ev.from_id;
                ex_over->to = ev.to_id;

                PostQueuedCompletionStatus(iocp_hd, 0, ev.to_id, &ex_over->over);
            }
            else
            {
                g_evt_queue.push(ev);
                this_thread::sleep_for(chrono::milliseconds(10));
            }
        }
    }
}

void initialize_monster()
{
    for (auto& a : npcs)
    {
        a.setup(setid_npc());
    }
    cout << "몬스터 초기화 완료" << endl;
}

void initialize_server()
{
    setlocale(LC_ALL, "korean");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        server_error("WSAStartup failed");

    g_server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (g_server == INVALID_SOCKET)
        server_error("WSASocket failed");

    // 소켓을 비동기 모드로 설정
    u_long mode = 1;
    if (ioctlsocket(g_server, FIONBIO, &mode) != NO_ERROR) {
        server_error("ioctlsocket failed");
    }

    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_NUM);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(g_server, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
        server_error("bind failed");

    if (listen(g_server, SOMAXCONN) == SOCKET_ERROR)
        server_error("listen failed");

    g_iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (g_iocp_handle == NULL)
        server_error("CreateIoCompletionPort failed");

    if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_server), g_iocp_handle, 0, 0) == NULL)
        server_error("CreateIoCompletionPort for server socket failed");
}


void push_evt_queue(int from, int to, TASK_TYPE ev, int time) // time: second 단위.
{
    // todo
	EVENT evt;
	switch (ev)
	{
    case TASK_TYPE::EV_DB_UPDATE:
		evt.setup(ev, time, from, to);
		break;
    case TASK_TYPE::EV_RANDOM_MOVE:
		evt.setup(ev, time, from, to);
        break;
	default:
		break;
	}

	g_evt_queue.push(evt);
}