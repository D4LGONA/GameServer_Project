#include "stdafx.h"
#include "Player.h"
#include "Monster.h"

void Player::send(void* packet)
{
	EXT_OVER* ov = new EXT_OVER();
	unsigned short p_size;
	memcpy(&p_size, packet, 2);
	ov->setup_send(reinterpret_cast<char*>(packet), p_size);

	int result = WSASend(socket, &ov->wsabuf, 1, 0, 0, &ov->over, NULL);
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			printf("WSASend failed with error: %d\n", error);
			{
				std::lock_guard<std::mutex> lock(st_lock);
				state = NONE;
			}
			closesocket(socket);
		}
	}
}

void Player::recv()
{
	if (socket == INVALID_SOCKET) {
		printf("Invalid socket\n");
		return;
	}

	ZeroMemory(&over.over, sizeof(over.over));
	over.wsabuf.len = BUFSIZE - packet_data.size();
	over.wsabuf.buf = over.wb_buf + packet_data.size();

	if (over.wsabuf.buf == nullptr) {
		printf("Buffer is null\n");
		return;
	}

	DWORD flags = 0;
	DWORD bytesReceived = 0;

	int result = WSARecv(socket, &over.wsabuf, 1, &bytesReceived, &flags, &over.over, NULL);
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			printf("WSARecv failed with error: %d\n", error);
			{
				std::lock_guard<std::mutex> lock(st_lock);
				state = NONE;
			}
			closesocket(socket);
			WSACleanup();
		}
	}
}

void Player::handle_packet(char* packet, unsigned short length, SQLHSTMT& hstmt)
{
    char type = packet[2];

    switch (type)
    {
    case CS_LOGIN:// ok
    {
        CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

        std::string str(p->name);
        std::wstring ws = strtowstr(str);
        std::wstring query = L"SELECT * FROM user_table WHERE user_name = '" + ws + L"';";

        SQLCloseCursor(hstmt);
        SQLRETURN ret = SQLExecDirect(hstmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            SQLLEN cbID = 0, cbx = 0, cby = 0, cblevel = 0, cbexp = 0, cbhp = 0, cbatk = 0, cbdef = 0, cbvisual = 0, cbd = 0;
            bool playing;
            ret = SQLBindCol(hstmt, 1, SQL_C_CHAR, name, 20, &cbID);
            ret = SQLBindCol(hstmt, 2, SQL_C_SHORT, &x, 10, &cbx);
            ret = SQLBindCol(hstmt, 3, SQL_C_SHORT, &y, 10, &cby);
            ret = SQLBindCol(hstmt, 4, SQL_C_SHORT, &level, 10, &cblevel);
            ret = SQLBindCol(hstmt, 5, SQL_C_LONG, &exp, 10, &cbexp);
            ret = SQLBindCol(hstmt, 6, SQL_C_SHORT, &hp, 10, &cbhp);
            ret = SQLBindCol(hstmt, 7, SQL_C_SHORT, &attack, 10, &cbatk);
            ret = SQLBindCol(hstmt, 8, SQL_C_SHORT, &defense, 10, &cbdef);
            ret = SQLBindCol(hstmt, 9, SQL_C_LONG, &visual, 10, &cbvisual);
            ret = SQLBindCol(hstmt, 10, SQL_C_BIT, &playing, 1, &cbd);

            for (int i = 0; ; i++) {
                ret = SQLFetch(hstmt);
                if (ret == SQL_NO_DATA && i == 0)
                {
                    std::wstring insert_query = L"INSERT INTO user_table (user_name, user_x, user_y, user_level, user_exp, user_hp, user_atk, user_def, user_visual, user_isplay) VALUES ('" +
                        ws + L"', " +
                        to_wstring(x) + L", " +
                        to_wstring(y) + L", " +
                        to_wstring(level) + L", " +
                        to_wstring(exp) + L", " +
                        to_wstring(hp) + L", " +
                        to_wstring(attack) + L", " +
                        to_wstring(defense) + L", " +
                        to_wstring(visual) + L", " +
                        to_wstring(1) + L");";

                    SQLCloseCursor(hstmt);
                    ret = SQLExecDirect(hstmt, (SQLWCHAR*)insert_query.c_str(), SQL_NTS);
                    break;
                }
                if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
                {
                    if (playing)
                    {
                        send_login_fail();
                        {
                            std::lock_guard<std::mutex> lock(st_lock);
                            state = NONE;
                        }
                        return;
                    }
                    else
                    {
                        std::wstring update_query = L"UPDATE user_table SET user_isplay = 1 WHERE user_name = '" + ws + L"';";
                        SQLCloseCursor(hstmt);
                        ret = SQLExecDirect(hstmt, (SQLWCHAR*)update_query.c_str(), SQL_NTS);
                        break;
                    }
                }
            }
        }
        memcpy(name, p->name, NAME_SIZE);
        max_hp = hp;
        send_login_info();
        {
            std::lock_guard<std::mutex> ll{ g_SectorLock }; // sector에 추가하는 부분
            sector_x = x / SECTOR_SIZE;
            sector_y = y / SECTOR_SIZE;
            g_SectorList[sector_x][sector_y].insert(id);
        }

        // view list 만들고 주변 npc 깨우기
        for (const auto& n : Nears) {
            short sx = sector_x + n.x;
            short sy = sector_y + n.y;

            if (sx >= 0 && sx <= W_WIDTH / SECTOR_SIZE && sy >= 0 && sy <= W_HEIGHT / SECTOR_SIZE) {
                std::lock_guard<std::mutex> ll{ g_SectorLock };
                for (auto& i : g_SectorList[sx][sy]) {
                    if (!isNear(i)) continue;
                    if (i == id) continue;
                    if (i < 0)
                    {
                        int k = (i * -1) - 1;
                        if (npcs[k].active) continue;
                        npcs[k].active = true;
                        npcs[k].doing_Ai(id);
                        send_add_object(npcs[k].x, npcs[k].y, npcs[k].name, npcs[k].id, npcs[k].visual);
                        send_stat_change(0, npcs[k].hp, npcs[k].level, npcs[k].max_hp, npcs[k].id);
                    }
                    else
                    {
                        if (players[i].get_state() != PLAYING)
                            continue;
                        players[i].send_add_object(x, y, name, id, visual);
                        players[i].send_stat_change(exp, hp, level, max_hp, id);
                        send_add_object(players[i].x, players[i].y, players[i].name, players[i].id, players[i].visual);
                        send_stat_change(players[i].exp, players[i].hp, players[i].level, players[i].max_hp, players[i].id);
                    }
                    view_list.insert(i);
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(st_lock);
            state = PLAYING;
        }

        for (auto& i : players)
        {
            if (i.get_state() != PLAYING) continue;
            std::string s = "system";
            std::wstring cont = strtowstr(name); // char[] name을 std::wstring으로 변환
            cont += L"이 접속했다.";
            i.send_chat(s.c_str(), cont.c_str());
        }

        break;
    }
    case CS_MOVE:// ok
    {
        CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);

        switch (p->direction)
        {
        case 0: // up
            if (can_move(x, y - 1))
                y--;
            break;
        case 1: // down
            if (can_move(x, y + 1))
                y++;
            break;
        case 2: // left
            if (can_move(x - 1, y))
                x--;
            break;
        case 3: // right
            if (can_move(x + 1, y))
                x++;
            break;
        }

        if (x / SECTOR_SIZE != sector_x || y / SECTOR_SIZE != sector_y)
        {
            std::lock_guard<std::mutex> ll{ g_SectorLock };
            g_SectorList[sector_x][sector_y].erase(id);
            sector_x = x / SECTOR_SIZE;
            sector_y = y / SECTOR_SIZE;
            g_SectorList[sector_x][sector_y].insert(id);
        }

        last_move_time = p->move_time;

        vl_l.lock();
        std::unordered_set<int> old_viewlist = view_list;
        vl_l.unlock();
        std::unordered_set<int> new_viewlist;

        for (const auto& n : Nears) {
            short sx = sector_x + n.x;
            short sy = sector_y + n.y;

            if (sx >= 0 && sx <= W_WIDTH / SECTOR_SIZE && sy >= 0 && sy <= W_HEIGHT / SECTOR_SIZE) {
                std::lock_guard<std::mutex> sector_ll(g_SectorLock);
                for (auto& i : g_SectorList[sx][sy]) {
                    if (!isNear(i)) continue;
                    if (i == id) continue;

                    if (i < 0) {
                        int a = i * (-1) - 1;
                        if (npcs[a].hp <= 0) continue;
                    }
                    else if (players[abs(i)].get_state() != PLAYING) {
                        continue;
                    }
                    new_viewlist.insert(i);
                }
            }
        }

        send_move_object(x, y, id, last_move_time);

        for (auto& i : new_viewlist) {
            if (old_viewlist.count(i) == 0) { // 새로 들어온 객체
                if (i < 0) {
                    int k = -1 * i - 1;
                    send_add_object(npcs[k].x, npcs[k].y, npcs[k].name, npcs[k].id, npcs[k].visual);
                    send_stat_change(0, npcs[k].hp, npcs[k].level, npcs[k].max_hp, npcs[k].id);
                    if (npcs[k].active) continue;
                    bool expt = false;
                    if (true == atomic_compare_exchange_strong(&npcs[k].active, &expt, true))
                        npcs[k].doing_Ai(id);
                }
                else {
                    players[i].send_add_object(x, y, name, id, visual);
                    players[i].send_stat_change(exp, hp, level, max_hp, id);
                    send_add_object(players[i].x, players[i].y, players[i].name, players[i].id, players[i].visual);
                    send_stat_change(players[i].exp, players[i].hp, players[i].level, players[i].max_hp, players[i].id);
                }
            }
            else { // 기존에 있던 객체
                if (i >= 0) {
                    players[i].send_move_object(x, y, id, last_move_time);
                }
            }
        }

        for (auto& i : old_viewlist) {
            if (new_viewlist.count(i) == 0) { // 멀어진 객체
                if (i < 0) {
                    send_remove_object(i);
                }
                else {
                    send_remove_object(i);
                    players[i].send_remove_object(id);
                }
            }
        }

        view_list = new_viewlist;
        break;
    }
    case CS_CHAT:
    {
        CS_CHAT_PACKET* p = reinterpret_cast<CS_CHAT_PACKET*>(packet);
        wstring message{ p->mess, (p->size - 3) / sizeof(wchar_t) };
        message += L'\0';
        wcout << message << endl;

        // 모든 플레이어에게 채팅 메시지를 브로드캐스트
        for (auto& a : players)
        {
            if (a.get_state() != PLAYING) continue;
            a.send_chat(name, message.c_str());
        }
        break;
    }
    case CS_ATTACK:
    {
        CS_ATTACK_PACKET* p = (CS_ATTACK_PACKET*)packet;
        switch (p->atk_type)
        {
        case 0:
        {
            for (auto& a : view_list)
            {
                if (a >= 0) continue;
                int idx = (a * -1) - 1;
                int target_x = npcs[idx].x;
                int target_y = npcs[idx].y;

                bool is_adjacent =
                    (target_x == x && (target_y == y - 1 || target_y == y + 1)) || // 상하
                    (target_y == y && (target_x == x - 1 || target_x == x + 1)); // 좌우

                if (is_adjacent)
                {
                    npcs[idx].hp -= attack; 

                    std::string s = "system";
                    std::wstring cont = L""; // char[] name을 std::wstring으로 변환
                    cont += strtowstr(npcs[idx].name) + L"에게" + to_wstring(attack) +L"의 데미지를 입혔다.";
                    send_chat(s.c_str(), cont.c_str());

                    if (npcs[idx].type == Follow)
                        npcs[idx].target_id = id;

                    for (const auto& n : Nears) {
                        short sx = sector_x + n.x;
                        short sy = sector_y + n.y;

                        if (sx >= 0 && sx <= W_WIDTH / SECTOR_SIZE && sy >= 0 && sy <= W_HEIGHT / SECTOR_SIZE) {
                            std::lock_guard<std::mutex> sector_ll(g_SectorLock);
                            for (auto& i : g_SectorList[sx][sy]) {
                                if (!isNear(i)) continue;
                                if (i < 0) continue;
                                if (players[i].get_state() != PLAYING) continue;
                                players[i].send_stat_change(0, npcs[idx].hp, npcs[idx].level, npcs[idx].max_hp, npcs[idx].id);  // 상태변환.
                            }
                        }
                    }

                    if (npcs[idx].hp <= 0) // 죽음 조건
                    {
                        for (const auto& n : Nears) {
                            short sx = sector_x + n.x;
                            short sy = sector_y + n.y;

                            if (sx >= 0 && sx <= W_WIDTH / SECTOR_SIZE && sy >= 0 && sy <= W_HEIGHT / SECTOR_SIZE) { 
                                std::lock_guard<std::mutex> sector_ll(g_SectorLock);
                                for (auto& i : g_SectorList[sx][sy]) {
                                    if (!isNear(i)) continue;
                                    if (i < 0) continue;
                                    if (players[i].get_state() != PLAYING) continue;

                                    int old_exp = players[i].exp;
                                    int old_level = players[i].level;

                                    players[i].exp += npcs[idx].level * 50;
                                    while (players[i].exp >= players[i].level * 100) {
                                        players[i].exp -= players[i].level * 100;
                                        players[i].level += 1;
                                        players[i].attack += 10;
                                        players[i].hp += 50;
                                    }

                                    // 경험치와 레벨 변화 알림
                                    if (players[i].exp != old_exp || players[i].level != old_level) {
                                        std::string s = "system";
                                        std::wstring cont = L"";
                                        cont += L"경험치가 " + to_wstring(players[i].exp - old_exp) + L"만큼 증가했다.";
                                        players[i].send_chat(s.c_str(), cont.c_str());

                                        if (players[i].level != old_level) {
                                            cont = L"레벨이 " + to_wstring(players[i].level) + L"(으)로 증가했다.";
                                            players[i].send_chat(s.c_str(), cont.c_str());
                                        }
                                    }

                                    players[i].send_remove_object(a); // 모든 플레이어에게 remove
                                    push_evt_queue(a, a, TASK_TYPE::EV_RESPAWN, respawnTime);
                                }
                            }
                        }
                        npcs[idx].active = false;
                        for (auto& i : players)
                        {
                            if (i.get_state() != PLAYING) continue;
                            std::string s = "system";
                            std::wstring cont = strtowstr(npcs[idx].name);
                            cont += L"가 죽었다.";
                            i.send_chat(s.c_str(), cont.c_str());
                        }
                    }
                }
            }
            break;
        }
        }
        break;
    }
    case CS_TELEPORT: // ok
    {
        // 맵의 랜덤한 위치로 텔레포트
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis_x(0, W_WIDTH - 1);
        std::uniform_int_distribution<> dis_y(0, W_HEIGHT - 1);

        while (true)
        {
            int new_x = dis_x(gen);
            int new_y = dis_y(gen);

            if (can_move(new_x, new_y))
            {
                x = new_x;
                y = new_y;
                break;
            }
        }

        if (x / SECTOR_SIZE != sector_x || y / SECTOR_SIZE != sector_y)
        {
            std::lock_guard<std::mutex> ll{ g_SectorLock };
            g_SectorList[sector_x][sector_y].erase(id);
            sector_x = x / SECTOR_SIZE;
            sector_y = y / SECTOR_SIZE;
            g_SectorList[sector_x][sector_y].insert(id);
        }

        vl_l.lock();
        std::unordered_set<int> old_viewlist = view_list;
        vl_l.unlock();
        std::unordered_set<int> new_viewlist;

        for (const auto& n : Nears) {
            short sx = sector_x + n.x;
            short sy = sector_y + n.y;

            if (sx >= 0 && sx <= W_WIDTH / SECTOR_SIZE && sy >= 0 && sy <= W_HEIGHT / SECTOR_SIZE) {
                std::lock_guard<std::mutex> sector_ll(g_SectorLock);
                for (auto& i : g_SectorList[sx][sy]) {
                    if (!isNear(i)) continue;
                    if (i == id) continue;

                    if (i < 0) {
                        int a = i * (-1) - 1;
                        if (npcs[a].hp <= 0) continue;
                    }
                    else if (players[abs(i)].get_state() != PLAYING) {
                        continue;
                    }
                    new_viewlist.insert(i);
                }
            }
        }

        send_move_object(x, y, id, last_move_time);

        for (auto& i : new_viewlist) {
            if (old_viewlist.count(i) == 0) { // 새로 들어온 객체
                if (i < 0) {
                    int k = -1 * i - 1;
                    send_add_object(npcs[k].x, npcs[k].y, npcs[k].name, npcs[k].id, npcs[k].visual);
                    send_stat_change(0, npcs[k].hp, npcs[k].level, npcs[k].max_hp, npcs[k].id);
                    if (npcs[k].active) continue;
                    bool expt = false;
                    if (true == atomic_compare_exchange_strong(&npcs[k].active, &expt, true))
                        push_evt_queue(k, k, TASK_TYPE::EV_RANDOM_MOVE, 1000);
                }
                else {
                    players[i].send_add_object(x, y, name, id, visual);
                    players[i].send_stat_change(exp, hp, level, max_hp, id);
                    send_add_object(players[i].x, players[i].y, players[i].name, players[i].id, players[i].visual);
                    send_stat_change(players[i].exp, players[i].hp, players[i].level, players[i].max_hp, players[i].id);
                }
            }
            else { // 기존에 있던 객체
                if (i >= 0) {
                    players[i].send_move_object(x, y, id, last_move_time);
                }
            }
        }

        for (auto& i : old_viewlist) {
            if (new_viewlist.count(i) == 0) { // 멀어진 객체
                if (i < 0) {
                    send_remove_object(i);
                }
                else {
                    send_remove_object(i);
                    players[i].send_remove_object(id);
                }
            }
        }

        view_list = new_viewlist;
        break;
    }
    case CS_LOGOUT:
    {
        std::cout << "CS_LOGOUT" << std::endl;
        {
            std::lock_guard<std::mutex> lock(st_lock);
            state = NONE;
        }
        break;
    }
    default:
        break;
    }
}

void Player::send_login_fail()
{
	std::cout << "SC_LOGIN_FAIL_PACKET" << std::endl;
	SC_LOGIN_FAIL_PACKET packet;
	packet.size = sizeof(SC_LOGIN_FAIL_PACKET);
	packet.type = SC_LOGIN_FAIL;
	send(&packet);
}

void Player::send_login_info()
{
	std::cout << "SC_LOGIN_INFO_PACKET" << std::endl;
	SC_LOGIN_INFO_PACKET packet;
	packet.id = id;
	packet.size = sizeof(SC_LOGIN_INFO_PACKET);
	packet.type = SC_LOGIN_INFO;
	packet.hp = hp;
	packet.max_hp = hp;
	packet.exp = exp;
	packet.level = level;
	packet.visual = 0;
	packet.x = x;
	packet.y = y;
	send(&packet);
}

void Player::send_add_object(int ox, int oy, const char* oname, int oid, int ov)
{
	std::cout << "SC_ADD_OBJECT_PACKET" << std::endl;
	SC_ADD_OBJECT_PACKET packet;
	packet.size = sizeof(SC_ADD_OBJECT_PACKET);
	packet.type = SC_ADD_OBJECT;
	packet.x = ox;
	packet.y = oy;
	strncpy_s(packet.name, oname, NAME_SIZE - 1);
	packet.name[NAME_SIZE - 1] = '\0'; // Ensure null-termination
	packet.id = oid;
	packet.visual = ov;
	send(&packet);
}

void Player::send_remove_object(int oid)
{
	std::cout << "SC_REMOVE_OBJECT_PACKET" << std::endl;
	SC_REMOVE_OBJECT_PACKET packet;
	packet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
	packet.type = SC_REMOVE_OBJECT;
	packet.id = oid;
	send(&packet);
}

void Player::send_move_object(int ox, int oy, int oid, unsigned int lmt)
{
	std::cout << "SC_MOVE_OBJECT_PACKET" << std::endl;
	SC_MOVE_OBJECT_PACKET packet;
	packet.size = sizeof(SC_MOVE_OBJECT_PACKET);
	packet.type = SC_MOVE_OBJECT;
	packet.id = oid;
	packet.move_time = lmt;
	packet.x = ox;
	packet.y = oy;
	send(&packet);
}

void Player::send_chat(const char* name, const wchar_t* msg)
{
	std::wcout << L"SC_CHAT_PACKET" << std::endl;
	SC_CHAT_PACKET packet;
	size_t message_len = wcsnlen(msg, CHAT_SIZE - 1);

	packet.size = 23 + message_len * sizeof(wchar_t);
	packet.type = SC_CHAT;

	strncpy_s(packet.name, name, NAME_SIZE - 1);
	packet.name[NAME_SIZE - 1] = '\0'; // Ensure null-termination
	wmemcpy(packet.mess, msg, message_len);
	packet.mess[message_len] = L'\0'; // Ensure null-termination

	send(&packet);
}

void Player::send_stat_change(int oe, int oh, int ol, int omh, int oid)
{
	std::cout << "SC_STAT_CHANGE_PACKET" << std::endl;
	SC_STAT_CHANGE_PACKET packet;
	packet.size = sizeof(SC_STAT_CHANGE_PACKET);
	packet.type = SC_STAT_CHANGE;
	packet.exp = oe;
	packet.hp = oh;
	packet.level = ol;
	packet.max_hp = omh;
	packet.id = oid;
	send(&packet);
}