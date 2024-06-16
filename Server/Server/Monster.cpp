#include "stdafx.h"
#include "Monster.h"

void Monster::random_move()
{
    std::random_device rd;  // 난수 생성기
    std::mt19937 gen(rd()); // Mersenne Twister 엔진을 사용한 난수 생성기
    std::uniform_int_distribution<> dis(0, 3); // 0부터 3까지 균일 분포

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
        if (can_move(x + 1, y))
            ++x;
        break;
    case 1:
        if (can_move(x - 1, y))
            --x;
        break;
    case 2:
        if (can_move(x, y + 1))
            ++y;
        break;
    case 3:
        if (can_move(x, y - 1))
            --y;
        break;
    }

    if (x / SECTOR_SIZE != sector_x || y / SECTOR_SIZE != sector_y)
    {
        lock_guard<mutex> ll{ g_SectorLock }; // 섹터 이동
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
