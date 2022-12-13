#pragma once

#include <stdio.h>

#include "raylib.h"
#include "raymath.h"

#include "raygui.h"

#include "logger.h"
#include "assets.h"
#include "entity.h"

extern size_t Allocations;

#define new(X) (X*)M_MemAlloc(sizeof(X))

void assert(bool cond);

void* M_MemAlloc(size_t size);
void* M_MemFree(void* ptr);

void CheckAllocations();
