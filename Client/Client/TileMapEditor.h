#pragma once
#include "Scene.h"
#include "Tile.h"

// TileMapScene 클래스 정의
class TileMapScene : public Scene 
{
public:
    TileMapScene() { map = new Tilemap(); };

    ~TileMapScene() { delete map; }

    void render(HDC& dc, HWND& hwnd) override 
    {
        map->render(dc, curx, cury);
    }

    void update() override {
    }

    void keydown(WPARAM wparam) override;

    void keyup(WPARAM wparam) override
    {

    }

    void LbuttonDown(int x, int y) override;
    
    void reset() override
    {

    }

private:
    int curx = 0, cury = 0;
    Tilemap* map;
    int curtype = GRASS;
    bool curcan_move = true;
    int curcanmove_dir = CANT;

};