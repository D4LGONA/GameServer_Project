#pragma once

// 상속의 기본형 
class Object
{
protected:
	int id;
	short x, y;
	int hp; // 체력
	int max_hp; // 최대체력
	int level;
	int visual;
	int attack, defense; // 공격력, 방어력
	char name[NAME_SIZE]; // 이름
	short sector_x, sector_y;

public:
	Object() {}
	~Object() {}

	short getX() { return x; }
	short getY() { return y; }
	int getVis() { return visual; }
	bool isNear(int other_id);
	
};

