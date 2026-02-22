#include "render.h"

namespace render {
    void Preview(HWND hwndDlg, BYTE *buffer) {
      HWND preview = GetDlgItem(hwndDlg, lcdpreview);
      if (!preview)
        return;

      RECT rect;
      GetClientRect(preview, &rect);
      int W = rect.right - rect.left;
      int H = rect.bottom - rect.top;
      int dib[160 * 43];
      for (int i = 0; i < 160 * 43; ++i) {
          BYTE px = 255 - buffer[i];
          dib[i] = RGB(px, px, px);
      }

      BITMAPINFO bmi = {0};
      bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth = 160;
      bmi.bmiHeader.biHeight = -43;
      bmi.bmiHeader.biPlanes = 1;
      bmi.bmiHeader.biBitCount = 32;
      bmi.bmiHeader.biCompression = BI_RGB;

      HDC dc = GetDC(preview);

      SetStretchBltMode(dc, COLORONCOLOR);

      StretchDIBits(dc, 0, 0, W, H, 0, 0, 160, 43, dib, &bmi, DIB_RGB_COLORS,
                    SRCCOPY);

      ReleaseDC(preview, dc);
    }

    void DrawTimeBar(BYTE *buffer, int progress) {
      if (progress < 0)
        progress = 0;
      if (progress > 100)
        progress = 100;

      if (!settings.cfg.filledbar) {
        for (int y = 38; y <= 42; y++) {
          for (int x = 0; x < 160; x++) {
            int pIdx = y * 160 + x;
            if (y == 38 || y == 42 || x == 0 || x == 159) {
              buffer[pIdx] = logion;
            } else {
              if (x <= (progress * 158) / 100 && y >= 39 && y <= 41 &&
                  ((x - 1) % 4 < 2)) {
                buffer[pIdx] = logion;
              } else {
                buffer[pIdx] = logioff;
              }
            }
          }
        }
      } else {
        for (int y = 38; y <= 42; y++) {
          for (int x = 0; x < 160; x++) {
            int pIdx = y * 160 + x;

            buffer[pIdx] = logion;
            if (y >= 39 && y <= 41 && x > 0 && x <= (progress * 158) / 100) {

              if ((x - 1) % 4 < 2) {
                buffer[pIdx] = logioff;
              }
            }
          }
        }
      }
    }

    void DrawFocusRect(unsigned char *buffer, int x, int y, int w, int h,
                       COLORREF color) {
      unsigned char r = GetRValue(color);
      unsigned char g = GetGValue(color);
      unsigned char b = GetBValue(color);

      for (int i = 0; i < w; i++) {
        int off1 = (y * 320 + (x + i)) * 4;
        int off2 = ((y + h - 1) * 320 + (x + i)) * 4;
        buffer[off1] = b;
        buffer[off1 + 1] = g;
        buffer[off1 + 2] = r;
        buffer[off1 + 3] = 255;
        buffer[off2] = b;
        buffer[off2 + 1] = g;
        buffer[off2 + 2] = r;
        buffer[off2 + 3] = 255;
      }
      for (int i = 0; i < h; i++) {
        int off1 = ((y + i) * 320 + x) * 4;
        int off2 = ((y + i) * 320 + (x + w - 1)) * 4;
        buffer[off1] = b;
        buffer[off1 + 1] = g;
        buffer[off1 + 2] = r;
        buffer[off1 + 3] = 255;
        buffer[off2] = b;
        buffer[off2 + 1] = g;
        buffer[off2 + 2] = r;
        buffer[off2 + 3] = 255;
      }
    }

    void LoadLcdBackground(unsigned char *buffer, int resId, int destX, int destY,
                           int targetW, int targetH) {
      HBITMAP hBmp = (HBITMAP)LoadImage(plugin.hDllInstance, MAKEINTRESOURCE(resId),
                                        IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
      if (!hBmp)
        return;

      BITMAP bmp;
      GetObject(hBmp, sizeof(BITMAP), &bmp);

      int finalW = (targetW > 0) ? targetW : bmp.bmWidth;
      int finalH = (targetH > 0) ? targetH : bmp.bmHeight;

      HDC hdcScreen = GetDC(NULL);
      HDC hdcSrc = CreateCompatibleDC(hdcScreen);
      HDC hdcDest = CreateCompatibleDC(hdcScreen);

      HBITMAP hBmpResized = CreateCompatibleBitmap(hdcScreen, finalW, finalH);

      HGDIOBJ oldSrc = SelectObject(hdcSrc, hBmp);
      HGDIOBJ oldDest = SelectObject(hdcDest, hBmpResized);

      SetStretchBltMode(hdcDest, COLORONCOLOR);
      StretchBlt(hdcDest, 0, 0, finalW, finalH, hdcSrc, 0, 0, bmp.bmWidth,
                 bmp.bmHeight, SRCCOPY);

      std::vector<BYTE> spriteData(finalW * finalH * 4);
      BITMAPINFO bmi = {0};
      bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth = finalW;
      bmi.bmiHeader.biHeight = -finalH;
      bmi.bmiHeader.biPlanes = 1;
      bmi.bmiHeader.biBitCount = 32;
      bmi.bmiHeader.biCompression = BI_RGB;

      GetDIBits(hdcDest, hBmpResized, 0, finalH, spriteData.data(), &bmi,
                DIB_RGB_COLORS);

      for (int y = 0; y < finalH; y++) {
        for (int x = 0; x < finalW; x++) {
          int curX = destX + x;
          int curY = destY + y;

          if (curX >= 0 && curX < 320 && curY >= 0 && curY < 240) {
            int destIdx = (curY * 320 + curX) * 4;
            int srcIdx = (y * finalW + x) * 4;

            buffer[destIdx] = spriteData[srcIdx];
            buffer[destIdx + 1] = spriteData[srcIdx + 1];
            buffer[destIdx + 2] = spriteData[srcIdx + 2];
            buffer[destIdx + 3] = 255;
          }
        }
      }

      SelectObject(hdcSrc, oldSrc);
      SelectObject(hdcDest, oldDest);
      DeleteObject(hBmpResized);
      DeleteObject(hBmp);
      DeleteDC(hdcSrc);
      DeleteDC(hdcDest);
      ReleaseDC(NULL, hdcScreen);
    }

    void DrawSprite(unsigned char *buffer, int resId, int srcX0, int srcY0,
                    int srcX1, int srcY1, int destX3, int destY3) {
      HBITMAP hBmp = (HBITMAP)LoadImage(plugin.hDllInstance, MAKEINTRESOURCE(resId),
                                        IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
      if (!hBmp)
        return;

      BITMAP bmp;
      GetObject(hBmp, sizeof(BITMAP), &bmp);

      int partW = srcX1 - srcX0;
      int partH = srcY1 - srcY0;

      HDC hdc = CreateCompatibleDC(NULL);
      HGDIOBJ oldBmp = SelectObject(hdc, hBmp);

      std::vector<BYTE> fullSheet(bmp.bmWidth * bmp.bmHeight * 4);
      BITMAPINFO bmi = {0};
      bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth = bmp.bmWidth;
      bmi.bmiHeader.biHeight = -bmp.bmHeight;
      bmi.bmiHeader.biPlanes = 1;
      bmi.bmiHeader.biBitCount = 32;
      bmi.bmiHeader.biCompression = BI_RGB;

      GetDIBits(hdc, hBmp, 0, bmp.bmHeight, fullSheet.data(), &bmi, DIB_RGB_COLORS);

      for (int y = 0; y < partH; y++) {
        for (int x = 0; x < partW; x++) {
          int sX = srcX0 + x;
          int sY = srcY0 + y;

          int dX = destX3 + x;
          int dY = destY3 + y;

          if (sX >= 0 && sX < bmp.bmWidth && sY >= 0 && sY < bmp.bmHeight &&
              dX >= 0 && dX < 320 && dY >= 0 && dY < 240) {

            int srcIdx = (sY * bmp.bmWidth + sX) * 4;
            int destIdx = (dY * 320 + dX) * 4;

            buffer[destIdx] = fullSheet[srcIdx];
            buffer[destIdx + 1] = fullSheet[srcIdx + 1];
            buffer[destIdx + 2] = fullSheet[srcIdx + 2];
            buffer[destIdx + 3] = 255;
          }
        }
      }

      SelectObject(hdc, oldBmp);
      DeleteDC(hdc);
      DeleteObject(hBmp);
    }
}