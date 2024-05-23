#pragma once
#include "stdafx.h"
#include "Image.h"



class Tile
{
public:
	Tile(int idxx, int idxy) : x{ idxx }, y{ idxy } { }
	~Tile();

	void render(HDC dc, vector<Image*>& vec, int padx, int pady); // Ÿ���� �׸��� �Լ�
	void write(ofstream& out) // Ÿ�� �� �����ϴ� �Լ�
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

private: // �� enum�� �ᱹ �̷��� ���� ���ȳ�..
	int type = GRASS;
	int x, y;
	bool can_move = true;
	int canmove_dir = CANT; // �ƹ����⿡���� �������� ���ٴ� ��.
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




		// Ÿ�ϸ� ��ü �ʱ�ȭ
		for (int i = 0; i < 2000; ++i) {
			vector<Tile*> row;
			for (int j = 0; j < 2000; ++j) {
				row.push_back(new Tile(i, j)); // ���ο� Ÿ�� ����
			}
			map.push_back(row);
		}
	}
	
	void render(HDC dc, int padx, int pady) // ��� Ÿ�� �׸���.
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



		// �� �� ä���� ����
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
		// ��ä������
		for (int i = 0; i < 990; ++i) // x = 0 ~ 990
			for (int j = 1010; j < 2000; ++j) // y = 1010 ~ 2000
				map[i][j]->setState(DUST, true, CANT);
		// �Ҷ�
		for (int i = 1010; i < 2000; ++i) // x = 0 ~ 990
			for (int j = 1010; j < 2000; ++j) // y = 1010 ~ 2000
				map[i][j]->setState(FIREFIELD, true, CANT);
		// ��
		for (int i = 1010; i < 2000; ++i) // x = 0 ~ 990
			for (int j = 0; j < 990; ++j) // y = 1010 ~ 2000
				map[i][j]->setState(FOREST, true, CANT);

		// ���� ���� ����
		for (int y = 999 - 100; y <= 999 + 100; ++y) {
			for (int x = 999 - 100; x <= 999 + 100; ++x) {
				// ���⿡ �� x, y ��ǥ������ �۾��� �����մϴ�.
				map[x][y]->setState(BOSS, true, CANT);
			}
		}

	}

	vector<vector<Tile*>>& getmap() { return map; }

private:
	vector<vector<Tile*>> map;
	vector<Image*> img_refs;
};