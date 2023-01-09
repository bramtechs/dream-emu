#pragma once

// dump all globals in here lol
#include "magma.h"

typedef struct {
    bool unlockFrameRate;
    bool freeCam;
    bool drawGrid;
    bool drawOutlines;
    bool editorVisible;
    bool skipIntro;
} UserPrefs;

extern UserPrefs Prefs;

void SaveUserPrefs();
void LoadUserPrefs();
