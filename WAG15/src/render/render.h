#pragma once
#include "../pch.h"

namespace render{
    void Preview(HWND hwndDlg, BYTE* buffer);

    void DrawTimeBar(BYTE* buffer, int progress);

    void SetPixelColor(unsigned char* buffer, int x, int y, BYTE r, BYTE g, BYTE b, BYTE a = 255);

    void DrawFocusRect(unsigned char* buffer, int x, int y, int w, int h, COLORREF color);

    void LoadLcdBackground(unsigned char* buffer, int resId, int destX = 0, int destY = 0, int targetW = 0, int targetH = 0);

    void DrawSprite(unsigned char* buffer, int resId, int srcX0, int srcY0, int srcX1, int srcY1, int destX3, int destY3);
}