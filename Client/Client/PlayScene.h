#pragma once
#include "stdafx.h"
#include "Scene.h"
#include "Tile.h"
#include "Player.h"

class PlayScene : public Scene
{
public:
	PlayScene(SOCKET s, const char* username)
		: g_socket{s}
	{
		pl = new Player();
		SendLoginPacket(username);
		ReceiveFromServer();
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
		ReceiveFromServer();
	}

	void keydown(WPARAM wparam) override
	{
		POINT player_p = { curx + 10, cury + 10 };
		switch (wparam)
		{
		case VK_UP:
			if (chrono::high_resolution_clock::now() - last_move_time > 1s)
				if (canMove({ player_p.x, player_p.y - 1 }))
					SendMovePacket(0);
			break;
		case VK_DOWN:
			if (chrono::high_resolution_clock::now() - last_move_time > 1s)
				if (canMove({ player_p.x, player_p.y + 1 }))
					SendMovePacket(1);
			break;
		case VK_LEFT:
			if (chrono::high_resolution_clock::now() - last_move_time > 1s)
				if (canMove({ player_p.x - 1, player_p.y }))
					SendMovePacket(2);
			break;
		case VK_RIGHT:
			if (chrono::high_resolution_clock::now() - last_move_time > 1s)
				if (canMove({ player_p.x + 1, player_p.y}))
					SendMovePacket(3);
			break;
		case 'a': // 4방향 공격
		case 'A':
			break;
		}

	}

	void keyup(WPARAM wparam) override
	{

	}

	void LbuttonDown(int x, int y) override {}

	void reset() override
	{

	}

	bool canMove(POINT pt)
	{
		if (map->getmap()[pt.x][pt.y].can_move)
			return true;
		return false;
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
	Player* pl;
	int curx = 0, cury = 0;
	Tilemap* map;
	chrono::time_point<chrono::high_resolution_clock> last_move_time; // 1초에 한번만 이동하도록 함.
	chrono::time_point<chrono::high_resolution_clock> last_atk_time; // 1초에 한번만 공격 가능
};