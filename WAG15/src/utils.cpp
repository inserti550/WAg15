#include "utils.h"
#include "../lib/LogitechLCDLib.h"
#include <string>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#include "../resource.h"
#include <vector>
#include "textwork.h"

unsigned char settings::lcdBuffer[160 * 43];

std::wstring settings::GetConfig() {
    char* inipatch = (char*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETINIFILE);
    std::string path = inipatch;
    path = path.substr(0, path.find_last_of("\\/") + 1) + "g15cp.ini";
    int size = MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, NULL, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, &wstr[0], size);
    // C:\Users\user\AppData\Roaming\Winamp\g15cp.ini
    return wstr;
}

void settings::SaveConfig() {
    std::wstring path = settings::GetConfig();

    for (int i = 0; i < 8; ++i) {
        std::wstring key = L"ButtonAction" + std::to_wstring(i);    
        std::wstring val = std::to_wstring(settings::buttonaction[i]);

        WritePrivateProfileStringW(L"Settings", key.c_str(), val.c_str(), path.c_str());
    }
    WritePrivateProfileStringW(L"Settings", L"VolumeStep", std::to_wstring(settings::volumestep).c_str(), path.c_str());
    WritePrivateProfileStringW(L"Settings", L"timeThreshold", std::to_wstring(settings::PressThreshold).c_str(), path.c_str());
    WritePrivateProfileStringW(L"Settings", L"filledbar", std::to_wstring(settings::filledbar).c_str(), path.c_str());
}

void settings::LoadConfig() {
    std::wstring path = settings::GetConfig();

    for (int i = 0; i < 8; ++i) {
        std::wstring key = L"ButtonAction" + std::to_wstring(i);
        settings::buttonaction[i] = GetPrivateProfileIntW(L"Settings", key.c_str(), settings::buttonaction[i], path.c_str());
    }
    settings::volumestep = GetPrivateProfileIntW(L"Settings", L"VolumeStep", 5, path.c_str());
    settings::PressThreshold = GetPrivateProfileIntW(L"Settings", L"timeThreshold", 250, path.c_str());
    settings::filledbar = GetPrivateProfileIntW(L"Settings", L"filledbar", true, path.c_str());
}

void utils::Preview(HWND hwndDlg, BYTE* buffer) {
    HWND preview = GetDlgItem(hwndDlg, lcdpreview);
    if (!preview) return;

    RECT rect;
    GetClientRect(preview, &rect);
    int W = rect.right - rect.left;
    int H = rect.bottom - rect.top;

    HDC dc = GetDC(preview);
    HDC cDC = CreateCompatibleDC(dc);
    HBITMAP mem = CreateCompatibleBitmap(dc, 160, 43);
    SelectObject(cDC, mem);

    for (int y = 0; y < 43; y++) {
        for (int x = 0; x < 160; x++) {
            BYTE pixel = 255 - buffer[y * 160 + x];
            SetPixel(cDC, x, y, RGB(pixel, pixel, pixel));
        }
    }

    StretchBlt(dc, 0, 0, W, H, cDC, 0, 0, 160, 43, SRCCOPY);
    DeleteObject(mem);
    DeleteDC(cDC);
    ReleaseDC(preview, dc);
}

INT_PTR CALLBACK utils::SettingsDlg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    HWND volumeSlider = GetDlgItem(hwnd, volumestepslider);
    HWND timeSlider = GetDlgItem(hwnd, presstimerscroll);
    HWND filledcheck = GetDlgItem(hwnd, filledBar);
    std::wstring volumeText;
    std::wstring timeText;
    switch (message) {
    case WM_INITDIALOG:
        for (int c = 0; c < 8; ++c) {
            HWND Combo = GetDlgItem(hwnd, settings::dlgsett::combolist[c]);
            SendMessage(Combo, CB_RESETCONTENT, 0, 0);
            for (int i = 0; i < sizeof(settings::dlgsett::BIND_ACTIONS) / sizeof(settings::dlgsett::BIND_ACTIONS[0]); ++i) {
                SendMessage(Combo, CB_ADDSTRING, 0, (LPARAM)settings::dlgsett::BIND_ACTIONS[i]);
            }
            SendMessage(Combo, CB_SETCURSEL, settings::buttonaction[c], 0);
        }
        if (volumeSlider) {
            SendMessage(volumeSlider, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 255));
            SendMessage(volumeSlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)settings::volumestep);
            volumeText = std::to_wstring((int)(100 * (settings::volumestep / 255.0) + 0.5)) + L" %";
            SetDlgItemText(hwnd, IDC_VOLUME, volumeText.c_str());
        }
        if (timeSlider)
        {
            SendMessage(timeSlider, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(100, 1500));
            SendMessage(timeSlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)settings::PressThreshold);
            settings::PressThreshold = (int)SendMessage(timeSlider, TBM_GETPOS, 0, 0);
            timeText = std::to_wstring(settings::PressThreshold) + L" ms";
            SetDlgItemText(hwnd, IDC_TIMEF, timeText.c_str());
        }
        if (filledcheck) {
            SendMessage(filledcheck, BM_SETCHECK, settings::filledbar ? BST_CHECKED : BST_UNCHECKED, 0);
        }
        SetTimer(hwnd, 101, 100, NULL);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (HIWORD(wParam) == CBN_SELCHANGE) {
            HWND Combo = (HWND)lParam;
            int selIndex = (int)SendMessage(Combo, CB_GETCURSEL, 0, 0);
            switch (LOWORD(wParam))
            {
            case actionbutton0:
                settings::buttonaction[0] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton1:
                settings::buttonaction[1] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton2:
                settings::buttonaction[2] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton3:
                settings::buttonaction[3] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton4:
                settings::buttonaction[4] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton5:
                settings::buttonaction[5] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton6:
                settings::buttonaction[6] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton7:
                settings::buttonaction[7] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            default:
                break;
            }
            
        }
        if (LOWORD(wParam) == filledBar) {
            HWND checkbox = (HWND)lParam;
            settings::filledbar = (SendMessage(checkbox, BM_GETCHECK, 0, 0) == BST_CHECKED);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDOK) {

            settings::SaveConfig(); 
            EndDialog(hwnd, IDOK);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            settings::LoadConfig();
            EndDialog(hwnd, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCLOSE) {
            EndDialog(hwnd, IDCLOSE);
            return (INT_PTR)TRUE;
        }

        break;
    case WM_HSCROLL:
        settings::volumestep = (int)SendMessage(volumeSlider, TBM_GETPOS, 0, 0);
        volumeText = std::to_wstring((int)(100 * (settings::volumestep / 255.0))) + L" %";
        SetDlgItemText(hwnd, IDC_VOLUME, volumeText.c_str());

        settings::PressThreshold = (int)SendMessage(timeSlider, TBM_GETPOS, 0, 0);
        timeText = std::to_wstring(settings::PressThreshold) + L" ms";
        SetDlgItemText(hwnd, IDC_TIMEF, timeText.c_str());
        break;
    case WM_TIMER:
        if (wParam == 101) {
            utils::Preview(hwnd, settings::lcdBuffer);
        }
        break;
    case WM_DESTROY:
        KillTimer(hwnd, 101);
        break;
    }
    return (INT_PTR)FALSE;

}

using namespace utils;

void utils::CommandProccess(int com)
{
    if (com <= 0) return;
    switch (com)
    {
    case 1: Winamp::NextTrack(); break;
    case 2: Winamp::PreviousTrack(); break;
    case 3:
        (Winamp::IsPlay() == 1) ? Winamp::Pause() : Winamp::Play();
        break;
    case 4: Winamp::Stop(); break;
    case 5: Winamp::JumpToTime((int)Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackElapsed) + 5000); break;
    case 6: Winamp::JumpToTime((int)Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackElapsed) - 5000); break;
    case 7: Winamp::JumpToTime(0); break;
    case 8: Winamp::JumpToTime((int)Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackLengthms)); break;
    case 9: Winamp::SetRepeate(!Winamp::GetRepeate()); break;
    case 10: Winamp::SetShuffle(!Winamp::GetShuffle()); break;

    case 11: {
        int cur = Winamp::GetVolume();
        int next = cur + settings::volumestep;
        Winamp::SetVolume(next > 255 ? 255 : next);
        break;
    }
    case 12: {
        int cur = Winamp::GetVolume();

        int next = cur - settings::volumestep;
        Winamp::SetVolume(next < 0 ? 0 : next);
        break;
    }
    case 13: Winamp::SetVolume(0); break;
    case 14: Winamp::SetVolume(64); break;
    case 15: Winamp::SetVolume(127); break;
    case 16: Winamp::SetVolume(191); break;
    case 17: Winamp::SetVolume(255); break;
    default: break;
    }
}

inline const char* ToCp(const wchar_t* wstr) {
    static char buffer[1024];
    WideCharToMultiByte(1251, 0, wstr, -1, buffer, sizeof(buffer), NULL, NULL);
    return buffer;
}

void utils::DrawTimeBar(BYTE* buffer, int progress) {
    if (progress < 0) progress = 0;
    if (progress > 100) progress = 100;

    if (!settings::filledbar) {
        for (int y = 38; y <= 42; y++) {
            for (int x = 0; x < 160; x++) {
                int pIdx = y * 160 + x;
                if (y == 38 || y == 42 || x == 0 || x == 159) {
                    buffer[pIdx] = logion;
                }
                else {
                    if (x <= (progress * 158) / 100 && y >= 39 && y <= 41 && ((x - 1) % 4 < 2)) {
                        buffer[pIdx] = logion;
                    }
                    else {
                        buffer[pIdx] = logioff;
                    }
                }
            }
        }
    }
    else {
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

ScrollingText trackTitle(160, 150);
StaticText ElapsedTime(FontType::FONT_3X6);
StaticText GenTime(FontType::FONT_3X6);
StaticText SymState(FontType::SYMBOL_3X6);
static wchar_t lastTrack[256] = L"";

void CALLBACK utils::MainLoop(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    int buttons[] = { LOGI_LCD_MONO_BUTTON_0, LOGI_LCD_MONO_BUTTON_1, LOGI_LCD_MONO_BUTTON_2, LOGI_LCD_MONO_BUTTON_3 };
    for (int i = 0; i < 4; ++i) {
        bool isPressed = LogiLcdIsButtonPressed(buttons[i]);

        if (isPressed) {
            if (settings::PressTime[i] == 0) {
                settings::PressTime[i] = GetTickCount64();
                settings::PressEx[i] = false;
            }
            else if (!settings::PressEx[i]) {
                if (GetTickCount64() - settings::PressTime[i] > settings::PressThreshold) {
                    CommandProccess(settings::buttonaction[i + 4]);
                    settings::PressEx[i] = true;
                }
            }
        }
        else if (settings::PressTime[i] != 0) {
            if (!settings::PressEx[i]) {
                CommandProccess(settings::buttonaction[i]);
            }
            settings::PressTime[i] = 0;
            settings::PressEx[i] = false;
        }
    }
    
    /*
    ** if (LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_0)) CommandProccess(settings::buttonaction[0]);
    ** if (LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_1)) CommandProccess(settings::buttonaction[1]);
    ** if (LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_2)) CommandProccess(settings::buttonaction[2]);
    ** if (LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_3)) CommandProccess(settings::buttonaction[3]);
    */

    wchar_t TrackID = Winamp::GetListPosition();
    wchar_t *Trackname = Winamp::GetPlaylistTitleW(TrackID);
    int TrackElapsed = Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackElapsed);
    int TracLength = Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackLengthms);
    int TrackState = Winamp::IsPlay();
    int progress = 0;
    if (TracLength > 0) {
        progress = (TrackElapsed * 100) / TracLength;
    }

    if (wcscmp(Trackname, lastTrack) != 0) {
        trackTitle.SetText(ToCp(Trackname), 0, 5);
        wcscpy_s(lastTrack, 256, Trackname);
    }
    trackTitle.Update();

    memset(settings::lcdBuffer, 0, sizeof(settings::lcdBuffer));

    utils::DrawTimeBar(settings::lcdBuffer, progress);

    trackTitle.Draw(settings::lcdBuffer);

    char bufElap[16], bufGen[16];
    sprintf_s(bufElap, "%d:%02d", (TrackElapsed / 1000) / 60, (TrackElapsed / 1000) % 60);
    sprintf_s(bufGen, "%d:%02d", (TracLength / 1000) / 60, (TracLength / 1000) % 60);

    ElapsedTime.SetText(bufElap, 1, 30);
    ElapsedTime.Draw(settings::lcdBuffer); 

    GenTime.SetText(bufGen, 1, 30);
    GenTime.DrawRight(settings::lcdBuffer, 157, 30);

    switch (TrackState)
    {
    case 1:
        SymState.SetText("\x01",80,30);
        break;
    case 3:
        SymState.SetText("\x02", 80, 30);
        break;
    case 0:
        SymState.SetText("\x00", 80, 30);
        break;
    default:
        SymState.SetText("\x00", 80, 30);
        break;
    }

    SymState.DrawCenter(settings::lcdBuffer, 80, 30);

    LogiLcdMonoSetBackground(settings::lcdBuffer);
    LogiLcdUpdate();
}