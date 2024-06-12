#include "stdafx.h"
#include "Player.h"

array<Player, MAX_USER> players;

void Player::send(void* packet)
{
	EXT_OVER* ov = new EXT_OVER();
	ov->setup_send(reinterpret_cast<char*>(packet));

	printf("Before WSASend: wsabuf.len = %lu, wsabuf.buf = %p\n", ov->wsabuf.len, ov->wsabuf.buf);
	int result = WSASend(socket, &ov->wsabuf, 1, 0, 0, &ov->over, NULL);
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			printf("WSASend failed with error: %d\n", error);
			closesocket(socket);
		}
		else
			printf("WSASend pending: %d\n", error);
	}
}

void Player::recv()
{
	// 소켓이 유효한지 확인
	if (socket == INVALID_SOCKET) {
		printf("Invalid socket\n");
		return;
	}

	// EXT_OVER 구조체 초기화
	ZeroMemory(&over.over, sizeof(over.over));
	over.wsabuf.len = BUFSIZE - packet_data.size();
	over.wsabuf.buf = over.wb_buf + packet_data.size();

	// WSABUF 버퍼가 유효한지 확인
	if (over.wsabuf.buf == nullptr) {
		printf("Buffer is null\n");
		return;
	}

	DWORD flags = 0;
	DWORD bytesReceived = 0;

	// WSARecv 호출 전 진단 정보 출력
	printf("Before WSARecv: wsabuf.len = %lu, wsabuf.buf = %p\n", over.wsabuf.len, over.wsabuf.buf);

	// WSARecv 호출
	int result = WSARecv(socket, &over.wsabuf, 1, &bytesReceived, &flags, &over.over, NULL);
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			printf("WSARecv failed with error: %d\n", error);
			closesocket(socket);
			WSACleanup();
		}
		else
		{
			printf("WSARecv pending: %d\n", error);
		}
	}
}

void Player::packet_setup(PK_TYPE n, int who = -1, const char* msg = NULL)
{
	switch (n)
	{
	case LOGIN_FAIL:
	{
		SC_LOGIN_FAIL_PACKET packet;
		packet.size = sizeof(SC_LOGIN_FAIL_PACKET);
		packet.type = SC_LOGIN_FAIL;
		send(&packet);
		break;
	}
	case LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET packet;
		packet.id = id;
		packet.size = sizeof(SC_LOGIN_INFO_PACKET);
		packet.type = SC_LOGIN_INFO;
		packet.hp = hp;
		packet.max_hp = hp;
		packet.exp = exp;
		packet.level = level;
		packet.visual = 0;
		packet.x = x;
		packet.y = y;
		send(&packet);
		break;
	}
	case ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET packet;
		packet.size = sizeof(SC_ADD_OBJECT_PACKET);
		packet.type = SC_LOGIN_INFO;
		packet.x = x;
		packet.y = y;
		memcpy(packet.name, name, NAME_SIZE);
		packet.id = who;
		packet.visual = 0;
		send(&packet);
		break;
	}
	case REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET packet;
		packet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
		packet.type = SC_REMOVE_OBJECT;
		packet.id = who;
		send(&packet);
		break;
	}
	case MOVE_OBJECT:
	{
		SC_MOVE_OBJECT_PACKET packet;
		packet.size = sizeof(SC_MOVE_OBJECT_PACKET);
		packet.type = SC_MOVE_OBJECT;
		packet.id = who;
		packet.move_time = last_move_time;
		packet.x = x;
		packet.y = y;
		send(&packet);
		break;
	}
	case CHAT:
	{
		SC_CHAT_PACKET packet;
		packet.size = sizeof(SC_CHAT_PACKET);
		packet.type = SC_CHAT;
		packet.id = who;
		memcpy(packet.mess, msg, CHAT_SIZE);
		send(&packet);
		break;
	}
	case STAT_CHANGE: // 흠...
	{
		SC_STAT_CHANGE_PACKET packet;
		packet.size = sizeof(SC_STAT_CHANGE_PACKET);
		packet.type = SC_STAT_CHANGE;
		packet.exp = exp;
		packet.hp = hp;
		packet.level = level;
		packet.max_hp = max_hp;
		send(&packet);
		break;
	}
	default:
		break;
	}
}

void Player::handle_packet(char* packet, unsigned short length, SQLHSTMT& hstmt)
{
	char type = packet[2];

	switch (type) // todo
	{
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		
		string str(p->name);
		wstring ws = strtowstr(str);
		std::wstring query = L"SELECT * FROM user_table WHERE user_name = '" + ws + L"';";

		SQLCloseCursor(hstmt);
		SQLRETURN ret = SQLExecDirect(hstmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
			SQLLEN cbID = 0, cbx = 0, cby = 0, cblevel = 0, cbid = 0, cbexp = 0, cbhp = 0, cbatk = 0, cbdef = 0, cbvisual = 0, cbd = 0;
			bool playing;
			ret = SQLBindCol(hstmt, 1, SQL_C_CHAR, name, 20, &cbID);
			ret = SQLBindCol(hstmt, 2, SQL_C_SHORT, &x, 10, &cbx);
			ret = SQLBindCol(hstmt, 3, SQL_C_SHORT, &y, 10, &cby);
			ret = SQLBindCol(hstmt, 4, SQL_C_SHORT, &level, 10, &cblevel);
			ret = SQLBindCol(hstmt, 5, SQL_C_SHORT, &id, 10, &cbid);
			ret = SQLBindCol(hstmt, 6, SQL_C_LONG, &exp, 10, &cbexp);
			ret = SQLBindCol(hstmt, 7, SQL_C_SHORT, &hp, 10, &cbhp);
			ret = SQLBindCol(hstmt, 8, SQL_C_SHORT, &attack, 10, &cbatk);
			ret = SQLBindCol(hstmt, 9, SQL_C_SHORT, &defense, 10, &cbdef);
			ret = SQLBindCol(hstmt, 10, SQL_C_LONG, &visual, 10, &cbvisual);
			ret = SQLBindCol(hstmt, 11, SQL_C_BIT, &playing, 1, &cbd);

			for (int i = 0; ; i++) { 
				ret = SQLFetch(hstmt);
				if (ret == SQL_NO_DATA and i == 0)
				{
					std::wstring insert_query = L"INSERT INTO user_table (user_name, user_x, user_y, user_level, user_id, user_exp, user_hp, user_atk, user_def, user_visual, user_isplay) VALUES ('" +
						ws + L"', " +
						to_wstring(x) + L", " +
						to_wstring(y) + L", " +
						to_wstring(level) + L", " +
						to_wstring(id) + L", " +
						to_wstring(exp) + L", " +
						to_wstring(hp) + L", " +
						to_wstring(attack) + L", " +
						to_wstring(defense) + L", " +
						to_wstring(visual) + L", " +
						to_wstring(1) + L");";

					SQLCloseCursor(hstmt);
					ret = SQLExecDirect(hstmt, (SQLWCHAR*)insert_query.c_str(), SQL_NTS);
					break;
				}
				if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
				{
					if (playing)
					{
						packet_setup(LOGIN_FAIL);
						state = NONE;
						return;
					}
					else
					{
						std::wstring update_query = L"UPDATE user_table SET user_isplay = 1 WHERE user_name = '" + ws + L"';";
						SQLCloseCursor(hstmt);
						ret = SQLExecDirect(hstmt, (SQLWCHAR*)update_query.c_str(), SQL_NTS);
						break;
					}
				}
			}
		}
		packet_setup(LOGIN_INFO);
		state = PLAYING;
		break;
	}
	case CS_MOVE: // 이동패킷이 왔다
	{
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);

		// 근데 여기는 맵이 없는데 어떻게 하지 ?
		switch (p->direction)
		{
		case 0: // up
			y--;
			break;
		case 1: // down
			y++;
			break;
		case 2: // left
			x--;
			break;
		case 3: // right
			x++;
			break;
		}

		last_move_time = p->move_time;
		// 시야의 모든 애들한테 누가 어디로 이동했다는 사실을 알려야 함...?
		packet_setup(MOVE_OBJECT, id);
		break;
	}
	case CS_CHAT:
	{
		CS_CHAT_PACKET* p = (CS_CHAT_PACKET*)packet;
		// 모든 플레이어에게 
		std::cout << "Received CS_CHAT packet: " << p->mess << std::endl;
		break;
	}
	default:
		std::cout << "Unknown packet type: " << (int)type << std::endl;
		break;
	}
}

