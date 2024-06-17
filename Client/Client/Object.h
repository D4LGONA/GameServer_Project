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
		// todo: 이름 hp 레벨 띄우기
		RECT rc;
		rc.left = (x - curx) * 50;
		rc.top = (y - cury) * 50;
		rc.right = (x + 1 - curx) * 50; // x 좌표에 50을 더함
		rc.bottom = (y + 1 - cury) * 50; // y 좌표에 50을 더함
		TransparentBlt(dc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, img->img.GetDC(), 0, 0, img->img.GetWidth(), img->img.GetHeight(), MAGENTA);

		// 이름 및 HP 렌더링
		SetBkMode(dc, TRANSPARENT); // 투명 배경 모드로 설정
		SetTextColor(dc, RGB(0, 0, 0)); // 텍스트 색상 설정 (검은색)

		// 이름 텍스트의 위치 계산 (캐릭터 이미지의 머리 위)
		int textWidth = (rc.right - rc.left) / 2;
		int textX = rc.left + textWidth - (name.length() * 3); // 간격 조정
		int textY = rc.top - 30; // 캐릭터 이미지 머리 위에 30 픽셀 띄움

		// 이름 렌더링
		wstring ws = strtowstr(name);
		TextOut(dc, textX, textY, ws.c_str(), ws.length());

		// HP 텍스트의 위치 계산
		textY += 15; // 이름 아래에 15 픽셀 띄움

		// HP 정보 렌더링
		wstring hp_info = L"HP: " + to_wstring(hp);
		TextOut(dc, textX, textY, hp_info.c_str(), hp_info.length());
	}

	void setup(int x, int y, int exp, int hp, int level, int visual, int max_hp, int id, string name)
	{
		this->x = x;
		this->y = y;
		this->exp = exp;
		this->hp = hp;
		this->level = level;
		this->visual = visual;
		this->maxhp = max_hp;
		this->id = id;
		this->name = name;
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
	string name;
protected:
	int maxhp;
	int x, y;
	int hp;
	int level;
	int exp;
};