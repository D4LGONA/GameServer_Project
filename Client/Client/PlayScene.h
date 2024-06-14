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
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/w_p.png"));
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/w_p.png"));
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/boss_c.png"));

        pl = new Player();
        SendLoginPacket(username);
        ReceiveFromServer();
        map = new Tilemap();
        map->load("tilemap_test.txt");
        UIsetup();
    }

    ~PlayScene() {
        SendLogoutPacket();

        delete pl; delete map;
        for (auto& a : imgs)
            delete a;
        imgs.clear();
        for (auto& [_, a] : objs)
            delete a;
        objs.clear();
        for (auto& a : ui_imgs)
            delete a;
        ui_imgs.clear();
    }

    void render(HDC& dc, HWND& hwnd) override
    {
        if (imgs.size() == 0) return;
        map->render(dc, curx, cury);
        for (auto& [_, a] : objs)
        {
            a->render(dc, imgs[a->visual], curx, cury);
        }

        for (auto it = eft_objs.begin(); it != eft_objs.end();) {
            if (chrono::high_resolution_clock::now() - (*it).second < 500ms) {
                POINT pt = { (*it).first.x, (*it).first.y };
                int left = (pt.x - curx) * 50;
                int top = (pt.y - cury) * 50;
                int right = (pt.x + 1 - curx) * 50;
                int bottom = (pt.y + 1 - cury) * 50;

                // CImage�� AlphaBlend �޼��带 ����Ͽ� �̹��� �׸���
                ui_imgs[3]->img.AlphaBlend(dc, left, top, right - left, bottom - top, 0, 0, ui_imgs[3]->img.GetWidth(), ui_imgs[3]->img.GetHeight(), 128);

                ++it;
            }
            else {
                // ����Ʈ �ð��� �������� ���Ϳ��� ����
                it = eft_objs.erase(it);
            }
        }

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
        ui_imgs.push_back(new Image());
        ui_imgs.back()->img.Load(TEXT("resources/atk1.png"));
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
    bool ProcessPackets();
    void SendToServer(const char* data, int len);
    void ReceiveFromServer();
    void SendLoginPacket(const char* name);
    void SendMovePacket(char direction);
    void SendChatPacket(const char* message);
    void SendAtkPacket(char atk_type);
    void SendLogoutPacket();

private:
    vector<char> packets;
    SOCKET g_socket;
    vector<Image*> imgs; // player image = 0, 1, 2, monsters = 3
    vector<Image*> ui_imgs; // 0 - hp bar, 1/2/3 characeter
    vector<pair<POINT, chrono::time_point<chrono::high_resolution_clock>>> eft_objs; // ����Ʈ ��
    unordered_map<int, Object*> objs;
    Player* pl;
    int curx = 0, cury = 0;
    Tilemap* map;
    chrono::time_point<chrono::high_resolution_clock> last_move_time; // 1�ʿ� �ѹ��� �̵��ϵ��� ��.
    chrono::time_point<chrono::high_resolution_clock> last_atk_time; // 1�ʿ� �ѹ��� ���� ����
};
