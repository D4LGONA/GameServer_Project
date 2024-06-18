#pragma once

// 상속의 기본형 
class Object
{
protected:
	
	int visual;
	int attack, defense; // 공격력, 방어력

public:
	char name[NAME_SIZE]; // 이름
	atomic_bool ishealing = false;
	int level;
	short sector_x, sector_y;
	int max_hp; // 최대체력
	int id;
	short x, y;
	int hp; // 체력

	Object() {}
	~Object() {}

	bool do_healing();
	short getX() { return x; }
	short getY() { return y; }
	int getVis() { return visual; }
	bool isNear(int other_id);
	bool isNear(int other_id, int distance);
	double distance(int other_id);
	int getSecX() { return sector_x; }
	int getSecY() { return sector_y; }
};

