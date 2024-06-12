#pragma once
#include "stdafx.h"

class Object
{
public:
	Object()
		: x{ 10 }, y{ 10 }, hp{ 100 }, level{ 1 }, exp{ 0 }
	{}
	~Object() {};


	void render(HDC dc, Image*& img)
	{
		RECT rc;
		rc.left = x * 50;
		rc.top = y * 50;
		rc.right = (x + 1) * 50; // x 좌표에 50을 더함
		rc.bottom = (y + 1) * 50; // y 좌표에 50을 더함
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

	int Gethp() { return hp; }
	float Maxhp() { return float(maxhp); }

	int visual;
protected:
	int maxhp;
	int x, y;
	int hp;
	int level;
	int exp;
};