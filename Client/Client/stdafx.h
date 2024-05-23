#pragma once

#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <atlimage.h>
#include <fstream>
#include <queue>
#include <cmath>

#define WIDTHMAX 1000
#define HEIGHTMAX 1000
#define MAGENTA RGB(255,0,255)

using namespace std;

enum DIR {UP, DOWN, LEFT, RIGHT, CANT};
enum TILETYPE {GRASS = 0, WATER = 1, DUST = 2, ROCK = 3, FIREFIELD = 4, ROAD = 5, FOREST = 6, BOSS = 7};



#include "Image.h"
#include "Scene.h"
