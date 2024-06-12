#pragma once
#include "stdafx.h"
#include "Scene.h"
#include "Tile.h"
#include "Object.h"
#include "Player.h"

class PlayScene : public Scene
{
public:
    PlayScene(SOCKET s, const char* username)
        : g_socket{ s }
    {
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/w_p.png"));

        pl = new Player();
        SendLoginPacket(username);
        ReceiveFromServer();
        map = new Tilemap();
        map->load("tilemap_test.txt");
        UIsetup();
    }

    ~PlayScene() {
        delete pl; delete map;
        for (auto& a : imgs)
            delete a;
        imgs.clear();
        for (auto& a : objs)
            delete a;
        objs.clear();
        for (auto& a : ui_imgs)
            delete a;
        ui_imgs.clear();
    }

    void render(HDC dc, HWND hwnd) const override
    {
        if (imgs.size() == 0) return;
        map->render(dc, curx, cury);
        switch (pl->visual)
        {
        case 0:
            pl->render(dc, imgs[0]);
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        }
        UIrender(dc);
    }

    void update() override {
        ReceiveFromServer();
    }

    void keydown(WPARAM wparam) override;

    void keyup(WPARAM wparam) override {}

    void LbuttonDown(int x, int y) override {}

    void reset() override {}

    bool canMove(POINT pt)
    {
        if (map->getmap()[pt.x][pt.y].can_move)
            return true;
        return false;
    }

    void UIsetup() 
    {
        ui_imgs.push_back(new Image());
        ui_imgs.back()->img.Load(TEXT("resources/hpui.png"));
        ui_imgs.push_back(new Image());
        ui_imgs.back()->img.Load(TEXT("resources/hpbar.png"));
        ui_imgs.push_back(new Image());
        ui_imgs.back()->img.Load(TEXT("resources/p_profile.png"));
    }
    void UIrender(HDC& dc) const
    {
        RECT hpbar_rc;
        hpbar_rc.left = 105;
        hpbar_rc.right = min(max(105, 105 + (pl->Gethp() / pl->Maxhp()) * 390), 495);
        hpbar_rc.top = 55;
        hpbar_rc.bottom = 90;
        ui_imgs[0]->img.TransparentBlt(dc, {0, 0, 500, 100}, MAGENTA);
        ui_imgs[1]->img.StretchBlt(dc, hpbar_rc );
        ui_imgs[2]->img.TransparentBlt(dc, {0, 0, 100, 100}, RGB(255, 0, 255));
    }

    void ProcessReceivedData(const char*, int);
    void SendToServer(const char* data, int len);
    void ReceiveFromServer();
    void SendLoginPacket(const char* name);
    void SendMovePacket(char direction);
    void SendChatPacket(const char* message);

private:
    vector<char> packets;
    SOCKET g_socket;
    vector<Image*> imgs; // player image = 0, 1, 2,
    vector<Image*> ui_imgs; // 0 - hp bar, 1/2/3 characeter
    vector<Object*> objs;
    Player* pl;
    int curx = 0, cury = 0;
    Tilemap* map;
    chrono::time_point<chrono::high_resolution_clock> last_move_time; // 1초에 한번만 이동하도록 함.
    chrono::time_point<chrono::high_resolution_clock> last_atk_time; // 1초에 한번만 공격 가능
};
