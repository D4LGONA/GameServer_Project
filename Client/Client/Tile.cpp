#include "stdafx.h"
#include "Tile.h"

void Tile::render(HDC dc, vector<Image*>& vec, int padx, int pady)
{
    RECT rc;
    rc.left = (x - padx) * 50;
    rc.top = (y - pady) * 50;
    rc.right = (x + 1 - padx) * 50; // x ��ǥ�� 50�� ����
    rc.bottom = (y + 1 - pady) * 50; // y ��ǥ�� 50�� ����

    vec[type]->img.StretchBlt(dc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0, 0, vec[type]->img.GetWidth(), vec[type]->img.GetHeight(), SRCCOPY);

    //int centerX = (rc.left + rc.right) / 2;
    //int centerY = (rc.top + rc.bottom) / 2;

    //// �ؽ�Ʈ ����� ��ġ ���
    //SIZE textSize;
    //TCHAR text[20];
    //_stprintf_s(text, _T("(%d, %d)"), x, y);
    //GetTextExtentPoint32(dc, text, _tcslen(text), &textSize); // �ؽ�Ʈ�� ũ�� ���
    //int textX = (rc.left + rc.right - textSize.cx) / 2; // �ؽ�Ʈ�� x ��ǥ ���
    //int textY = (rc.top + rc.bottom - textSize.cy) / 2; // �ؽ�Ʈ�� y ��ǥ ���

    //// �ؽ�Ʈ ���
    //SetTextColor(dc, RGB(0, 0, 0)); // �ؽ�Ʈ ���� ���� (������)
    //SetBkMode(dc, TRANSPARENT); // ��� �����ϰ� ����
    //TextOut(dc, textX, textY, text, _tcslen(text));

    //if (can_move)
    //    FrameRect(dc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
    //else
    //    FrameRect(dc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
}
