#pragma once
#define WIN32_LEAN_AND_MEAN
#include "../lib/LogitechLCDLib.h"
#include "../lib/gen.h"
#include "../lib/wa_ipc.h"
#include "../resource.h"
#include <commctrl.h>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

extern winampGeneralPurposePlugin plugin;

#include "settings.h"

#pragma comment(lib, "comctl32.lib")

#define LOOPTIMERID 5219
#define MAX_CHARS 27
#define logioff 0x00
#define logion 0xFF

extern unsigned char lcdBuffer[160 * 43];
extern unsigned char colorBuffer[320 * 240 * 4];
extern SettingsSystem settings;