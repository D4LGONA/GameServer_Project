#pragma once

// ����� �⺻�� 
class Object
{
protected:
	
	int max_hp; // �ִ�ü��
	int level;
	int visual;
	int attack, defense; // ���ݷ�, ����
	char name[NAME_SIZE]; // �̸�
	short sector_x, sector_y;
	chrono::time_point<chrono::high_resolution_clock> last_heal_time = chrono::high_resolution_clock::now();

public:
	int id;

	short x, y;
	int hp; // ü��

	Object() {}
	~Object() {}

	short getX() { return x; }
	short getY() { return y; }
	int getVis() { return visual; }
	bool isNear(int other_id);
	bool isNear(int other_id, int distance);
	double distance(int other_id);
	int getSecX() { return sector_x; }
	int getSecY() { return sector_y; }
};

