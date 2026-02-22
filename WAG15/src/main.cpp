#ifdef _WIN64
#pragma comment(lib, "LogitechLCDLib64.lib")
#else
#pragma comment(lib, "LogitechLCDLib.lib")
#endif
#include "mainloop.h"
#include "pch.h"
#include "settings.h"
#include "utils.h"


unsigned char lcdBuffer[160 * 43];
unsigned char colorBuffer[320 * 240 * 4];

SettingsSystem settings;

int init() {
#ifdef _DEBUG
  AllocConsole();
  FILE *f;
  freopen_s(&f, "CONOUT$", "w", stdout);
  freopen_s(&f, "CONOUT$", "w", stderr);
  freopen_s(&f, "CONIN$", "r", stdin);
  SetConsoleTitleW(L"WAg15 Debug Console");
#endif
  std::cout << "If you see this console, it means that you have installed the "
               "\"Debug\" version of WAg15."
            << std::endl
            << "If you do not want to see it, install the DLL without the "
               "\"Debug\" prefix. You can download it from Github."
            << std::endl
            << "https://github.com/inserti550/WAg15/releases";
  Winamp::InitSA();

  bool initRes = LogiLcdInit((wchar_t *)L"WINAMP",
                             LOGI_LCD_TYPE_MONO | LOGI_LCD_TYPE_COLOR);
  if (!initRes)
    return GEN_INIT_FAILURE;

  settings.LoadConfig();

  settings.cfg.type = 0;
  if (LogiLcdIsConnected(LOGI_LCD_TYPE_MONO))
    settings.cfg.type |= LOGI_LCD_TYPE_MONO;
  if (LogiLcdIsConnected(LOGI_LCD_TYPE_COLOR))
    settings.cfg.type |= LOGI_LCD_TYPE_COLOR;

  if (settings.cfg.type == 0)
    return GEN_INIT_FAILURE;

  SetTimer(plugin.hwndParent, LOOPTIMERID, 100, (TIMERPROC)MainLoop);
  return GEN_INIT_SUCCESS;
}

void config() {
  DialogBox(plugin.hDllInstance, MAKEINTRESOURCE(IDD_DIALOG1),
            plugin.hwndParent, utils::SettingsDlg);
}

void quit() {
  KillTimer(plugin.hwndParent, LOOPTIMERID);
  LogiLcdShutdown();
#ifdef _DEBUG
  FreeConsole();
#endif
}

winampGeneralPurposePlugin plugin = {
    GPPHDR_VER, (char *)"WAg15", init, config, quit, 0, 0};

extern "C" {
__declspec(dllexport) winampGeneralPurposePlugin *
winampGetGeneralPurposePlugin() {
  return &plugin;
}
}