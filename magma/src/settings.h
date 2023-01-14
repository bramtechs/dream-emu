#pragma once

// dump all globals in here lol
#include "magma.h"
#include <sys/stat.h>
#include <sys/types.h>

#define MAGMA_CONF_PATH "../save/engine_conf.ini"

typedef struct {
    bool unlockFrameRate;
    bool skipIntro;
} MagmaSettings;

extern MagmaSettings Settings;

void SaveMagmaSettings();
void LoadMagmaSettings();

void ClearConfig(const char* file);

inline void PutBool(const char* file, const char* name, bool on);
void PutField(const char* file, const char* name, const char* value);
