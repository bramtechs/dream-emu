#pragma once
#include "magma.h"

#define MAX_ASSETS 128

typedef struct {
    void* assets[MAX_ASSETS];
    int count;
} Assets;

constructor(Assets);
destructor(Assets);
