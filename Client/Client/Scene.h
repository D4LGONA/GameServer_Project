#pragma once
#include "stdafx.h"
#ifndef _Scene
#define _Scene

class Scene
{
protected:
	static int selected;
	static bool isChanged;

public:

	Scene() {};
	~Scene() {};

	static void setSelected(int value) { selected = value; }
	static int getSelected() { return selected; }

	static void setIsChanged(bool b) { isChanged = b; }
	static bool getIsChanged() { return isChanged; }

	virtual void render(HDC dc, HWND hwnd) const = 0;
	virtual void update() = 0;
	virtual void reset() = 0;
	virtual void keydown(WPARAM wparam) = 0;
	virtual void keyup(WPARAM wparam) = 0;
	virtual void LbuttonDown(int x, int y) = 0;
};
#endif
