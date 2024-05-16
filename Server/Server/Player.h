#pragma once
#include "stdafx.h"
#include "Object.h"

enum STATES{ NONE = 0, CONNECTING = 1, PLAYING = 2 };
class Player : public Object
{
	SOCKET socket;
	STATES state = NONE;
	unordered_set<int> view_list;

public:
	Player()
	{
		Object();
		socket = 0;
		state = NONE;
	}

	~Player() {}
};

