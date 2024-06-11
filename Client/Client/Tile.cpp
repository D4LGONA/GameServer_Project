#include "stdafx.h"
#include "Tile.h"

void Tile::render(HDC dc, vector<Image*>& vec, int padx, int pady)
{
    RECT rc;
    rc.left = (x - padx) * 50;
    rc.top = (y - pady) * 50;
    rc.right = (x + 1 - padx) * 50; // x 좌표에 50을 더함
    rc.bottom = (y + 1 - pady) * 50; // y 좌표에 50을 더함

    vec[type]->img.StretchBlt(dc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0, 0, vec[type]->img.GetWidth(), vec[type]->img.GetHeight(), SRCCOPY);

    //int centerX = (rc.left + rc.right) / 2;
    //int centerY = (rc.top + rc.bottom) / 2;

    //// 텍스트 출력할 위치 계산
    //SIZE textSize;
    //TCHAR text[20];
    //_stprintf_s(text, _T("(%d, %d)"), x, y);
    //GetTextExtentPoint32(dc, text, _tcslen(text), &textSize); // 텍스트의 크기 계산
    //int textX = (rc.left + rc.right - textSize.cx) / 2; // 텍스트의 x 좌표 계산
    //int textY = (rc.top + rc.bottom - textSize.cy) / 2; // 텍스트의 y 좌표 계산

    //// 텍스트 출력
    //SetTextColor(dc, RGB(0, 0, 0)); // 텍스트 색상 설정 (검정색)
    //SetBkMode(dc, TRANSPARENT); // 배경 투명하게 설정
    //TextOut(dc, textX, textY, text, _tcslen(text));

    //if (can_move)
    //    FrameRect(dc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
    //else
    //    FrameRect(dc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
}
