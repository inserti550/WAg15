#pragma once
#include "pch.h"
#include "render/render.h"
#include "render/textwork.h"
#include "UImanager.h"
#include "utils.h"

UImanager mainui;

void CALLBACK MainLoop(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    

    /*
    ** if (LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_0)) CommandProccess(settings::buttonaction[0]);
    ** if (LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_1)) CommandProccess(settings::buttonaction[1]);
    ** if (LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_2)) CommandProccess(settings::buttonaction[2]);
    ** if (LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_3)) CommandProccess(settings::buttonaction[3]);
    */

    wchar_t TrackID = Winamp::GetListPosition();
    wchar_t* Trackname = Winamp::GetPlaylistTitleW(TrackID);
    int TracLength = Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackLengthms);
    int m = (TracLength / 1000) / 60;
    int s = (TracLength / 1000) % 60;
    mainui.UpdateTrackName(Trackname, TrackID, m, s);
    mainui.ControllHandler();
    mainui.DrawUI();
    LogiLcdUpdate();
}
