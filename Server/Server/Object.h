#pragma once
#include "stdafx.h"

// ����� �⺻�� 
class Object
{
protected:
	int id;
	int x, y;
	int hp; // ü��
	int max_hp; // �ִ�ü��
	int level;
	int attack, defense; // ���ݷ�, ����
	char name[NAME_SIZE]; // �̸�

public:
	Object() {}
	~Object() {}
};

