#pragma once

// dump all globals in here lol
#include "magma.h"
#include <utils.h>

#define PREFS_PATH "../save/engine_conf.dat"

struct UserPrefs {
    bool drawGrid;
    bool drawOutlines;
    bool freeCam;
    bool editorVisible;

    static void Save();
    static void Load();
};

extern UserPrefs Prefs;
