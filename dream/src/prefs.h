#pragma once

// dump all globals in here lol
#include "magma.h"

#define PREFS_PATH "../save/engine_conf.dat"

typedef struct {
    bool drawGrid;
    bool drawOutlines;
    bool freeCam;
    bool editorVisible;
} UserPrefs;

extern UserPrefs Prefs;

void SaveUserPrefs();
void LoadUserPrefs();
