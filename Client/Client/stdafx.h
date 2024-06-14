#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <atlimage.h>
#include <fstream>
#include <queue>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include "../../Server/Server/protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")


#define WIDTHMAX 1000
#define HEIGHTMAX 1000
#define MAGENTA RGB(255,0,255)

using namespace std;

enum DIR {UP, DOWN, LEFT, RIGHT, CANT};
enum TILETYPE {GRASS = 0, WATER = 1, DUST = 2, ROCK = 3, FIREFIELD = 4, ROAD = 5, FOREST = 6, BOSS = 7};

#include "Image.h"
#include "Scene.h"
