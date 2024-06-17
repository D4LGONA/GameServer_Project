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
		// todo: �̸� hp ���� ����
		RECT rc;
		rc.left = (x - curx) * 50;
		rc.top = (y - cury) * 50;
		rc.right = (x + 1 - curx) * 50; // x ��ǥ�� 50�� ����
		rc.bottom = (y + 1 - cury) * 50; // y ��ǥ�� 50�� ����
		TransparentBlt(dc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, img->img.GetDC(), 0, 0, img->img.GetWidth(), img->img.GetHeight(), MAGENTA);

		// �̸� �� HP ������
		SetBkMode(dc, TRANSPARENT); // ���� ��� ���� ����
		SetTextColor(dc, RGB(0, 0, 0)); // �ؽ�Ʈ ���� ���� (������)

		// �̸� �ؽ�Ʈ�� ��ġ ��� (ĳ���� �̹����� �Ӹ� ��)
		int textWidth = (rc.right - rc.left) / 2;
		int textX = rc.left + textWidth - (name.length() * 3); // ���� ����
		int textY = rc.top - 30; // ĳ���� �̹��� �Ӹ� ���� 30 �ȼ� ���

		// �̸� ������
		wstring ws = strtowstr(name);
		TextOut(dc, textX, textY, ws.c_str(), ws.length());

		// HP �ؽ�Ʈ�� ��ġ ���
		textY += 15; // �̸� �Ʒ��� 15 �ȼ� ���

		// HP ���� ������
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