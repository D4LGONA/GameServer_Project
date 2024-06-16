#include "stdafx.h"

array<array<bool, W_WIDTH>, W_HEIGHT> map;

int setid()
{
    return id++;
}

int setid_npc()
{
    return npcid++;
}

std::wstring strtowstr(const std::string& str)
{
    if (str.empty()) return std::wstring();

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

void server_error(const char* msg)
{
    printf("%s with error: %d\n", msg, WSAGetLastError());
    exit(1);
}

bool can_move(int x, int y)
{
    if (x < 0 or y < 0 or x >= W_WIDTH or y >= W_HEIGHT) return false;
    if (map[x][y]) return true;
    return false;
}

