#pragma once

#define SETTINGS_FILE "save/settings.ini"

// dump all globals in here lol
#include "magma.h"
#include "inih/ini.h"

#define STRING(X) strcpy(X)

typedef struct {
    #define CFG(s, n, default) char* s##_##n;
    #include "settings.def"
} UserSettings;

extern UserSettings Settings;

bool BOOL(char* ptr);
void SetBoolean(char* ptr, bool enabled);
void ToggleBoolean(char* ptr);

void SaveUserSettings();
void LoadUserSettings();
