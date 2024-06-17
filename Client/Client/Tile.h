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
	}
	
	void render(HDC dc, int padx, int pady)
	{
		for (int i = max(padx, 0); i < min(padx + 20, W_WIDTH); ++i)
			for (int j = max(pady, 0); j < min(pady + 20, W_HEIGHT); ++j)
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
	}

	array<array<Tile, W_WIDTH>, W_HEIGHT>& getmap() { return map; }

private:
	array<array<Tile, W_WIDTH>, W_HEIGHT> map;
	vector<Image*> img_refs;
};