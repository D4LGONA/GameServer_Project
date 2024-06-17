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

POINT a_star_find_next_move(POINT start, POINT goal)
{
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;

    std::vector<std::vector<POINT>> came_from(W_WIDTH, std::vector<POINT>(W_HEIGHT));
    std::vector<std::vector<double>> g_score(W_WIDTH, std::vector<double>(W_HEIGHT, std::numeric_limits<double>::infinity()));
    std::vector<std::vector<double>> f_score(W_WIDTH, std::vector<double>(W_HEIGHT, std::numeric_limits<double>::infinity()));

    auto heuristic = [](const POINT& a, const POINT& b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
        };

    auto get_neighbors = [](const POINT& p) {
        std::vector<POINT> neighbors;
        std::vector<POINT> directions = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

        for (const POINT& d : directions) {
            POINT np = { p.x + d.x, p.y + d.y };
            if (np.x >= 0 && np.x < W_WIDTH && np.y >= 0 && np.y < W_HEIGHT && map[np.x][np.y]) {
                neighbors.push_back(np);
            }
        }

        return neighbors;
        };

    g_score[start.x][start.y] = 0;
    f_score[start.x][start.y] = heuristic(start, goal);
    open_set.push(Node{ start, f_score[start.x][start.y] });

    while (!open_set.empty()) {
        POINT current = open_set.top().POINT;
        open_set.pop();

        if (current.x == goal.x && current.y == goal.y) {
            std::vector<POINT> path;
            while (!(current.x == start.x && current.y == start.y)) {
                path.push_back(current);
                current = came_from[current.x][current.y];
            }
            std::reverse(path.begin(), path.end());
            return path.empty() ? start : path[0];
        }

        for (const POINT& neighbor : get_neighbors(current)) {
            double tentative_g_score = g_score[current.x][current.y] + 1;

            if (tentative_g_score < g_score[neighbor.x][neighbor.y]) {
                came_from[neighbor.x][neighbor.y] = current;
                g_score[neighbor.x][neighbor.y] = tentative_g_score;
                f_score[neighbor.x][neighbor.y] = g_score[neighbor.x][neighbor.y] + heuristic(neighbor, goal);
                open_set.push(Node{ neighbor, f_score[neighbor.x][neighbor.y] });
            }
        }
    }

    return start; // 경로를 찾지 못한 경우 시작점 반환
}