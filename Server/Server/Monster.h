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
		// ����ӽ��� �̿��ؾ� �ϴµ� ..
		active = true;
		this->id =  -1 * id;
		x = rand() % 20, y = rand() % 20;
		hp = 200; // ü��
		max_hp = 200; // �ִ�ü��
		level = 10;
		visual = 3;
		attack = 50, defense = 50; // ���ݷ�, ����
		string strname = "Monster:" + to_string(this->id);
		memcpy(name, strname.c_str(), strname.size());
		sector_x = x / SECTOR_SIZE;
		sector_y = y / SECTOR_SIZE;
		{
			lock_guard<mutex> ll{ g_SectorLock };
			g_SectorList[sector_x][sector_y].insert(this->id);
		}
	}

	void ai_move()
	{
		// �̵��� ó���ϴ� ��
	}

private:
	int target_id;
	bool active = false;
};
