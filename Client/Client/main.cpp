#include "stdafx.h"
#include "TileMapEditor.h"
#include "PlayScene.h"

vector<Scene*> Scenes;

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"windows program 1";
bool lbuttondown = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lparam);

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hinstance;

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
		Scenes.emplace_back(new PlayScene());
		Scene::setSelected(0);
		Scene::setIsChanged(false);
		SetTimer(hwnd, 1, timercnt, NULL);
		break;

	case WM_TIMER:
		switch (wParam)
		{
		case 1:
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

