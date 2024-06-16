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
	
		// �̸� ������
		SetBkMode(dc, TRANSPARENT); // ���� ��� ���� ����
		SetTextColor(dc, RGB(0, 0, 0)); // �ؽ�Ʈ ���� ���� (���)

		// �̸� �ؽ�Ʈ�� ��ġ ��� (ĳ���� �̹����� �Ӹ� ��)
		int textWidth = (rc.right - rc.left) / 2;
		int textX = rc.left + textWidth - (name.length() * 3); // ���� ����
		int textY = rc.top - 15; // ĳ���� �̹��� �Ӹ� ���� 15 �ȼ� ���

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