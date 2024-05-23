#pragma once
#include "stdafx.h"
#include "Scene.h"
#include "Tile.h"
#include "Player.h"

class PlayScene : public Scene
{
public:
	PlayScene()
	{
		pl = new Player();
		map = new Tilemap();
		map->load("tilemap_test.txt");
	}

	~PlayScene() { delete pl; delete map; }

	void render(HDC dc, HWND hwnd) const override
	{
		map->render(dc, curx, cury);
		pl->render(dc, curx, cury);
	}

	void update() override {
	}

	void keydown(WPARAM wparam) override
	{

	}

	void keyup(WPARAM wparam) override
	{

	}

	void LbuttonDown(int x, int y) override {}

	void reset() override
	{

	}

private:
	Player* pl;
	int curx = 0, cury = 0;
	Tilemap* map;
};