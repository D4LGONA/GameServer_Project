#include "stdafx.h"
#include "Player.h"

void Player::send(void* packet)
{
	EXT_OVER* ov = new EXT_OVER();
	ov->setup_send(reinterpret_cast<char*>(packet));

	int result = WSASend(socket, &ov->wsabuf, 1, 0, 0, &ov->over, NULL);
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			printf("WSASend failed with error: %d\n", error);
			closesocket(socket);
		}
	}
}

void Player::recv()
{
	EXT_OVER* over = new EXT_OVER();
	ZeroMemory(&over->over, sizeof(over->over));
	over->wsabuf.len = BUFSIZE - packet_data.size();
	over->wsabuf.buf = over->wb_buf + packet_data.size();

	int result = WSARecv(socket, &over->wsabuf, 1, 0, 0, &over->over, NULL);
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			printf("WSARecv failed with error: %d\n", error);
			closesocket(socket);
			WSACleanup();
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
	case STAT_CHANGE: // »Ï...
	{
		SC_STAT_CHANGE_PACKET packet;
		packet.size = sizeof(SC_STAT_CHANGE_PACKET);
		packet.type = SC_STAT_CHANGE;
		packet.exp = exp;
		packet.hp = hp;
		packet.level = level;
		packet.max_hp = max_hp;
		send(&packet);
	}
	default:
		break;
	}
}

void Player::handle_packet(char* packet, unsigned short length, SQLHSTMT& hstmt)
{
	char type = packet[1];

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
			ret = SQLBindCol(hstmt, 6, SQL_C_SHORT, &exp, 10, &cbexp);
			ret = SQLBindCol(hstmt, 7, SQL_C_SHORT, &hp, 10, &cbhp);
			ret = SQLBindCol(hstmt, 8, SQL_C_SHORT, &attack, 10, &cbatk);
			ret = SQLBindCol(hstmt, 9, SQL_C_SHORT, &defense, 10, &cbdef);
			ret = SQLBindCol(hstmt, 10, SQL_C_SHORT, &visual, 10, &cbvisual);
			ret = SQLBindCol(hstmt, 11, SQL_C_BIT, &playing, 1, &cbd);

			for (int i = 0; ; i++) { 
				ret = SQLFetch(hstmt);
				if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) break;
			}
			if (playing)
			{
				packet_setup(LOGIN_FAIL);
			}
		}
		else
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
		}
		break;
	}
	case CS_MOVE:
	{
		CS_MOVE_PACKET* p = (CS_MOVE_PACKET*)packet;
		std::cout << "Received CS_MOVE packet: " << (int)p->direction << std::endl;
		break;
	}
	case CS_CHAT:
	{
		CS_CHAT_PACKET* p = (CS_CHAT_PACKET*)packet;
		std::cout << "Received CS_CHAT packet: " << p->mess << std::endl;
		break;
	}
	default:
		std::cout << "Unknown packet type: " << (int)type << std::endl;
		break;
	}
}

