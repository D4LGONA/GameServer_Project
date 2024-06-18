#pragma once

// ����� �⺻�� 
class Object
{
protected:
	
	int visual;
	int attack, defense; // ���ݷ�, ����

public:
	char name[NAME_SIZE]; // �̸�
	atomic_bool ishealing = false;
	int level;
	short sector_x, sector_y;
	int max_hp; // �ִ�ü��
	int id;
	short x, y;
	int hp; // ü��

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

