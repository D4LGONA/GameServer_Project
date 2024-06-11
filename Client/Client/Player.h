#pragma once
#include "stdafx.h"

class Player
{
public:
	Player()
		: x{ 10 }, y{ 10 }, hp{ 100 }, level{ 1 }, exp{ 0 } 
	{
		img = new Image();
		img->img.Load(TEXT("resources/w_p.png"));
	}

	void render(HDC dc, int padx, int pady)
	{
		RECT rc;
		rc.left = (x - padx) * 50;
		rc.top = (y - pady) * 50;
		rc.right = (x + 1 - padx) * 50; // x 좌표에 50을 더함
		rc.bottom = (y + 1 - pady) * 50; // y 좌표에 50을 더함
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

	~Player();

	string name;
private:
	int maxhp, visual;

	int x, y;
	Image* img;
	int hp;
	int level;
	int exp;
};