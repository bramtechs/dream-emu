#pragma once

// dump all globals in here lol
#include "magma.h"

typedef struct {
    bool unlockFrameRate;
    bool skipIntro;
} MagmaSettings;

extern MagmaSettings Settings;

void SaveMagmaSettings();
void LoadMagmaSettings();
