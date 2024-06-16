#pragma once
#include "stdafx.h"
#include "Image.h"



class Tile
{
public:
	Tile() {}
	Tile(int idxx, int idxy) : x{ idxx }, y{ idxy } { }
	~Tile() {};

	void render(HDC dc, vector<Image*>& vec, int padx, int pady); // 타일을 그리는 함수
	void write(ofstream& out) // 타일 값 저장하는 함수
	{
		out << type << " " << x << " " << y << " " << can_move << endl;
	}

	void read(ifstream& in)
	{
		in >> type >> x >> y >> can_move;
	}

	void setState(int type, bool cm) // 
	{
		this->type = type;
		this->can_move = cm;
	}

	int type = GRASS;
	int x, y;
	bool can_move = true;
};

class Tilemap
{
public:
	Tilemap()
	{
		// img_refs 초기화
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t1.png")); //0번 이미지 - 잔디.
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t2.png")); //1번 이미지 - 물
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t3.png")); //3번 이미지 - 모래
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t4.png")); //4번 이미지 - 돌
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t5.png")); //5번 이미지 - 네더랙
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t6.png")); //6번 이미지 - 길
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t7.png")); //7번 이미지 - 숲
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t8.png")); //8번 이미지 - 보스!
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t9.png")); //8번 이미지 - 보스!
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t10.png")); //8번 이미지 - 보스!




		// 타일맵 전체 초기화
		for (int i = 0; i < W_WIDTH; ++i) {
			for (int j = 0; j < W_HEIGHT; ++j) {
				map[i][j] = Tile(i, j);
			}
		}

		//// 먼저 물 채우기
		//for (int i = 0; i < 2000; ++i)
		//	for (int j = 0; j < 2000; ++j)
		//		map[i][j].setState(WATER, false);

		//// 1번 공간 채우기
		//for (int i = 0; i < 990; ++i) // x = 0 ~ 990
		//	for (int j = 0; j < 990; ++j) // y = 1010 ~ 2000
		//		map[i][j].setState(GRASS, true);

		//// 2번 공간 채우기
		//for (int i = 1010; i < 2000; ++i) // x = 0 ~ 990
		//	for (int j = 0; j < 990; ++j) // y = 1010 ~ 2000
		//		map[i][j].setState(FOREST, true);

		//// 3번 공간 채우기
		//for (int i = 1010; i < 2000; ++i) // x = 0 ~ 990
		//	for (int j = 1010; j < 2000; ++j) // y = 1010 ~ 2000
		//		map[i][j].setState(FIREFIELD, true);

		//// 4번 공간 채우기
		//for (int i = 0; i < 990; ++i) // x = 0 ~ 990
		//	for (int j = 1010; j < 2000; ++j) // y = 1010 ~ 2000
		//		map[i][j].setState(DUST, true);

		//// 보스 맵 물
		//for (int i = 999 - 105; i < 999 + 105; ++i) 
		//	for (int j = 999 - 105; j < 999 + 105; ++j) 
		//		map[i][j].setState(WATER, false);
	
		//// 보스 맵
		//for (int i = 999 - 100; i < 999 + 100; ++i) 
		//	for (int j = 999 - 100; j < 999 + 100; ++j) 
		//		map[i][j].setState(BOSS, true);
	
		//// 다리들
		//for (int i = 990; i < 1010; ++i)
		//	for(int j = 999+100; j < 999 + 150; ++j)
		//		map[i][j].setState(ROCK, true);

		//for (int i = 500 - 25; i < 500 + 25; ++i)
		//	for(int j = 990; j < 1010; ++j)
		//		map[i][j].setState(ROCK, true);

		//for (int i = 1500 - 25; i < 1500 + 25; ++i)
		//	for (int j = 990; j < 1010; ++j)
		//		map[i][j].setState(ROCK, true);

		//for (int i = 500 - 25; i < 500 + 25; ++i)
		//	for (int j = 990; j < 1010; ++j)
		//		map[j][i].setState(ROCK, true);

		//{
		//	int radius = 10;
		//	int centerX = 100, centerY = 40;
		//	int radiusSquared = radius * radius;
		//	for (int x = centerX - radius; x <= centerX + radius; ++x) {
		//		for (int y = centerY - radius; y <= centerY + radius; ++y) {
		//			if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radiusSquared) {
		//				if (x >= 0 && x < W_WIDTH && y >= 0 && y < W_HEIGHT) {
		//					map[x][y].setState(ROCK, false);
		//				}
		//			}
		//		}
		//	}
		//}

		//{
		//	int radius = 10;
		//	int centerX = 300, centerY = 268;
		//	int radiusSquared = radius * radius;
		//	for (int x = centerX - radius; x <= centerX + radius; ++x) {
		//		for (int y = centerY - radius; y <= centerY + radius; ++y) {
		//			if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radiusSquared) {
		//				if (x >= 0 && x < W_WIDTH && y >= 0 && y < W_HEIGHT) {
		//					map[x][y].setState(ROCK, false);
		//				}
		//			}
		//		}
		//	}
		//}

		//{
		//	for (int i = 0; i < 5000; ++i)
		//	{
		//		int a = rand() % 1000, b = rand() % 1000;
		//		if (map[a][b].type != GRASS) continue;
		//		map[a][b].setState(ROAD, true);
		//	}
		//}

		//{
		//	int radius = 10;
		//	int centerX = 820, centerY = 100;
		//	int radiusSquared = radius * radius;
		//	for (int x = centerX - radius; x <= centerX + radius; ++x) {
		//		for (int y = centerY - radius; y <= centerY + radius; ++y) {
		//			if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radiusSquared) {
		//				if (x >= 0 && x < W_WIDTH && y >= 0 && y < W_HEIGHT) {
		//					map[x][y].setState(ROCK, false);
		//				}
		//			}
		//		}
		//	}
		//}

		//{
		//	int radius = 10;
		//	int centerX = 540, centerY = 420;
		//	int radiusSquared = radius * radius;
		//	for (int x = centerX - radius; x <= centerX + radius; ++x) {
		//		for (int y = centerY - radius; y <= centerY + radius; ++y) {
		//			if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radiusSquared) {
		//				if (x >= 0 && x < W_WIDTH && y >= 0 && y < W_HEIGHT) {
		//					map[x][y].setState(ROCK, false);
		//				}
		//			}
		//		}
		//	}
		//}

		//{
		//	for (int i = 0; i < 5000; ++i)
		//	{
		//		int a = rand() % 1000 + 1000, b = rand() % 1000;
		//		if (map[a][b].type != FOREST) continue;
		//		map[a][b].setState(FLOWER, true);
		//	}
		//}

		//{
		//	int radius = 17;
		//	int centerX = 1280, centerY = 1900;
		//	int radiusSquared = radius * radius;
		//	for (int x = centerX - radius; x <= centerX + radius; ++x) {
		//		for (int y = centerY - radius; y <= centerY + radius; ++y) {
		//			if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radiusSquared) {
		//				if (x >= 0 && x < W_WIDTH && y >= 0 && y < W_HEIGHT) {
		//					map[x][y].setState(ROCK, false);
		//				}
		//			}
		//		}
		//	}
		//}

		//{
		//	int radius = 6;
		//	int centerX = 1790, centerY = 1305;
		//	int radiusSquared = radius * radius;
		//	for (int x = centerX - radius; x <= centerX + radius; ++x) {
		//		for (int y = centerY - radius; y <= centerY + radius; ++y) {
		//			if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radiusSquared) {
		//				if (x >= 0 && x < W_WIDTH && y >= 0 && y < W_HEIGHT) {
		//					map[x][y].setState(ROCK, false);
		//				}
		//			}
		//		}
		//	}
		//}

		//{
		//	for (int i = 0; i < 5000; ++i)
		//	{
		//		int a = rand() % 1000 + 1000, b = rand() % 1000 + 1000;
		//		if (map[a][b].type != FIREFIELD) continue;
		//		map[a][b].setState(ROAD, true);
		//	}
		//}

		//{
		//	int radius = 8;
		//	int centerX = 792, centerY = 1900;
		//	int radiusSquared = radius * radius;
		//	for (int x = centerX - radius; x <= centerX + radius; ++x) {
		//		for (int y = centerY - radius; y <= centerY + radius; ++y) {
		//			if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radiusSquared) {
		//				if (x >= 0 && x < W_WIDTH && y >= 0 && y < W_HEIGHT) {
		//					map[x][y].setState(ROCK, false);
		//				}
		//			}
		//		}
		//	}
		//}

		//{
		//	int radius =18;
		//	int centerX = 204, centerY = 1305;
		//	int radiusSquared = radius * radius;
		//	for (int x = centerX - radius; x <= centerX + radius; ++x) {
		//		for (int y = centerY - radius; y <= centerY + radius; ++y) {
		//			if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radiusSquared) {
		//				if (x >= 0 && x < W_WIDTH && y >= 0 && y < W_HEIGHT) {
		//					map[x][y].setState(ROCK, false);
		//				}
		//			}
		//		}
		//	}
		//}

		//{
		//	for (int i = 0; i < 5000; ++i)
		//	{
		//		int a = rand() % 1000, b = rand() % 1000 + 1000;
		//		if (map[a][b].type != DUST) continue;
		//		map[a][b].setState(WT2, true);
		//	}
		//}


		//for (int i = 0; i < 10000; ++i)
		//{
		//	int a = rand() % 2000, b = rand() % 2000;
		//	if (map[a][b].can_move == false) continue;
		//	map[a][b].setState(ROCK, false);
		//}

	}
	
	void render(HDC dc, int padx, int pady) // 모든 타일 그리기.
	{
		for (int i = max(padx, 0); i < min(padx + 200, W_WIDTH); ++i)
			for (int j = max(pady, 0); j < min(pady + 200, W_HEIGHT); ++j)
				map[i][j].render(dc, img_refs, padx, pady);
	}

	void save(string filepath)
	{
		ofstream out{ filepath };
		ofstream out2{ "map_server.txt" };

		for (array<Tile, W_WIDTH>& T : map)
			for (Tile& t : T)
				t.write( out );

		for (array<Tile, W_WIDTH>& T : map)
		{
			for (Tile& t : T)
				out2 << t.can_move << " ";
			out2 << endl;
		}
	}

	void load(string filepath)
	{
		std::ifstream in(filepath);

		if (!in.is_open()) {
			std::cerr << "Error: Failed to open file " << filepath << std::endl;
			return;
		}

		std::stringstream buffer;
		buffer << in.rdbuf();
		in.close();

		int type, x, y, can_move;
		for (int i = 0; i < map.size(); ++i) {
			for (int j = 0; j < map[i].size(); ++j) {
				if (!(buffer >> type >> x >> y >> can_move)) {
					std::cerr << "Error: Failed to read tile data from file " << filepath << std::endl;
					return;
				}
				map[i][j].setState(type, can_move);
			}
		}


		

		//// 자 물 채웠고 이제
		//for (int i = 990; i < 1010; ++i)
		//{
		//	for (int j = 0; j < 2000; ++j)
		//		map[i][j]->setState(WATER, false, CANT);
		//}
		//for (int i = 990; i < 1010; ++i)
		//{
		//	for (int j = 0; j < 2000; ++j)
		//		map[j][i]->setState(WATER, false, CANT);
		//}
		//// 모래채울차례
		//for (int i = 0; i < 990; ++i) // x = 0 ~ 990
		//	for (int j = 1010; j < 2000; ++j) // y = 1010 ~ 2000
		//		map[i][j]->setState(DUST, true, CANT);
		//// 불땅
		//for (int i = 1010; i < 2000; ++i) // x = 0 ~ 990
		//	for (int j = 1010; j < 2000; ++j) // y = 1010 ~ 2000
		//		map[i][j]->setState(FIREFIELD, true, CANT);
		//// 숲
		//for (int i = 1010; i < 2000; ++i) // x = 0 ~ 990
		//	for (int j = 0; j < 990; ++j) // y = 1010 ~ 2000
		//		map[i][j]->setState(FOREST, true, CANT);

		//// 보스 공간 ㄷㄷ
		//for (int y = 999 - 100; y <= 999 + 100; ++y) {
		//	for (int x = 999 - 100; x <= 999 + 100; ++x) {
		//		// 여기에 각 x, y 좌표에서의 작업을 수행합니다.
		//		map[x][y]->setState(BOSS, true, CANT);
		//	}
		//}

	}

	array<array<Tile, W_WIDTH>, W_HEIGHT>& getmap() { return map; }

private:
	array<array<Tile, W_WIDTH>, W_HEIGHT> map;
	vector<Image*> img_refs;
};