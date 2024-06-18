#pragma once
#include "stdafx.h"
#include "Object.h"

enum MonsterType { Peace, Follow, Agro };
enum MovementType { Fixed, Roaming };

class Monster : public Object
{
public:
    friend class Player;

    Monster() {};
    ~Monster() {};

    void setup(int id, MonsterType type, MovementType movement, int level, int hp, int x, int y, int visual);
    void follow_move();
    void takeDamage(int damage);
    void respawn();
    int getExperience() const;

    void setState(bool b) { active = b; }
    bool isAlive() const { if (hp > 0) return true; return false; }

    void randomMove();
    void doing_Ai(int target);
    bool isOrigin();
    

    std::atomic_int target_id = -1;
    MonsterType type;
    MovementType movement;
private:
    std::atomic_bool active = false;
    POINT LT, RB;
    POINT origin;
    std::chrono::time_point<std::chrono::steady_clock> lastAtkTime;
};
