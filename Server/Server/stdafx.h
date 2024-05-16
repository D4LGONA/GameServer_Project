#pragma once

#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <random>
#include <thread>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;

constexpr int EYESIGHT = 5;