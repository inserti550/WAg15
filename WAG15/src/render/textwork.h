#pragma once
#include "../pch.h"

const BYTE symbol3x6_data[][3] = {
    {0x0E, 0x0E, 0x0E}, // stop
    {0x1F, 0x0E, 0x04}, // play
    {0x1E, 0x00, 0x1E}  // pause
};

class TextRender {
protected:
    std::wstring text;
    int posX, posY;
    int screenWidth, screenHeight;
    HFONT hFont;
    HDC memDC;
    HBITMAP memBitmap;
    void* pBits;
    int renderW, renderH;

    void CreateGDIResources(int fontSize, bool bold) {
        renderW = 1024;
        renderH = fontSize + 10;

        HDC hdc = GetDC(NULL);
        memDC = CreateCompatibleDC(hdc);

        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = renderW;
        bmi.bmiHeader.biHeight = -renderH;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        memBitmap = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
        SelectObject(memDC, memBitmap);

        hFont = CreateFontW(fontSize, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Small Fonts");

        SelectObject(memDC, hFont);
        SetTextColor(memDC, RGB(255, 255, 255));
        SetBkMode(memDC, TRANSPARENT);
        ReleaseDC(NULL, hdc);
    }

    void ClearDC() {
        RECT r = { 0, 0, renderW, renderH };
        SetBkColor(memDC, RGB(0, 0, 0));
        ExtTextOut(memDC, 0, 0, ETO_OPAQUE, &r, NULL, 0, NULL);
    }

public:
    TextRender(int w, int h, int fontSize = 13, bool bold = false) {
        screenWidth = w; screenHeight = h;
        posX = 0; posY = 0;
        CreateGDIResources(fontSize, bold);
    }

    virtual ~TextRender() {
        DeleteObject(hFont);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
    }

    virtual void SetText(const wchar_t* newText, int x = 0, int y = 0) {
        text = newText;
        posX = x;
        posY = y;
        RenderInternal();
    }

    void SetSymbol(int symbolIdx) {
        ClearDC();
        if (symbolIdx < 0 || symbolIdx > 2) return;
        for (int col = 0; col < 3; col++) {
            BYTE colData = symbol3x6_data[symbolIdx][col];
            for (int row = 0; row < 6; row++) {
                if (colData & (1 << row)) {
                    ((DWORD*)pBits)[row * renderW + col] = 0x00FFFFFF;
                }
            }
        }
    }

    int GetRealTextWidth() {
        RECT r = { 0, 0, 0, 0 };
        DrawTextW(memDC, text.c_str(), -1, &r, DT_CALCRECT | DT_NOPREFIX);
        return r.right;
    }

    void RenderInternal() {
        ClearDC();
        RECT r = { 0, 0, renderW, renderH };
        DrawTextW(memDC, text.c_str(), -1, &r, DT_LEFT | DT_TOP | DT_NOPREFIX);
    }

    void DrawRaw(unsigned char* buffer, int destX, int destY, bool colorMode, COLORREF color, int scrollOffset = 0) {
        int tW = GetRealTextWidth();
        unsigned char r_c = GetRValue(color);
        unsigned char g_c = GetGValue(color);
        unsigned char b_c = GetBValue(color);

        int limitX = (tW < screenWidth) ? tW : screenWidth;

        for (int y = 0; y < renderH; y++) {
            for (int x = 0; x < limitX; x++) {
                int srcX = x + scrollOffset;
                if (srcX < 0 || srcX >= renderW) continue;

                DWORD pixel = ((DWORD*)pBits)[y * renderW + srcX];
                if (pixel > 0) {
                    int dx = destX + x;
                    int dy = destY + y;

                    if (colorMode) {
                        if (dx >= 0 && dx < 320 && dy >= 0 && dy < 240) {
                            int off = (dy * 320 + dx) * 4;
                            buffer[off + 0] = b_c; buffer[off + 1] = g_c;
                            buffer[off + 2] = r_c; buffer[off + 3] = 255;
                        }
                    }
                    else {
                        if (dx >= 0 && dx < 160 && dy >= 0 && dy < 43) {
                            buffer[dy * 160 + dx] = 255;
                        }
                    }
                }
            }
        }
    }
};

class ScrollingText : public TextRender {
private:
    int sPos = 0;
    int textWidth = 0;
    int scrollSpeed;
    ULONGLONG lastUpd = 0;
    bool scrollflag = false;
    int pauseFrame = 0;
    bool movingLeft = true;

public:
    ScrollingText(int w, int h, int fontSize = 13, int speed = 50)
        : TextRender(w, h, fontSize, true) {
        scrollSpeed = speed;
    }

    void SetText(const wchar_t* newText, int x = 0, int y = 0) override {
        TextRender::SetText(newText, x, y);
        textWidth = GetRealTextWidth();
        scrollflag = (textWidth > screenWidth);
        sPos = 0;
        pauseFrame = 30;
        movingLeft = true;
    }

    void Update() {
        if (!scrollflag) return;
        ULONGLONG now = GetTickCount64();
        if (now - lastUpd < (ULONGLONG)scrollSpeed) return;
        lastUpd = now;

        if (pauseFrame > 0) { pauseFrame--; return; }

        if (movingLeft) {
            sPos++;
            if (sPos >= textWidth - screenWidth) {
                movingLeft = false;
                pauseFrame = 30;
            }
        }
        else {
            sPos--;
            if (sPos <= 0) {
                sPos = 0;
                movingLeft = true;
                pauseFrame = 30;
            }
        }
    }

    void Draw(unsigned char* buffer, bool colorMode, COLORREF color = RGB(0, 255, 0)) {
        if (!scrollflag) {
            int centeredX = posX + (screenWidth - textWidth) / 2;
            DrawRaw(buffer, centeredX, posY, colorMode, color, 0);
        }
        else {
            DrawRaw(buffer, posX, posY, colorMode, color, sPos);
        }
    }
};

class LoopedScrollingText : public TextRender {
private:
    int sPos = 0;
    int fullTextWidth = 0;
    int cleanTextWidth = 0;
    int scrollSpeed;
    ULONGLONG lastUpd = 0;
    std::wstring divider = L" *** ";
    bool isLooping = false;

public:
    LoopedScrollingText(int w, int h, int fontSize = 13, int speed = 40)
        : TextRender(w, h, fontSize, true) {
        scrollSpeed = speed;
    }

    void SetText(const wchar_t* newText, int x = 0, int y = 0) override {
        posX = x;
        posY = y;

        text = newText;
        RenderInternal();
        cleanTextWidth = GetRealTextWidth();

        if (cleanTextWidth > screenWidth) {
            text = std::wstring(newText) + divider;
            RenderInternal();
            fullTextWidth = GetRealTextWidth();
            isLooping = true;
        }
        else {
            isLooping = false;
        }

        sPos = 0;
    }

    void Update() {
        if (!isLooping) return;

        ULONGLONG now = GetTickCount64();
        if (now - lastUpd < (ULONGLONG)scrollSpeed) return;
        lastUpd = now;

        sPos++;
        if (sPos >= fullTextWidth) {
            sPos = 0;
        }
    }

    void DrawLoopedRaw(unsigned char* buffer, int drawX, int drawY, bool colorMode, COLORREF color, int offset) {
        unsigned char r_c = GetRValue(color);
        unsigned char g_c = GetGValue(color);
        unsigned char b_c = GetBValue(color);

        for (int y = 0; y < renderH; y++) {
            for (int x = 0; x < screenWidth; x++) {
                int srcX = x + offset;

                if (srcX >= fullTextWidth) srcX %= fullTextWidth;

                DWORD pixel = ((DWORD*)pBits)[y * renderW + srcX];
                if (pixel > 0) {
                    int dx = drawX + x;
                    int dy = drawY + y;

                    if (colorMode) {
                        if (dx >= 0 && dx < 320 && dy >= 0 && dy < 240) {
                            int off = (dy * 320 + dx) * 4;
                            buffer[off + 0] = b_c; buffer[off + 1] = g_c;
                            buffer[off + 2] = r_c; buffer[off + 3] = 255;
                        }
                    }
                    else {
                        if (dx >= 0 && dx < 160 && dy >= 0 && dy < 43) {
                            buffer[dy * 160 + dx] = 255;
                        }
                    }
                }
            }
        }
    }

    void Draw(unsigned char* buffer, bool colorMode, COLORREF color = RGB(0, 255, 0)) {
        if (!isLooping) {
            DrawRaw(buffer, posX, posY, colorMode, color, 0);
        }
        else {
            DrawLoopedRaw(buffer, posX, posY, colorMode, color, sPos);
        }
    }
};

class StaticText : public TextRender {
public:
    StaticText(int w, int h, int fontSize = 13, bool bold = false)
        : TextRender(w, h, fontSize, bold) {
    }

    void Draw(unsigned char* buffer, bool colorMode, COLORREF color = RGB(0, 255, 0)) {
        DrawRaw(buffer, posX, posY, colorMode, color, 0);
    }

    void DrawCenter(unsigned char* buffer, int centerX, int y, bool colorMode, COLORREF color = RGB(0, 255, 0)) {
        int w = GetRealTextWidth();
        DrawRaw(buffer, centerX - w / 2, y, colorMode, color, 0);
    }

    void DrawRight(unsigned char* buffer, int rightX, int y, bool colorMode, COLORREF color = RGB(0, 255, 0)) {
        int w = GetRealTextWidth();
        DrawRaw(buffer, rightX - w, y, colorMode, color, 0);
    }
};

class PlaylistRender : public TextRender {
private:
    int scrollOffset = 0;
    int selectedIdx = 0;
    int itemHeight = 15;
    int maxVisible = 8;
    int panelWidth = 299;

public:
    PlaylistRender(int w, int h, int fontSize = 14) : TextRender(w, h, fontSize, true) {
        maxVisible = h / itemHeight;
    }

    void DrawTimeRight(unsigned char* buffer, const wchar_t* timeStr, int y, COLORREF color) {
        int oldPosX = posX;
        int oldPosY = posY;

        this->SetText(timeStr, 0, 0);
        int tw = GetRealTextWidth();
        int drawX = (5 + panelWidth) - tw - 5;

        DrawRaw(buffer, drawX, y, true, color, 0);

        posX = oldPosX;
        posY = oldPosY;
    }

    void MoveSelection(int delta, int total) {
        selectedIdx += delta;
        if (selectedIdx < 0) selectedIdx = 0;
        if (selectedIdx >= total) selectedIdx = total - 1;

        scrollOffset = selectedIdx - (maxVisible / 2);

        if (scrollOffset < 0) scrollOffset = 0;
        if (scrollOffset > total - maxVisible) scrollOffset = total - maxVisible;
        if (total < maxVisible) scrollOffset = 0;
    }

    void Draw(unsigned char* buffer, bool isActiveMode) {
        int total = Winamp::GetPlaylistLength();
        int playingIdx = Winamp::GetListPosition();

        for (int i = 0; i < maxVisible; i++) {
            int currentTrack = i + scrollOffset;
            if (currentTrack >= total) break;

            wchar_t* name = Winamp::GetPlaylistTitleW(currentTrack);

            int durationMs = Winamp::GetTrackLengthMs(currentTrack);
            wchar_t timeBuf[16] = L"";
            if (durationMs > 0) {
                int totalSec = durationMs / 1000;
                swprintf_s(timeBuf, L"%d:%02d", totalSec / 60, totalSec % 60);
            }

            wchar_t line[512];
            swprintf_s(line, L"%d. %s", currentTrack + 1, name);

            int drawY = 105 + (i * itemHeight);
            int drawX = 6;

            COLORREF textColor = RGB(0, 255, 0);
            bool drawBg = false;

            if (isActiveMode && currentTrack == selectedIdx) {
                textColor = RGB(255, 255, 255);
                drawBg = true;
            }
            else if (currentTrack == playingIdx) {
                textColor = RGB(255, 255, 255);
            }

            if (drawBg) {
                for (int py = drawY; py < drawY + itemHeight; py++) {
                    for (int px = drawX; px < drawX + panelWidth; px++) {
                        if (px < 320 && py < 240) {
                            int off = (py * 320 + px) * 4;
                            buffer[off + 0] = 255; buffer[off + 1] = 0; buffer[off + 2] = 0; buffer[off + 3] = 255;
                        }
                    }
                }
            }

            this->SetText(line, drawX, drawY);
            DrawRaw(buffer, drawX, drawY, true, textColor, 0);

            if (durationMs > 0) {
                DrawTimeRight(buffer, timeBuf, drawY, textColor);
            }
        }
    }

    int GetSelected() { return selectedIdx; }
};