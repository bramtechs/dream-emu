#pragma once

// dump all globals in here lol
#include "magma.h"

typedef struct {
    bool drawGrid;
    bool drawOutlines;
    bool freeCam;
    bool editorVisible;
} UserPrefs;

extern UserPrefs Prefs;

void SaveUserPrefs();
void LoadUserPrefs();
