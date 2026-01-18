#include "utils.h"
#ifdef _WIN64
#pragma comment(lib, "LogitechLCDLib64.lib")
#else
#pragma comment(lib, "LogitechLCDLib.lib")
#endif
#include "../lib/LogitechLCDLib.h" 

int init();
void config();
void quit();

winampGeneralPurposePlugin plugin = {
    GPPHDR_VER,
    (char *)"WAg15",
    init,
    config,
    quit,
    0,
    0
};

extern "C" {
    __declspec(dllexport) winampGeneralPurposePlugin* winampGetGeneralPurposePlugin() {
        return &plugin;
    }
}

int init() {

    LogiLcdInit((wchar_t*)L"WINAMP", LOGI_LCD_TYPE_MONO);
    settings::LoadConfig();
    SetTimer(plugin.hwndParent, LOOPTIMERID, 100, (TIMERPROC)utils::MainLoop);

    return GEN_INIT_SUCCESS;
}

void config() {
    DialogBox(plugin.hDllInstance, MAKEINTRESOURCE(IDD_DIALOG1), plugin.hwndParent, utils::SettingsDlg);
}

void quit() {
    KillTimer(plugin.hwndParent, LOOPTIMERID);
    LogiLcdShutdown();
}