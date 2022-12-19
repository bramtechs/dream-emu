#pragma once

// dump all globals in here lol
#include "magma.h"

typedef struct {
    bool unlockFrameRate;
    bool freeCam;
    bool drawGrid;
    bool drawOutlines;
} UserSettings;

extern UserSettings Settings;
