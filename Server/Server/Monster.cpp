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
    switch (type)
    {
    case Peace:
        LT = { x - 10, y - 10 };
        RB = { x + 10, y + 10 };
        break;
    case Follow:
        LT = { x - 10, y - 10 };
        RB = { x + 10, y + 10 };
        break;
    case Agro:
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

void Monster::follow_move(int targetX, int targetY)
{
    POINT pt;
    if (distance(target_id) < 25) {
        target_id = -1;
        pt = a_star_find_next_move({ x, y }, origin);
    }
    else
    {
        pt = a_star_find_next_move({ x, y }, { targetX, targetY });
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
            players[pl].send_remove_object(id);
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
                if (type != Peace and isNear(i, 5) and distance(i) < near_dist) { // 쫓아갈 목표 확인.
                    target_id = i;
                }
                int target_x = players[i].x;
                int target_y = players[i].y;

                bool is_adjacent =
                    (target_x == x && (target_y == y - 1 || target_y == y + 1)) || // 상하
                    (target_y == y && (target_x == x - 1 || target_x == x + 1)); // 좌우

                if (is_adjacent)
                {
                    players[i].hp -= level * 10;
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
            players[pl].send_remove_object(id);
    }

}

void Monster::doing_Ai(int target)
{
    if (movement == Roaming)
        push_evt_queue(id, id, TASK_TYPE::EV_RANDOM_MOVE, 1000);
}
