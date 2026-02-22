#pragma once
#include "../lib/gen.h"
#include "../lib/wa_ipc.h"
#include "../lib/winamp.h"
#include <string>
#include <windows.h>
#include "../resource.h"


extern winampGeneralPurposePlugin plugin;

extern const wchar_t *BIND_ACTIONS[18];
extern int COMBO_IDS[8];

class SettingsSystem {
private:
  struct Settings {
    int PressThreshold = 250;
    int buttonaction[8] = {2, 1, 12, 11, 3, 14, 15, 16};
    int volumestep = 5;
    int textspeed = 100;
    int sOffset = 0;
    bool scroll = true;
    int ScrTime = 0;
    bool filledbar = true;
    bool enableMono = true;
    bool enableColor = true;
    // 1 = mono 2 = color 3 = both
    BYTE type = 0;
  };

public:
  Settings cfg;

  std::wstring GetConfig() {
    char *inipatch =
        (char *)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETINIFILE);
    std::string path = inipatch;
    path = path.substr(0, path.find_last_of("\\/") + 1) + "g15cp.ini";
    int size = MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, NULL, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, &wstr[0], size);
    // C:\Users\user\AppData\Roaming\Winamp\g15cp.ini
    return wstr;
  }

  void SaveConfig() {
    std::wstring path = this->GetConfig();

    for (int i = 0; i < 8; ++i) {
      std::wstring key = L"ButtonAction" + std::to_wstring(i);
      std::wstring val = std::to_wstring(this->cfg.buttonaction[i]);

      WritePrivateProfileStringW(L"Settings", key.c_str(), val.c_str(),
                                 path.c_str());
    }
    WritePrivateProfileStringW(L"Settings", L"VolumeStep",
                               std::to_wstring(this->cfg.volumestep).c_str(),
                               path.c_str());
    WritePrivateProfileStringW(
        L"Settings", L"timeThreshold",
        std::to_wstring(this->cfg.PressThreshold).c_str(), path.c_str());
    WritePrivateProfileStringW(L"Settings", L"filledbar",
                               std::to_wstring(this->cfg.filledbar).c_str(),
                               path.c_str());
  }

  void LoadConfig() {
    std::wstring path = this->GetConfig();

    for (int i = 0; i < 8; ++i) {
      std::wstring key = L"ButtonAction" + std::to_wstring(i);
      this->cfg.buttonaction[i] = GetPrivateProfileIntW(
          L"Settings", key.c_str(), this->cfg.buttonaction[i], path.c_str());
    }
    this->cfg.volumestep =
        GetPrivateProfileIntW(L"Settings", L"VolumeStep", 5, path.c_str());
    this->cfg.PressThreshold =
        GetPrivateProfileIntW(L"Settings", L"timeThreshold", 250, path.c_str());
    this->cfg.filledbar =
        GetPrivateProfileIntW(L"Settings", L"filledbar", true, path.c_str());
  }

  SettingsSystem() {}
};