#pragma once
#include "stdafx.h"
#include "Scene.h"
#include "Tile.h"
#include "Object.h"
#include "Player.h"
#include <vector>
#include <string>
#include <chrono>
#include <iostream>

class PlayScene : public Scene
{
public:
    PlayScene(SOCKET s, const char* username)
        : g_socket{ s }, in_chat_mode{ false }
    {
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/w_p.png"));
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/1.png"));
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/2.png"));
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/3.png"));
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/4.png"));
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/5.png"));
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/6.png"));
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/7.png"));
        imgs.push_back(new Image());
        imgs.back()->img.Load(TEXT("resources/8.png"));


        pl = new Player();
        pl->name = username;
        map = new Tilemap();
        map->load("tilemap.txt");
        UIsetup();
        is_loading = true;
        SendLoginPacket(username);
        ReceiveFromServer();
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

    void ProcessChar(WPARAM wParam) {
        if (in_chat_mode and chat_message.size() < CHAT_SIZE) {
            if (wParam == VK_BACK) {
                if (!chat_message.empty()) {
                    chat_message.pop_back();
                }
            }
            else if (wParam != '\r') { // '\r' ���ڸ� �����մϴ�.
                chat_message.push_back(static_cast<wchar_t>(wParam));
            }
        }
    }


    void render(HDC& dc, HWND& hwnd) override
    {
        if (is_loading)
        {
            if (imgs.size() == 0) return;
            map->render(dc, curx, cury);
            for (auto& [_, a] : objs) {
                if (a == nullptr) continue;
                a->render(dc, imgs[a->visual], curx, cury);
            }

            for (auto it = eft_objs.begin(); it != eft_objs.end();)
            {
                if (chrono::high_resolution_clock::now() < (*it).second)
                {
                    POINT pt = { (*it).first.x, (*it).first.y };
                    int left = (pt.x - curx) * 50;
                    int top = (pt.y - cury) * 50;
                    int right = (pt.x + 1 - curx) * 50;
                    int bottom = (pt.y + 1 - cury) * 50;

                    ui_imgs[3]->img.AlphaBlend(dc, left, top, right - left, bottom - top, 0, 0, ui_imgs[3]->img.GetWidth(), ui_imgs[3]->img.GetHeight(), 128);

                    ++it;
                }
                else
                    it = eft_objs.erase(it);
            }

            pl->render(dc, imgs[0]);

            UIrender(dc);
            RenderChatBox(dc); // ä�� â�� �׸��ϴ�.
        }
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
        hpbar_rc.right = min(max(105, 110 + (pl->Gethp() / pl->Maxhp()) * 390), 500);
        hpbar_rc.top = 55;
        hpbar_rc.bottom = 90;
        ui_imgs[0]->img.TransparentBlt(dc, { 0, 0, 500, 100 }, MAGENTA);
        ui_imgs[1]->img.StretchBlt(dc, hpbar_rc);
        ui_imgs[2]->img.TransparentBlt(dc, { 0, 0, 100, 100 }, RGB(255, 0, 255));

        SetBkMode(dc, TRANSPARENT); // ���� ��� ���� ����
        SetTextColor(dc, RGB(0, 0, 0)); // �ؽ�Ʈ ���� ���� (���)

        wstring ws = strtowstr(pl->name);
        int textX = 105; // HP ���� left�� �����ϰ� ����
        int textY = 35; // HP �� ���ʿ� ��ġ�ϵ��� ����

        TextOut(dc, textX, textY, ws.c_str(), ws.length());
    }


    void RenderChatBox(HDC& dc) const
    {
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(255, 255, 255));

        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));

        std::wstring chat_content;

        for (auto& t : chat_messages | views::reverse | views::take(5) | views::reverse)
            chat_content += t + L"\n";

        if (in_chat_mode)
            chat_content += L"> " + chat_message;

        // �ؽ�Ʈ�� ���� ���� ���缭 �ڸ���
        int maxWidth = 360; // �ִ� �ؽ�Ʈ �� (chatBox ������ ��)
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SelectObject(dc, hFont);

        SIZE size;
        std::wstring formatted_content;
        std::wstring current_line;

        for (const wchar_t& c : chat_content)
        {
            current_line += c;
            GetTextExtentPoint32(dc, current_line.c_str(), current_line.length(), &size);
            if (size.cx > maxWidth || c == L'\n')
            {
                if (c != L'\n')
                {
                    current_line.pop_back(); // ������ ���ڸ� ���� (�Ѿ ����)
                    formatted_content += current_line + L'\n'; // ���� �߰�
                    current_line = c; // ���ŵ� ���ڷ� ���ο� �� ����
                }
                else
                {
                    formatted_content += current_line;
                    current_line.clear();
                }
            }
        }
        formatted_content += current_line; // ������ �� �߰�

        RECT chatBox = { 0, 950, 400, 1000 };

        RECT textRect = { 20, 950, 380, 1000 };
        RECT calcRect = textRect;
        DrawText(dc, formatted_content.c_str(), -1, &calcRect, DT_LEFT | DT_EDITCONTROL | DT_CALCRECT);

        int newTop = 1000 - (calcRect.bottom - calcRect.top) - 20;
        calcRect.top = newTop;
        calcRect.bottom = 1000;

        chatBox.top = newTop - 10; // �ణ�� ���� ���� �߰�
        chatBox.bottom = 1000;

        FillRect(dc, &chatBox, brush);
        DrawText(dc, formatted_content.c_str(), -1, &calcRect, DT_LEFT | DT_EDITCONTROL);

        DeleteObject(brush);
    }

    void ProcessReceivedData(const char*, int);
    bool ProcessPackets();
    void SendToServer(const char* data, int len);
    void ReceiveFromServer();
    void SendLoginPacket(const char* name);
    void SendMovePacket(char direction);
    void SendChatPacket(const std::wstring& message);
    void SendAtkPacket(char atk_type);
    void SendLogoutPacket();

private:
    bool death = false;
    bool is_loading = false;
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
    bool in_chat_mode; // ä�� ��� ����
    std::wstring chat_message; // ���� �Է� ���� ä�� �޽���
    std::vector<std::wstring> chat_messages; // ä�� �޽����� ������ ����
};
