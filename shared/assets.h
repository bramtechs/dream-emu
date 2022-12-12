#pragma once
#include "magma.h"

#define MAX_ASSETS 128

typedef struct {
    void* assets[MAX_ASSETS];
    int count;
} Assets;

Assets* Assets_INIT(Assets* ptr);


Assets* assets_load();

void assets_dispose(Assets* assets);
