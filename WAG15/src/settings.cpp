#include "settings.h"

const wchar_t* BIND_ACTIONS[] = {
    L"None", L"Next Track", L"Previous Track", L"Play/Pause", L"Stop",
    L"Fast-forward 5s", L"Rewind 5s", L"Jump to Start", L"Jump to End",
    L"Toggle Repeat", L"Toggle Shuffle", L"Volume Up", L"Volume Down",
    L"Set Volume 0%", L"Set Volume 25%", L"Set Volume 50%", L"Set Volume 75%", L"Set Volume 100%"
};
int COMBO_IDS[] = { actionbutton0, actionbutton1, actionbutton2, actionbutton3, actionbutton4, actionbutton5, actionbutton6, actionbutton7 };