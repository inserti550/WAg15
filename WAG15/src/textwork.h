#include <Windows.h>
#include <string.h>

extern const BYTE font5x7[][5];
extern const BYTE font3x6[][3];

enum FontType {
    FONT_5X7,
    FONT_3X6
};

class TextRender {
protected:
    char text[512];
    int posX, posY;
    FontType currentFont;
    int cWidth;
    int cHeight;
    int cSpace;

public:
    TextRender(FontType font = FONT_5X7) {
        posX = 0;
        posY = 0;
        memset(text, 0, sizeof(text));
        SetFont(font);
    }

    void SetFont(FontType font) {
        currentFont = font;
        if (font == FONT_5X7) {
            cWidth = 5; cHeight = 7; cSpace = 2;
        }
        else {
            cWidth = 3; cHeight = 6; cSpace = 2;
        }
    }

    virtual void SetText(const char* newText, int x = 0, int y = 0) {
        strcpy_s(text, sizeof(text), newText);
        posX = x;
        posY = y;
    }

    int GetTextWidth(const char* str = nullptr) {
        if (!str) str = text;
        size_t len = strlen(str);
        if (!len) return 0;
        return (int)len * (cWidth + cSpace) - cSpace;
    }

protected:
    void DrawString(BYTE* screen, int w, int h, int startX, int startY, const char* str) {
        int offset = 0;
        const unsigned char* s = (const unsigned char*)str;

        while (*s) {
            if (startX + offset + cWidth > 0 && startX + offset < w) {
                DrawChar(screen, w, h, startX + offset, startY, *s);
            }
            offset += cWidth + cSpace;
            s++;
        }
    }

    void DrawChar(BYTE* screen, int w, int h, int x, int y, unsigned char ch) {
        if (currentFont == FONT_5X7) {
            Draw5x7(screen, w, h, x, y, ch);
        }
        else {
            Draw3x6(screen, w, h, x, y, ch);
        }
    }

    inline void SetPixel(BYTE* screen, int w, int h, int x, int y) {
        if (x >= 0 && x < w && y >= 0 && y < h) {
            screen[y * w + x] = 255;
        }
    }

    void Draw5x7(BYTE* screen, int w, int h, int x, int y, unsigned char ch) {
        int idx = -1;
        if (ch >= 32 && ch <= 126) idx = ch - 32;
        else if (ch >= 0xC0 && ch <= 0xDF) idx = ch - 0xC0 + 95;
        else if (ch >= 0xE0 && ch <= 0xFF) idx = ch - 0xE0 + 127;
        else if (ch == 0xA8) idx = 99;
        else if (ch == 0xB8) idx = 131;

        if (idx < 0 || idx >= 159) return;

        for (int col = 0; col < 5; col++) {
            int drawX = x + col;
            if (drawX < 0 || drawX >= w) continue;

            BYTE pixels = font5x7[idx][col];
            for (int row = 0; row < 7; row++) {
                if (pixels & (1 << row)) {
                    SetPixel(screen, w, h, drawX, y + row);
                }
            }
        }
        if (ch == 0xA8 || ch == 0xB8) {
            SetPixel(screen, w, h, x + 1, y - 2);
            SetPixel(screen, w, h, x + 3, y - 2);
        }
    }

    void Draw3x6(BYTE* screen, int w, int h, int x, int y, unsigned char ch) {
        int idx = -1;
        if (ch >= 32 && ch <= 126) idx = ch - 32;
        if (idx < 0 || idx >= 95) return;

        for (int col = 0; col < 3; col++) {
            int drawX = x + col;
            if (drawX < 0 || drawX >= w) continue;

            BYTE pixels = font3x6[idx][col];
            for (int row = 0; row < 6; row++) {
                if (pixels & (1 << row)) {
                    SetPixel(screen, w, h, drawX, y + row);
                }
            }
        }
    }
};

class StaticText : public TextRender {
public:
    StaticText(FontType font = FONT_5X7) : TextRender(font) {}

    void Draw(BYTE* screen, int w, int h) {
        DrawString(screen, w, h, posX, posY, text);
    }
    void DrawCenter(BYTE* screen, int w, int h, int centerX, int y) {
        int width = GetTextWidth();
        int x = centerX - width / 2;
        DrawString(screen, w, h, x, y, text);
    }
    void DrawRight(BYTE* screen, int w, int h, int rightX, int y) {
        int width = GetTextWidth();
        int x = rightX - width;
        DrawString(screen, w, h, x, y, text);
    }
};

class ScrollingText : public TextRender {
private:
    int screenWidth;
    int sPos;
    int TextWidth;
    int scrollSpeed;
    ULONGLONG lastUpd;
    bool scrollflag;
    int pauseFrame;
    int startPause;
    int endPause;
    bool movingLeft;

public:
    ScrollingText(int width = 160, int speed = 50, FontType font = FONT_5X7)
        : TextRender(font)
    {
        screenWidth = width;
        scrollSpeed = speed;
        sPos = 0;
        TextWidth = 0;
        lastUpd = 0;
        scrollflag = false;
        pauseFrame = 0;
        startPause = 30;
        endPause = 30;
        movingLeft = true;
    }
    void SetText(const char* newText, int x = 0, int y = 0) override {
        TextRender::SetText(newText, x, y);

        TextWidth = GetTextWidth();
        scrollflag = (TextWidth > screenWidth);

        sPos = 0;
        pauseFrame = startPause;
        movingLeft = true;
    }

    void Update() {
        if (!scrollflag) return;

        ULONGLONG now = GetTickCount64();
        if (now - lastUpd < (ULONGLONG)scrollSpeed) return;

        lastUpd = now;

        if (pauseFrame > 0) {
            pauseFrame--;
            return;
        }

        if (movingLeft) {
            sPos++;
            if (sPos >= TextWidth - screenWidth) {
                movingLeft = false;
                pauseFrame = endPause;
            }
        }
        else {
            sPos--;
            if (sPos <= 0) {
                sPos = 0;
                movingLeft = true;
                pauseFrame = startPause;
            }
        }
    }

    void Draw(BYTE* screen, int w, int h) {
        if (!scrollflag) {
            DrawString(screen, w, h, posX + (screenWidth - TextWidth) / 2, posY, text);
        }
        else {
            DrawString(screen, w, h, posX - sPos, posY, text);
        }
    }
};