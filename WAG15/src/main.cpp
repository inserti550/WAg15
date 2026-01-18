#include "utils.h"
#pragma comment(lib, "LogitechLCDLib.lib")
#include "../lib/LogitechLCDLib.h" 

int init();
void config();
void quit();

winampGeneralPurposePlugin plugin = {
    GPPHDR_VER,
    (char *)"G15 plugin",
    init,
    config,
    quit,
};

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllexport) winampGeneralPurposePlugin* winampGetGeneralPurposePlugin() {
        return &plugin;
    }

#ifdef __cplusplus
}
#endif

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