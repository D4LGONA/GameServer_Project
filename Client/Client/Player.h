#pragma once
#include "stdafx.h"

class Player : public Object
{
public:
	Player() { }

	void render(HDC dc, Image* img)
	{
		RECT rc;
		rc.left = 500;
		rc.top = 500;
		rc.right = 550; // x ��ǥ�� 50�� ����
		rc.bottom = 550; // y ��ǥ�� 50�� ����
		TransparentBlt(dc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, img->img.GetDC(), 0, 0, img->img.GetWidth(), img->img.GetHeight(), MAGENTA);
	}

	void setup(int x, int y, int exp, int hp, int level, int visual, int max_hp)
	{
		this->x = x;
		this->y = y;
		this->exp = exp;
		this->hp = hp;
		this->level = level;
		this->visual = visual;
		this->maxhp = max_hp;
	}

	void move(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	~Player() {}

	int Gethp() { return hp; }
	float Maxhp() { return float(maxhp); }

	string name;
private:
};