#pragma once

// dump all globals in here lol
#include "magma.h"

typedef struct {
    bool unlockFrameRate;
    bool freeCam;
    bool drawGrid;
    bool drawOutlines;
    bool editorVisible;
} UserSettings;

extern UserSettings Settings;
