#pragma once
#include "stdafx.h"
#include "Image.h"



class Tile
{
public:
	Tile() {}
	Tile(int idxx, int idxy) : x{ idxx }, y{ idxy } { }
	~Tile() {};

	void render(HDC dc, vector<Image*>& vec, int padx, int pady); // Ÿ���� �׸��� �Լ�
	void write(ofstream& out) // Ÿ�� �� �����ϴ� �Լ�
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
		// img_refs �ʱ�ȭ
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t1.png")); //0�� �̹��� - �ܵ�.
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t2.png")); //1�� �̹��� - ��
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t3.png")); //3�� �̹��� - ��
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t4.png")); //4�� �̹��� - ��
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t5.png")); //5�� �̹��� - �״���
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t6.png")); //6�� �̹��� - ��
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t7.png")); //7�� �̹��� - ��
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t8.png")); //8�� �̹��� - ����!
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t9.png")); //8�� �̹��� - ����!
		img_refs.emplace_back(new Image());
		img_refs.back()->img.Load(TEXT("resources/t10.png")); //8�� �̹��� - ����!




		// Ÿ�ϸ� ��ü �ʱ�ȭ
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