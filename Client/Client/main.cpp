#include "stdafx.h"
#include "TileMapEditor.h"
#include "PlayScene.h"
#include <iostream>
#include <string>

using namespace std;

vector<Scene*> Scenes;

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"windows program 1";
bool lbuttondown = false;
string userId;

SOCKET g_socket; // 서버와의 연결을 위한 소켓
const char* SERVER_IP = "127.0.0.1"; // 서버 IP 주소

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lparam);

bool InitWinSock()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        MessageBox(NULL, L"WinSock initialization failed", L"Error", MB_OK);
        return false;
    }
    return true;
}

void CleanupWinSock()
{
    WSACleanup();
}

bool ConnectToServer()
{
    g_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (g_socket == INVALID_SOCKET) {
        MessageBox(NULL, L"Socket creation failed", L"Error", MB_OK);
        return false;
    }

    // 소켓을 비동기 모드로 설정
    u_long mode = 1;
    ioctlsocket(g_socket, FIONBIO, &mode);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_NUM);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    int result = connect(g_socket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) { // WSAEWOULDBLOCK은 비동기 연결 시 정상적인 에러 코드
            MessageBox(NULL, L"Connection to server failed", L"Error", MB_OK);
            closesocket(g_socket);
            return false;
        }
    }

    return true;
}

void DisconnectFromServer()
{
    closesocket(g_socket);
}



int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG Message;
    WNDCLASSEX WndClass;
    g_hInst = hinstance;

    // 콘솔 창을 엽니다.
    AllocConsole();
    SetConsoleTitle(L"Client Console");

    // 콘솔의 표준 입출력 핸들을 얻습니다.
    FILE* pFile;
    freopen_s(&pFile, "CONOUT$", "w", stdout);
    freopen_s(&pFile, "CONIN$", "r", stdin);

    if (!InitWinSock()) {
        return -1;
    }

    if (!ConnectToServer()) {
        CleanupWinSock();
        return -1;
    }

    cout << "Enter your ID: ";
    cin >> userId;
    

    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hinstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = lpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassEx(&WndClass);

    hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, WIDTHMAX, HEIGHTMAX, NULL, (HMENU)NULL, hinstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, 0, 0, 0))
    {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    DisconnectFromServer();
    CleanupWinSock();
    return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc, mdc;
    HBITMAP hbitmap;
    static HBITMAP hBitmap;
    HBRUSH hbrush, oldbrush;
    HPEN hpen, oldpen;
    RECT rt;
    static POINT mousept;
    static int timercnt = 100;

    switch (iMsg)
    {
    case WM_CREATE:
        Scenes.emplace_back(new PlayScene(g_socket, userId.c_str()));
        Scene::setSelected(0);
        Scene::setIsChanged(false);
        SetTimer(hwnd, 1, timercnt, NULL);
        break;

    case WM_TIMER:
        switch (wParam)
        {
        case 1:
            Scenes[Scene::getSelected()]->update();
            break;
        }
        InvalidateRect(hwnd, NULL, false);
        break;

    case WM_MOUSEMOVE:
        if (lbuttondown) {
            mousept = { LOWORD(lParam), HIWORD(lParam) };
            Scenes[Scene::getSelected()]->LbuttonDown(mousept.x, mousept.y);
        }
        break;

    case WM_LBUTTONDOWN:
        lbuttondown = true;
        mousept = { LOWORD(lParam), HIWORD(lParam) };
        Scenes[Scene::getSelected()]->LbuttonDown(mousept.x, mousept.y);
        InvalidateRect(hwnd, NULL, false);
        break;

    case WM_LBUTTONUP:
        lbuttondown = false;
        break;

    case WM_KEYDOWN:
        Scenes[Scene::getSelected()]->keydown(wParam);
        InvalidateRect(hwnd, NULL, false);
        break;

    case WM_KEYUP:
        Scenes[Scene::getSelected()]->keyup(wParam);
        InvalidateRect(hwnd, NULL, false);
        break;

    case WM_PAINT:
        GetClientRect(hwnd, &rt);
        hdc = BeginPaint(hwnd, &ps);
        mdc = CreateCompatibleDC(hdc);
        hbitmap = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);
        SelectObject(mdc, (HBITMAP)hbitmap);
        hbrush = CreateSolidBrush(RGB(255, 255, 255));
        oldbrush = (HBRUSH)SelectObject(mdc, hbrush);
        Rectangle(mdc, 0, 0, rt.right, rt.bottom);
        SelectObject(mdc, oldbrush);
        DeleteObject(hbrush);
        // // // // // // // // // // // //

        Scenes[Scene::getSelected()]->render(mdc, hwnd);

        // // // // // // // // // //
        BitBlt(hdc, 0, 0, WIDTHMAX, HEIGHTMAX, mdc, 0, 0, SRCCOPY);

        DeleteDC(mdc);
        DeleteObject(hbitmap);

        EndPaint(hwnd, &ps);
        break;

    case WM_DESTROY:
        for (int i = 0; i < Scenes.size(); ++i)
            delete Scenes[i];
        Scenes.clear();
        KillTimer(hwnd, 1);
        DeleteObject(hBitmap);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
