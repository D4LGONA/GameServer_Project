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
		rc.right = 550; // x 좌표에 50을 더함
		rc.bottom = 550; // y 좌표에 50을 더함
		TransparentBlt(dc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, img->img.GetDC(), 0, 0, img->img.GetWidth(), img->img.GetHeight(), MAGENTA);
	
		// 이름 렌더링
		SetBkMode(dc, TRANSPARENT); // 투명 배경 모드로 설정
		SetTextColor(dc, RGB(0, 0, 0)); // 텍스트 색상 설정 (흰색)

		// 이름 텍스트의 위치 계산 (캐릭터 이미지의 머리 위)
		int textWidth = (rc.right - rc.left) / 2;
		int textX = rc.left + textWidth - (name.length() * 3); // 간격 조정
		int textY = rc.top - 15; // 캐릭터 이미지 머리 위에 15 픽셀 띄움

		wstring ws = strtowstr(name);
		TextOut(dc, textX, textY, ws.c_str(), name.length());
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

private:
};