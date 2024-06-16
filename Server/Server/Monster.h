#pragma once
#include "stdafx.h"

class Monster : public Object
{
public:
	friend class Player;

	Monster() {};
	~Monster() {};

	void setup(int id)
	{
		// 가상머신을 이용해야 하는데 ..
		//active = true;
		this->id =  -1 * id;
		x = rand() % 20, y = rand() % 20;
		hp = 200; // 체력
		max_hp = 200; // 최대체력
		level = 10;
		visual = rand() % 6 + 1;
		attack = 50, defense = 50; // 공격력, 방어력
		string strname = "Monster:" + to_string((this->id * -1));
		memcpy(name, strname.c_str(), strname.size());
		sector_x = x / SECTOR_SIZE;
		sector_y = y / SECTOR_SIZE;
		{
			lock_guard<mutex> ll{ g_SectorLock };
			g_SectorList[sector_x][sector_y].insert(this->id);
		}
	}

	void setState(bool b) { active = b; }

	void random_move();
    

	void ai_move()
	{


		
	}

	void follow_ai()
	{
	}

private:
	int target_id;
	bool active = false;
};
