#pragma once

// ����� �⺻�� 
class Object
{
protected:
	int id;
	short x, y;
	int hp; // ü��
	int max_hp; // �ִ�ü��
	int level;
	int visual;
	int attack, defense; // ���ݷ�, ����
	char name[NAME_SIZE]; // �̸�
	short sector_x, sector_y;

public:
	Object() {}
	~Object() {}

	short getX() { return x; }
	short getY() { return y; }
	int getVis() { return visual; }
	bool isNear(int other_id);
	
};

