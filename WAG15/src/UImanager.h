#pragma once
#include "pch.h"
#include "render/render.h"
#include "render/textwork.h"
#include "utils.h"

class UImanager {
private:
	struct UIGlobals
	{
		int PressTime[4] = { 0, 0, 0, 0 };
		bool PressEx[4] = { false, false, false, false };
		float g_peaks[38] = { 0 };
		float g_peakSpeed[38] = { 0 };
	};
	ScrollingText trackTitle;
	LoopedScrollingText trackTitleColor;
	PlaylistRender playlist;
	StaticText ElapsedTime;
	StaticText GenTime;
	StaticText SymState;
	StaticText KHZ;
	StaticText KBPS;

    wchar_t lastTrack[256] = L"";
public:

	enum class UIGroup {
		AUDIO_PANEL,
		SEEK_PANEL,
		TRANSPORT,
		PLAYLIST,
		COUNT
	};

	enum class UIElement {
		VOLUME, PAN,
		SEEK,
		PREV, PLAY, PAUSE, STOP, NEXT, SHUFFLE, REPEAT,
		PL_LIST
	};

	UIGroup currentGroup = UIGroup::AUDIO_PANEL;
	int elementIdx = 0;
	bool editMode = false;
	int currentFocus = 0;

	UIGlobals global;

	UImanager() :
		trackTitle(160, 15, 13), trackTitleColor(187, 14, 12),
		playlist(300, 127, 13), ElapsedTime(60, 12, 11),
		GenTime(60, 12, 11), SymState(20, 12, 11),
		KHZ(13, 10, 11), KBPS(18, 10, 11)
	{

	}

	void ControllHandler()
	{
        if (settings.cfg.type & LOGI_LCD_TYPE_MONO) {
            int buttons[] = { LOGI_LCD_MONO_BUTTON_0, LOGI_LCD_MONO_BUTTON_1, LOGI_LCD_MONO_BUTTON_2, LOGI_LCD_MONO_BUTTON_3 };
            for (int i = 0; i < 4; ++i) {
                bool isPressed = LogiLcdIsButtonPressed(buttons[i]);

                if (isPressed) {
                    if (this->global.PressTime[i] == 0) {
                        this->global.PressTime[i] = GetTickCount64();
                        this->global.PressEx[i] = false;
                    }
                    else if (!this->global.PressEx[i]) {
                        if (GetTickCount64() - this->global.PressTime[i] > settings.cfg.PressThreshold) {
                            utils::CommandProccess(settings.cfg.buttonaction[i + 4]);
                            this->global.PressEx[i] = true;
                        }
                    }
                }
                else if (this->global.PressTime[i] != 0) {
                    if (!this->global.PressEx[i]) {
                        utils::CommandProccess(settings.cfg.buttonaction[i]);
                    }
                    this->global.PressTime[i] = 0;
                    this->global.PressEx[i] = false;
                }
            }
        }

        if (settings.cfg.type & LOGI_LCD_TYPE_COLOR) {
            static bool lastUp = 0, lastDown = 0, lastLeft = 0, lastRight = 0, lastOk = 0, lastCancel = 0, lastMenu = 0;
            bool cUp = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_UP);
            bool cDown = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_DOWN);
            bool cLeft = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_LEFT);
            bool cRight = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_RIGHT);
            bool cOk = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_OK);
            bool cCancel = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_CANCEL);
            bool cMenu = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_MENU);

            if (cMenu && !lastMenu) {
                this->editMode = false;
                this->currentGroup = (UImanager::UIGroup)(((int)this->currentGroup + 1) % (int)UImanager::UIGroup::COUNT);
                this->elementIdx = 0;
            }

            if (!this->editMode) {
                if (cRight && !lastRight) this->elementIdx++;
                if (cLeft && !lastLeft) this->elementIdx--;
                if (cDown && !lastDown) {
                    this->currentGroup = (UImanager::UIGroup)(((int)this->currentGroup + 1) % (int)UImanager::UIGroup::COUNT);
                    this->elementIdx = 0;
                }
                if (cUp && !lastUp) {
                    this->currentGroup = (UImanager::UIGroup)(((int)this->currentGroup - 1 + (int)UImanager::UIGroup::COUNT) % (int)UImanager::UIGroup::COUNT);
                    this->elementIdx = 0;
                }

                if (this->currentGroup == UImanager::UIGroup::AUDIO_PANEL) this->elementIdx = (this->elementIdx + 2) % 2;
                if (this->currentGroup == UImanager::UIGroup::TRANSPORT) this->elementIdx = (this->elementIdx + 7) % 7;
                if (this->currentGroup == UImanager::UIGroup::SEEK_PANEL) this->elementIdx = 0;
                if (this->currentGroup == UImanager::UIGroup::PLAYLIST) this->elementIdx = 0;

                if (cOk && !lastOk) {
                    if (this->currentGroup == UImanager::UIGroup::AUDIO_PANEL || currentGroup == UImanager::UIGroup::SEEK_PANEL || currentGroup == UImanager::UIGroup::PLAYLIST) {
                        this->editMode = true;
                    }
                    else if (currentGroup == UImanager::UIGroup::TRANSPORT) {
                        int actions[] = { 13, 3, 2, 4, 1, 10, 11 };
                        utils::CommandProccess(actions[this->elementIdx]);
                    }
                }
            }
            else {
                if (cCancel && !lastCancel) this->editMode = false;

                if (this->currentGroup == UImanager::UIGroup::AUDIO_PANEL) {
                    int dir = cRight ? 5 : (cLeft ? -5 : 0);
                    if (this->elementIdx == 0) Winamp::SetVolume(Winamp::GetVolume() + dir);
                    else Winamp::setPan(Winamp::getPan() + (dir * 2));
                }
                else if (this->currentGroup == UImanager::UIGroup::SEEK_PANEL) {
                    if (cRight) Winamp::JumpToTime(Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackElapsed) + 5000);
                    if (cLeft)  Winamp::JumpToTime(Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackElapsed) - 5000);
                }
                else if (this->currentGroup == UImanager::UIGroup::PLAYLIST) {
                    static DWORD plHoldStart = 0;
                    static DWORD plLastRepeat = 0;
                    static int   plHoldDir = 0;

                    int dir = cDown ? 1 : (cUp ? -1 : 0);

                    if (dir != 0) {
                        DWORD now = (DWORD)GetTickCount64();
                        if (plHoldDir != dir) {
                            if ((dir == 1 && !lastDown) || (dir == -1 && !lastUp)) {
                                this->playlist.MoveSelection(dir, Winamp::GetPlaylistLength());
                            }
                            plHoldStart = now;
                            plLastRepeat = now;
                            plHoldDir = dir;
                        }
                        else {
                            DWORD held = now - plHoldStart;
                            if (held > 500) {
                                DWORD interval = (held > 1500) ? 50 : 100;
                                if (now - plLastRepeat >= interval) {
                                    this->playlist.MoveSelection(dir, Winamp::GetPlaylistLength());
                                    plLastRepeat = now;
                                }
                            }
                        }
                    }
                    else {
                        plHoldDir = 0;
                    }

                    if (cOk && !lastOk) {
                        SendMessage(plugin.hwndParent, WM_WA_IPC, this->playlist.GetSelected(), 121);
                    }
                }
                if (cOk && !lastOk && this->currentGroup != UImanager::UIGroup::PLAYLIST) this->editMode = false;
            }

            lastUp = cUp; lastDown = cDown; lastLeft = cLeft; lastRight = cRight; lastOk = cOk; lastCancel = cCancel; lastMenu = cMenu;
        }
	}

    void UpdateTrackName(const wchar_t* Trackname, int TrackID, int m, int s) {
        if (wcscmp(Trackname, lastTrack) != 0) {
            trackTitle.SetText(Trackname, 0, 2);

            wchar_t fullTitle[512];
            swprintf_s(fullTitle, L"%d. %s (%d:%02d)", TrackID + 1, Trackname, m, s);
            trackTitleColor.SetText(fullTitle, 113, 9);

            wcscpy_s(lastTrack, 256, Trackname);
        }
    }

    void DrawUI() {
        if ((settings.cfg.type & LOGI_LCD_TYPE_MONO) && settings.cfg.enableMono) {

            int progress = 0;
            if (Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackLengthms) > 0) {
                progress = (Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackElapsed) * 100) / Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackLengthms);
            }
            trackTitle.Update();

            memset(lcdBuffer, 0, sizeof(lcdBuffer));

            render::DrawTimeBar(lcdBuffer, progress);

            trackTitle.Draw(lcdBuffer, false);

            wchar_t bufElap[16], bufGen[16];
            swprintf_s(bufElap, L"%d:%02d", (Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackElapsed) / 1000) / 60, (Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackElapsed) / 1000) % 60);
            swprintf_s(bufGen, L"%d:%02d", (Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackLengthms) / 1000) / 60, (Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackLengthms) / 1000) % 60);

            ElapsedTime.SetText(bufElap, 1, 28);
            ElapsedTime.Draw(lcdBuffer, false);

            GenTime.SetText(bufGen, 158, 28);
            GenTime.DrawRight(lcdBuffer, 158, 28, false);

            switch (Winamp::IsPlay()) {
            case 1:  SymState.SetSymbol(1); break; // Play
            case 3:  SymState.SetSymbol(2); break; // Pause
            default: SymState.SetSymbol(0); break; // Stop
            }
            SymState.DrawCenter(lcdBuffer, 80, 28, false);

            LogiLcdMonoSetBackground(lcdBuffer);
        }
        if ((settings.cfg.type & LOGI_LCD_TYPE_COLOR) && settings.cfg.enableColor) {
            memset(colorBuffer, 250, 320 * 240 * 4);
            render::LoadLcdBackground(colorBuffer, IDB_BITMAP1, 0, 0, 320, 0);

            int volume = Winamp::GetVolume();
            int cellIndex = (volume * 27) / 255;
            int volindex = (volume * 54) / 255;

            int rawPan = Winamp::getPan();
            int panIndex = (abs(rawPan) * 27) / 127;
            int normPan = rawPan + 127;
            int panSliderX = (normPan * 24) / 254;

            int vSrcY = 20 + (cellIndex * 15);
            render::DrawSprite(colorBuffer, IDB_BITMAP2, 0, vSrcY, 68, vSrcY + 14, 109, 39);

            int pSrcY = 25 + (panIndex * 15);
            render::DrawSprite(colorBuffer, IDB_BITMAP2, 69, pSrcY, 107, pSrcY + 14, 180, 39);

            if (editMode && currentGroup == UIGroup::AUDIO_PANEL && elementIdx == 0)
                render::DrawSprite(colorBuffer, IDB_BITMAP2, 146, 13, 159, 23, 109 + volindex, 40);
            else
                render::DrawSprite(colorBuffer, IDB_BITMAP2, 131, 13, 145, 24, 109 + volindex, 40);

            if (editMode && currentGroup == UIGroup::AUDIO_PANEL && elementIdx == 1)
                render::DrawSprite(colorBuffer, IDB_BITMAP2, 146, 13, 159, 23, 180 + panSliderX, 40); // Инверсия
            else
                render::DrawSprite(colorBuffer, IDB_BITMAP2, 131, 13, 145, 24, 180 + panSliderX, 40); // Обычный

            long currentTime = Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackElapsed);
            long totalTime = Winamp::GetTrackTime(Winamp::OUTPUTTIME::TrackLengthms);
            if (totalTime > 0 && Winamp::IsPlay() != 0) {
                int trackSliderX = (int)(((long long)currentTime * 262) / totalTime);
                if (trackSliderX > 262) trackSliderX = 262;
                if (trackSliderX < 0) trackSliderX = 0;

                if (editMode && currentGroup == UIGroup::SEEK_PANEL)
                    render::DrawSprite(colorBuffer, IDB_BITMAP2, 131, 25, 159, 34, 14 + trackSliderX, 55); // Инверсия
                else
                    render::DrawSprite(colorBuffer, IDB_BITMAP2, 69, 14, 98, 24, 14 + trackSliderX, 55); // Обычный
            }

            wchar_t bitrateBuf[32];
            swprintf_s(bitrateBuf, L"%d", Winamp::GetBitrate());
            KBPS.SetText(bitrateBuf, 110, 25);
            KBPS.Draw(colorBuffer, true, RGB(0, 255, 0));

            swprintf_s(bitrateBuf, L"%d", Winamp::GetSampleRate());
            KHZ.SetText(bitrateBuf, 155, 25);
            KHZ.Draw(colorBuffer, true, RGB(0, 255, 0));

            if (Winamp::GetShuffle()) render::DrawSprite(colorBuffer, IDB_BITMAP2, 55, 0, 99, 12, 232, 75);
            if (Winamp::GetRepeate()) render::DrawSprite(colorBuffer, IDB_BITMAP2, 27, 0, 53, 12, 278, 75);

            render::DrawSprite(colorBuffer, IDB_BITMAP2, 108, 13, 129, 29, 14, 75);  // Prev
            render::DrawSprite(colorBuffer, IDB_BITMAP2, 108, 32, 129, 48, 38, 75);  // Play
            render::DrawSprite(colorBuffer, IDB_BITMAP2, 108, 50, 129, 66, 62, 75);  // Pause
            render::DrawSprite(colorBuffer, IDB_BITMAP2, 108, 68, 129, 84, 86, 75);  // Stop
            render::DrawSprite(colorBuffer, IDB_BITMAP2, 108, 86, 129, 102, 110, 75); // Next

            int chans = Winamp::GetChannels();
            if (chans == 1)      render::DrawSprite(colorBuffer, IDB_BITMAP2, 132, 0, 160, 11, 205, 26);
            else if (chans == 2) render::DrawSprite(colorBuffer, IDB_BITMAP2, 102, 0, 130, 11, 238, 26);

            int playStatus = Winamp::IsPlay();
            if (playStatus != 0) {
                int ms = Winamp::GetTrackTime(Winamp::TrackElapsed);
                if (ms < 0) ms = 0;
                int totalSec = ms / 1000;
                int hours = totalSec / 3600;
                int minutes = (totalSec / 60) % 60;
                int seconds = totalSec % 60;
                int h_digit = (hours > 0) ? (hours % 10) : 10;
                render::DrawSprite(colorBuffer, IDB_BITMAP3, h_digit * 9, 0, (h_digit * 9) + 9, 13, 39, 10);
                render::DrawSprite(colorBuffer, IDB_BITMAP3, (minutes / 10) * 9, 0, ((minutes / 10) * 9) + 9, 13, 51, 10);
                render::DrawSprite(colorBuffer, IDB_BITMAP3, (minutes % 10) * 9, 0, ((minutes % 10) * 9) + 9, 13, 63, 10);
                render::DrawSprite(colorBuffer, IDB_BITMAP3, (seconds / 10) * 9, 0, ((seconds / 10) * 9) + 9, 13, 81, 10);
                render::DrawSprite(colorBuffer, IDB_BITMAP3, (seconds % 10) * 9, 0, ((seconds % 10) * 9) + 9, 13, 93, 10);
            }

            switch (playStatus)
            {
            case 1:
                render::DrawSprite(colorBuffer, IDB_BITMAP2, 8, 10, 17, 19, 25, 12);
                render::DrawSprite(colorBuffer, IDB_BITMAP2, 19, 0, 22, 3, 21, 12);
                render::DrawSprite(colorBuffer, IDB_BITMAP2, 19, 4, 21, 6, 21, 18);
                break;
            case 3:
                render::DrawSprite(colorBuffer, IDB_BITMAP2, 9, 0, 18, 9, 25, 12);
                break;
            default:
                render::DrawSprite(colorBuffer, IDB_BITMAP2, 17, 10, 26, 19, 25, 12);
                break;
            }

            trackTitleColor.Update();
            trackTitleColor.Draw(colorBuffer, true, RGB(0, 255, 0));

            playlist.Draw(colorBuffer, (currentGroup == UIGroup::PLAYLIST));

            if (Winamp::export_sa_get) {
                char data[158];
                char* p = Winamp::export_sa_get(data);
                if (p) {
                    int startX = 27, bottomY = 42, maxHeight = 17, topLimit = 25, numBars = 38;
                    for (int i = 0; i < numBars; i++) {
                        int val = (unsigned char)p[i * 2];
                        int barHeight = (val * maxHeight) / 31;
                        if (barHeight > maxHeight) barHeight = maxHeight;
                        float curT = (float)(bottomY - barHeight);
                        if (this->global.g_peaks[i] < 1.0f || this->global.g_peaks[i] > (float)bottomY) this->global.g_peaks[i] = (float)bottomY;
                        if (curT <= this->global.g_peaks[i]) { this->global.g_peaks[i] = curT; this->global.g_peakSpeed[i] = 0.1f; }
                        else { this->global.g_peaks[i] += this->global.g_peakSpeed[i]; this->global.g_peakSpeed[i] += 0.2f; }
                        int x = startX + (i * 2);
                        for (int h = 0; h < barHeight; h++) {
                            int y = bottomY - h; if (y < topLimit) break;
                            int off = (y * 320 + x) * 4;
                            int r = (h <= 3) ? 0 : (h <= 8 ? (int)(255 * (h - 4) / 5.0f) : 255);
                            int g = (h <= 8) ? 255 : (int)(255 * (1.0f - (float)(h - 9) / (maxHeight - 9)));
                            colorBuffer[off] = 0; colorBuffer[off + 1] = (unsigned char)g;
                            colorBuffer[off + 2] = (unsigned char)r; colorBuffer[off + 3] = 255;
                        }
                        int pY = (int)this->global.g_peaks[i];
                        if (pY < bottomY && pY >= topLimit) {
                            int po = (pY * 320 + x) * 4;
                            colorBuffer[po] = 220; colorBuffer[po + 1] = 220; colorBuffer[po + 2] = 220; colorBuffer[po + 3] = 255;
                        }
                    }
                }
            }

            struct Rect { int x, y, w, h; };
            Rect audioRects[] = { {108, 39, 67, 13}, {179, 39, 39, 13} };
            Rect transRects[] = { {14,75,22,16}, {38,75,22,16}, {62,75,22,16}, {86,75,22,16}, {110,75,22,16}, {232,75,45,13}, {278,75,27,13} };

            Rect f = { 0, 0, 0, 0 };
            if (currentGroup == UIGroup::AUDIO_PANEL) f = audioRects[elementIdx % 2];
            else if (currentGroup == UIGroup::TRANSPORT) f = transRects[elementIdx % 7];
            else if (currentGroup == UIGroup::SEEK_PANEL) f = { 14, 55, 292, 11 };
            else if (currentGroup == UIGroup::PLAYLIST) f = { 5, 103, 300, 127 };

            if (f.w > 0) {
                COLORREF frameColor = editMode ? RGB(255, 0, 0) : RGB(255, 255, 0);
                render::DrawFocusRect(colorBuffer, f.x, f.y, f.w, f.h, frameColor);
            }

            LogiLcdColorSetBackground(colorBuffer);
        }
    }
};