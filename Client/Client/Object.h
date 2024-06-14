#pragma once
#include "stdafx.h"

class Object
{
public:
	Object()
		: x{ 10 }, y{ 10 }, hp{ 100 }, level{ 1 }, exp{ 0 }
	{}
	~Object() {};


	void render(HDC dc, Image*& img, int curx, int cury)
	{
		RECT rc;
		rc.left = (x - curx) * 50;
		rc.top = (y - cury) * 50;
		rc.right = (x + 1 - curx) * 50; // x 좌표에 50을 더함
		rc.bottom = (y + 1 - cury) * 50; // y 좌표에 50을 더함
		TransparentBlt(dc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, img->img.GetDC(), 0, 0, img->img.GetWidth(), img->img.GetHeight(), MAGENTA);
	}

	void setup(int x, int y, int exp, int hp, int level, int visual, int max_hp, int id)
	{
		this->x = x;
		this->y = y;
		this->exp = exp;
		this->hp = hp;
		this->level = level;
		this->visual = visual;
		this->maxhp = max_hp;
		this->id = id;
	}

	void move(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	int Gethp() { return hp; }
	float Maxhp() { return float(maxhp); }

	int visual;
	int id;
protected:
	int maxhp;
	int x, y;
	int hp;
	int level;
	int exp;
};