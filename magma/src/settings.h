#pragma once

// dump all globals in here lol
#include "magma.h"
#include <sys/stat.h>
#include <sys/types.h>

#define MAGMA_CONF_PATH "../save/engine_conf.dat"

typedef struct {
    bool unlockFrameRate;
    bool skipIntro;
} MagmaSettings;

extern MagmaSettings Settings;

void SaveMagmaSettings();
void LoadMagmaSettings();
