#include "utils.h"

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
        int next = cur + settings.cfg.volumestep;
        Winamp::SetVolume(next > 255 ? 255 : next);
        break;
    }
    case 12: {
        int cur = Winamp::GetVolume();

        int next = cur - settings.cfg.volumestep;
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

INT_PTR CALLBACK utils::SettingsDlg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    HWND volumeSlider = GetDlgItem(hwnd, volumestepslider);
    HWND timeSlider = GetDlgItem(hwnd, presstimerscroll);
    HWND filledcheck = GetDlgItem(hwnd, filledBar);
    std::wstring volumeText;
    std::wstring timeText;
    switch (message) {
    case WM_INITDIALOG:
        for (int c = 0; c < 8; ++c) {
            HWND Combo = GetDlgItem(hwnd, COMBO_IDS[c]);
            SendMessage(Combo, CB_RESETCONTENT, 0, 0);
            for (int i = 0; i < sizeof(BIND_ACTIONS) / sizeof(BIND_ACTIONS[0]); ++i) {
                SendMessage(Combo, CB_ADDSTRING, 0, (LPARAM)BIND_ACTIONS[i]);
            }
            SendMessage(Combo, CB_SETCURSEL, settings.cfg.buttonaction[c], 0);
        }
        if (volumeSlider) {
            SendMessage(volumeSlider, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 255));
            SendMessage(volumeSlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)settings.cfg.volumestep);
            volumeText = std::to_wstring((int)(100 * (settings.cfg.volumestep / 255.0) + 0.5)) + L" %";
            SetDlgItemText(hwnd, IDC_VOLUME, volumeText.c_str());
        }
        if (timeSlider)
        {
            SendMessage(timeSlider, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(100, 1500));
            SendMessage(timeSlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)settings.cfg.PressThreshold);
            settings.cfg.PressThreshold = (int)SendMessage(timeSlider, TBM_GETPOS, 0, 0);
            timeText = std::to_wstring(settings.cfg.PressThreshold) + L" ms";
            SetDlgItemText(hwnd, IDC_TIMEF, timeText.c_str());
        }
        if (filledcheck) {
            SendMessage(filledcheck, BM_SETCHECK, settings.cfg.filledbar ? BST_CHECKED : BST_UNCHECKED, 0);
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
                settings.cfg.buttonaction[0] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton1:
                settings.cfg.buttonaction[1] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton2:
                settings.cfg.buttonaction[2] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton3:
                settings.cfg.buttonaction[3] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton4:
                settings.cfg.buttonaction[4] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton5:
                settings.cfg.buttonaction[5] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton6:
                settings.cfg.buttonaction[6] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            case actionbutton7:
                settings.cfg.buttonaction[7] = SendMessage(Combo, CB_GETCURSEL, 0, 0);
                break;
            default:
                break;
            }

        }
        if (LOWORD(wParam) == filledBar) {
            HWND checkbox = (HWND)lParam;
            settings.cfg.filledbar = (SendMessage(checkbox, BM_GETCHECK, 0, 0) == BST_CHECKED);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDOK) {

            settings.SaveConfig();
            EndDialog(hwnd, IDOK);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            settings.LoadConfig();
            EndDialog(hwnd, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCLOSE) {
            EndDialog(hwnd, IDCLOSE);
            return (INT_PTR)TRUE;
        }

        break;
    case WM_HSCROLL:
        settings.cfg.volumestep = (int)SendMessage(volumeSlider, TBM_GETPOS, 0, 0);
        volumeText = std::to_wstring((int)(100 * (settings.cfg.volumestep / 255.0))) + L" %";
        SetDlgItemText(hwnd, IDC_VOLUME, volumeText.c_str());

        settings.cfg.PressThreshold = (int)SendMessage(timeSlider, TBM_GETPOS, 0, 0);
        timeText = std::to_wstring(settings.cfg.PressThreshold) + L" ms";
        SetDlgItemText(hwnd, IDC_TIMEF, timeText.c_str());
        break;
    case WM_TIMER:
        if (wParam == 101) {
            render::Preview(hwnd, lcdBuffer);
        }
        break;
    case WM_DESTROY:
        KillTimer(hwnd, 101);
        break;
    }
    return (INT_PTR)FALSE;

}