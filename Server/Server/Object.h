#pragma once
#include "stdafx.h"

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

public:
	Object() {}
	~Object() {}

	short getX() { return x; }
	short getY() { return y; }
	int getVis() { return visual; }
};

