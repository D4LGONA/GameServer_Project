#pragma once
#include "stdafx.h"
#include "Image.h"



class Tile
{
public:
	Tile(int idxx, int idxy) : x{ idxx }, y{ idxy } { }
	~Tile();

	void render(HDC dc, vector<Image*>& vec, int padx, int pady); // 타일을 그리는 함수
	void write(ofstream& out) // 타일 값 저장하는 함수
	{
		out << type << " " << x << " " << y << " " << can_move << " " << canmove_dir << endl;
	}

	void read(ifstream& in)
	{
		in >> type >> x >> y >> can_move >> canmove_dir;
	}

	void setState(int type, bool cm, int cm_dir) // 
	{
		this->type = type;
		this->canmove_dir = cm_dir;
		this->can_move = cm;
	}

private: // 하 enum을 결국 이렇게 적어 버렸네..
	int type = GRASS;
	int x, y;
	bool can_move = true;
	int canmove_dir = CANT; // 아무방향에서도 지나갈수 없다는 뜻.
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




		// 타일맵 전체 초기화
		for (int i = 0; i < 2000; ++i) {
			vector<Tile*> row;
			for (int j = 0; j < 2000; ++j) {
				row.push_back(new Tile(i, j)); // 새로운 타일 생성
			}
			map.push_back(row);
		}
	}
	
	void render(HDC dc, int padx, int pady) // 모든 타일 그리기.
	{
		for (int i = max(padx, 0); i < min(padx + 20, 2000); ++i)
			for (int j = max(pady, 0); j < min(pady + 20, 2000); ++j)
				map[i][j]->render(dc, img_refs, padx, pady);
	}

	void save(string filepath)
	{
		ofstream out{ filepath };

		for (vector<Tile*>& T : map)
			for (Tile*& t : T)
				t->write( out );
	}

	void load(string filepath)
	{
		ifstream in{ filepath };

		if (!in.is_open()) {
			cerr << "Error: Failed to open file " << filepath << endl;
			return;
		}

		int type, x, y, can_move, canmove_dir;
		for (int i = 0; i < map.size(); ++i) {
			for (int j = 0; j < map[i].size(); ++j) {
				if (!(in >> type >> x >> y >> can_move >> canmove_dir)) {
					cerr << "Error: Failed to read tile data from file " << filepath << endl;
					return;
				}
				map[i][j]->setState(type, can_move, canmove_dir);
			}
		}



		// 자 물 채웠고 이제
		for (int i = 990; i < 1010; ++i)
		{
			for (int j = 0; j < 2000; ++j)
				map[i][j]->setState(WATER, false, CANT);
		}
		for (int i = 990; i < 1010; ++i)
		{
			for (int j = 0; j < 2000; ++j)
				map[j][i]->setState(WATER, false, CANT);
		}
		// 모래채울차례
		for (int i = 0; i < 990; ++i) // x = 0 ~ 990
			for (int j = 1010; j < 2000; ++j) // y = 1010 ~ 2000
				map[i][j]->setState(DUST, true, CANT);
		// 불땅
		for (int i = 1010; i < 2000; ++i) // x = 0 ~ 990
			for (int j = 1010; j < 2000; ++j) // y = 1010 ~ 2000
				map[i][j]->setState(FIREFIELD, true, CANT);
		// 숲
		for (int i = 1010; i < 2000; ++i) // x = 0 ~ 990
			for (int j = 0; j < 990; ++j) // y = 1010 ~ 2000
				map[i][j]->setState(FOREST, true, CANT);

		// 보스 공간 ㄷㄷ
		for (int y = 999 - 100; y <= 999 + 100; ++y) {
			for (int x = 999 - 100; x <= 999 + 100; ++x) {
				// 여기에 각 x, y 좌표에서의 작업을 수행합니다.
				map[x][y]->setState(BOSS, true, CANT);
			}
		}

	}

	vector<vector<Tile*>>& getmap() { return map; }

private:
	vector<vector<Tile*>> map;
	vector<Image*> img_refs;
};