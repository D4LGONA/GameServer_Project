#include "TileMapEditor.h"

void TileMapScene::keydown(WPARAM wparam)
{
	switch (wparam)
	{
	case VK_RIGHT:
		curx++;
		break;
	case VK_LEFT:
		curx--;
		break;
	case VK_UP:
		cury--;
		break;
	case VK_DOWN:
		cury++;
		break;

	case '1': // 어떤 타일인지를 바꾸는 것
		curtype = GRASS;
		break;
	case '2':
		curtype = WATER;
		break;
	case '3':
		curtype = DUST;
		break;
	case '4':
		curtype = ROCK;
		break;
	case '5':
		curtype = FIREFIELD;
		break;
	case '6':
		curtype = ROAD;
		break;
	case '7':
		curtype = FOREST;
		break;

	case 'q':
	case 'Q':
		curcan_move = !curcan_move;
		break;

	case 'S':
	case 's': // save
		map->save("tilemap_test.txt");
		break;

	case 'L':
	case 'l':
		map->load("tilemap_test.txt");
	}

}

void TileMapScene::LbuttonDown(int x, int y)
{
	map->getmap()[(x / 50) + curx][(y / 50) + cury]->setState(curtype, curcan_move, curcanmove_dir);
}