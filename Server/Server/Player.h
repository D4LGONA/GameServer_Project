#pragma once
#include "stdafx.h"
#include "Object.h"

enum STATES{ NONE = 0, CONNECTING = 1, PLAYING = 2 };
class Player : public Object
{
	EXT_OVER over;
	SOCKET socket;
	vector<char> packet_data; // deque를 사용할까?
	int exp;
	unsigned int last_move_time;
	unordered_set<int> view_list;
	mutex vl_l;

public:
	STATES state;
	Player() : socket(0), state(NONE) {}

	~Player() {}

	void setup(int id, SOCKET sock)
	{
		this->id = id;
		socket = sock;
		exp = 0;
		attack = 10;
		defense = 10;
		hp = 100;
		last_move_time = 0;
		level = 1;
		max_hp = 100;
		name[0] = 0;
		state = CONNECTING;
		visual = 1;
		x = 10;
		y = 10;
	}

	void send(void* packet);
	void recv();

	void send_login_fail();
	void send_login_info();
	void send_add_object(int ox, int oy, const char* oname, int oid, int ov);
	void send_remove_object(int oid);
	void send_move_object(int ox, int oy, int oid, unsigned int lmt);
	void send_chat(int oid, const char* msg);
	void send_stat_change(int oe, int oh, int ol, int omh);
	void update_packet(EXT_OVER*& ov, DWORD num_bytes)
	{
		packet_data.insert(packet_data.end(), ov->wb_buf, ov->wb_buf + num_bytes);
	}

	void process_buffer(SQLHSTMT& hstmt)
	{
		if (packet_data.size() == 0) return;
		if (packet_data[0] <= packet_data.size()) // 일정 이상 받아지면
		{
			unsigned short expected_packet_size = packet_data[0];
			handle_packet(packet_data.data(), expected_packet_size, hstmt);
			packet_data.erase(packet_data.begin(), packet_data.begin() + expected_packet_size);
		}
	}

	void handle_packet(char* packet, unsigned short length, SQLHSTMT& hstmt);
	
	// 추가적인 기능을 위해 getter와 setter를 추가할 수 있습니다.
	SOCKET get_socket() const { return socket; }
	void set_socket(SOCKET sock) { socket = sock; }

	STATES get_state() const { return state; }
	void set_state(STATES st) { state = st; }

	char* getName() { return name; }
	int getEXP() { return exp; }

};

