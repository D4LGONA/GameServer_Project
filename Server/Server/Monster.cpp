#include "stdafx.h"

void Monster::setup(int id, MonsterType type, MovementType movement, int level, int hp, int x, int y, int visual)
{
    this->id = -1 * id;
    this->x = x;
    this->y = y;
    this->hp = hp;
    this->max_hp = hp;
    this->level = level;
    this->type = type;
    switch (movement)
    {
    case Roaming:
        LT = { x - 10, y - 10 };
        RB = { x + 10, y + 10 };
        break;
    }
    this->visual = visual;
    origin = { x, y };
    this->movement = movement;
    string strname = "Monster: " + to_string((this->id * -1));
    memcpy(name, strname.c_str(), strname.size());
    this->sector_x = x / SECTOR_SIZE;
    this->sector_y = y / SECTOR_SIZE;
    {
        std::lock_guard<std::mutex> ll{ g_SectorLock };
        g_SectorList[sector_x][sector_y].insert(this->id);
    }
    active = false;
    target_id = -1;
}

void Monster::follow_move()
{
    POINT pt;
    if (target_id == -1 or distance(target_id) > 25) {
        target_id = -1;
        pt = a_star_find_next_move({ x, y }, origin);
    }
    else
    {
        pt = a_star_find_next_move({ x, y }, { players[target_id].x, players[target_id].y });
    }
     
    unordered_set<int> old_vl;
    for (const auto& n : Nears) {
        short sx = sector_x + n.x;
        short sy = sector_y + n.y;

        if (sx >= 0 && sx <= W_WIDTH / SECTOR_SIZE && sy >= 0 && sy <= W_HEIGHT / SECTOR_SIZE) {
            lock_guard<mutex> ll{ g_SectorLock };
            for (auto& i : g_SectorList[sx][sy]) {
                if (!isNear(i)) continue;
                if (i == id) continue;
                if (i < 0) continue;
                if (players[i].state != PLAYING) continue;
                old_vl.insert(i);
            }
        }
    }

    x = pt.x;
    y = pt.y;

    if (x / SECTOR_SIZE != sector_x || y / SECTOR_SIZE != sector_y)
    {
        lock_guard<mutex> ll{ g_SectorLock };
        g_SectorList[sector_x][sector_y].erase(id);
        sector_x = x / SECTOR_SIZE;
        sector_y = y / SECTOR_SIZE;
        g_SectorList[sector_x][sector_y].insert(id);
    }

    unordered_set<int> new_vl;
    for (const auto& n : Nears) {
        short sx = sector_x + n.x;
        short sy = sector_y + n.y;

        if (sx >= 0 && sx <= W_WIDTH / SECTOR_SIZE && sy >= 0 && sy <= W_HEIGHT / SECTOR_SIZE) {
            lock_guard<mutex> ll{ g_SectorLock };
            for (auto& i : g_SectorList[sx][sy]) {
                if (!isNear(i)) continue;
                if (i == id) continue;
                if (i < 0) continue;
                if (players[i].state != PLAYING) continue;

                int target_x = players[i].x;
                int target_y = players[i].y;

                bool is_adjacent =
                    (target_x == x && (target_y == y - 1 || target_y == y + 1)) || // 상하
                    (target_y == y && (target_x == x - 1 || target_x == x + 1)); // 좌우

                if (!players[i].ishealing)
                {
                    push_evt_queue(i, i, TASK_TYPE::EV_HEAL, 5000); // hp가 깎이면 5초 후에 heal
                    players[i].ishealing = true;
                }

                if (is_adjacent || (target_x == x && target_y == y))
                {
                    players[i].hp -= level * 10;
                    players[i].send_stat_change(players[i].exp, players[i].hp, players[i].level, players[i].max_hp, i);
                    std::string s = "system";
                    std::wstring cont = L""; // char[] name을 std::wstring으로 변환
                    cont += strtowstr(name) + L"에게" + to_wstring(level * 10) + L"의 데미지를 입었다.";
                    players[i].send_chat(s.c_str(), cont.c_str());
                }

                new_vl.insert(i);
            }
        }
    }

    for (auto& pl : new_vl) {
        if (0 == old_vl.count(pl))
            players[pl].send_add_object(x, y, name, id, visual);
        else
            players[pl].send_move_object(x, y, id, 0);
    }

    for (auto& pl : old_vl) {
        if (0 == new_vl.count(pl))
        {
            players[pl].send_remove_object(id);
            {
                lock_guard<mutex> ll{ players[pl].vl_l };
                players[pl].view_list.erase(id);
            }
        }
    }
}

void Monster::takeDamage(int atk)
{
}

void Monster::respawn() // ok
{
    hp = max_hp;
    x = origin.x;
    y = origin.y;
    target_id = -1;

    // 섹터에 다시 추가
    {
        std::lock_guard<std::mutex> ll{ g_SectorLock };
        sector_x = x / SECTOR_SIZE;
        sector_y = y / SECTOR_SIZE;
        g_SectorList[sector_x][sector_y].insert(id);
    }

    // 주변 플레이어들에게 add object 및 stat change 패킷 브로드캐스팅
    for (const auto& n : Nears) {
        short sx = sector_x + n.x;
        short sy = sector_y + n.y;

        if (sx >= 0 && sx <= W_WIDTH / SECTOR_SIZE && sy >= 0 && sy <= W_HEIGHT / SECTOR_SIZE) {
            std::lock_guard<std::mutex> sector_ll(g_SectorLock);
            for (auto& i : g_SectorList[sx][sy]) {
                if (!isNear(i)) continue;
                if (i < 0) continue;
                if (players[i].get_state() != PLAYING) continue;
                players[i].send_add_object(x, y, name, id, visual);
                players[i].send_stat_change(0, hp, level, max_hp, id);
            }
        }
    }
}

int Monster::getExperience() const
{
    int baseExp = level * level * 2;
    if (type == Agro)
    {
        baseExp *= 2;
    }
    if (movement == Roaming)
    {
        baseExp *= 2;
    }
    return baseExp;
}

void Monster::randomMove() // ok
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 3);

    unordered_set<int> old_vl;
    for (const auto& n : Nears) {
        short sx = sector_x + n.x;
        short sy = sector_y + n.y;

        if (sx >= 0 && sx <= W_WIDTH / SECTOR_SIZE && sy >= 0 && sy <= W_HEIGHT / SECTOR_SIZE) {
            lock_guard<mutex> ll{ g_SectorLock };
            for (auto& i : g_SectorList[sx][sy]) {
                if (!isNear(i)) continue;
                if (i == id) continue;
                if (i < 0) continue;
                if (players[i].state != PLAYING) continue;
                old_vl.insert(i);
            }
        }
    }

    switch (dis(gen))
    {
    case 0:
        if (can_move(x + 1, y) and (x + 1 <= RB.x and x + 1 >= LT.x) and (y >= LT.y and y <= RB.y))
            ++x;
        break;
    case 1:
        if (can_move(x - 1, y) and (x - 1 <= RB.x and x - 1 >= LT.x) and (y >= LT.y and y <= RB.y))
            --x;
        break;
    case 2:
        if (can_move(x, y + 1) and (x <= RB.x and x >= LT.x) and (y + 1 >= LT.y and y + 1 <= RB.y))
            ++y;
        break;
    case 3:
        if (can_move(x, y - 1) and (x <= RB.x and x >= LT.x) and (y - 1 >= LT.y and y - 1 <= RB.y))
            --y;
        break;
    }

    if (x / SECTOR_SIZE != sector_x || y / SECTOR_SIZE != sector_y)
    {
        lock_guard<mutex> ll{ g_SectorLock };
        g_SectorList[sector_x][sector_y].erase(id);
        sector_x = x / SECTOR_SIZE;
        sector_y = y / SECTOR_SIZE;
        g_SectorList[sector_x][sector_y].insert(id);
    }

    float near_dist = FLT_MAX;
    unordered_set<int> new_vl;
    for (const auto& n : Nears) {
        short sx = sector_x + n.x;
        short sy = sector_y + n.y;

        if (sx >= 0 && sx <= W_WIDTH / SECTOR_SIZE && sy >= 0 && sy <= W_HEIGHT / SECTOR_SIZE) {
            lock_guard<mutex> ll{ g_SectorLock };
            for (auto& i : g_SectorList[sx][sy]) {
                if (!isNear(i)) continue;
                if (i == id) continue;
                if (i < 0) continue;
                if (players[i].state != PLAYING) continue;
                if (target_id == -1 and type == Agro and isNear(i, 5) and distance(i) < near_dist) { // 쫓아갈 목표 확인.
                    target_id = i;
                }

                int target_x = players[i].x;
                int target_y = players[i].y;

                bool is_adjacent =
                    (target_x == x && (target_y == y - 1 || target_y == y + 1)) || // 상하
                    (target_y == y && (target_x == x - 1 || target_x == x + 1)); // 좌우

                if (!players[i].ishealing)
                {
                    push_evt_queue(i, i, TASK_TYPE::EV_HEAL, 5000); // hp가 깎이면 5초 후에 heal
                    players[i].ishealing = true;
                }

                if (is_adjacent || (target_x == x && target_y == y))
                {
                    players[i].hp -= level * 10;
                    players[i].send_stat_change(players[i].exp, players[i].hp, players[i].level, players[i].max_hp, i);
                    std::string s = "system";
                    std::wstring cont = L""; // char[] name을 std::wstring으로 변환
                    cont += strtowstr(name) + L"에게" + to_wstring(level * 10) + L"의 데미지를 입었다.";
                    players[i].send_chat(s.c_str(), cont.c_str());
                }

                if (players[i].hp <= 0) // 플레이어가 죽었을 때
                {
                    std::wstring death_message = strtowstr(players[i].name) + L"가 죽었다.";
                    std::string s = "system";
                    for (auto& p : players)
                    {
                        if (p.get_state() == PLAYING)
                        {
                            p.send_chat(s.c_str(), death_message.c_str());
                            p.send_remove_object(i);
                        }
                    }

                    // 10초 후에 respawn 이벤트 추가
                    push_evt_queue(i, i, TASK_TYPE::EV_RESPAWN, 10000); // 10초 후 respawn

                    // 주변 플레이어에게 remove object 패킷 보내기
                    for (const auto& n : Nears) {
                        short sx = sector_x + n.x;
                        short sy = sector_y + n.y;

                        if (sx >= 0 && sx <= W_WIDTH / SECTOR_SIZE && sy >= 0 && sy <= W_HEIGHT / SECTOR_SIZE) {
                            for (auto& id : g_SectorList[sx][sy]) {
                                if (!isNear(id)) continue;
                                if (id == i) continue;
                                if (id < 0) continue;
                                if (players[id].get_state() != PLAYING) continue;
                                players[id].send_remove_object(i);
                            }
                        }
                    }

                    players[i].state = NONE; // 플레이어 상태를 NONE으로 설정
                }

                new_vl.insert(i);
            }
        }
    }

    for (auto& pl : new_vl) {
        if (0 == old_vl.count(pl))
            players[pl].send_add_object(x, y, name, id, visual);
        else
            players[pl].send_move_object(x, y, id, 0);
    }

    for (auto& pl : old_vl) {
        if (0 == new_vl.count(pl))
        {
            players[pl].send_remove_object(id);
            {
                lock_guard<mutex> ll{ players[pl].vl_l };
                players[pl].view_list.erase(id);
            }
        }
    }
}

bool Monster::isOrigin()
{
    if (x == origin.x and y == origin.y) {
        hp = max_hp;
        return true;
    }
    return false;
}

void Monster::doing_Ai(int target)
{
    if (movement == Roaming)
    {
         push_evt_queue(id, id, TASK_TYPE::EV_RANDOM_MOVE, 1000);
    }
    else
         push_evt_queue(target, id, TASK_TYPE::EV_FOLLOW_MOVE, 1000);

}
