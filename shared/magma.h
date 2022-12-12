#pragma once

#include "raylib.h"
#include "raymath.h"
#include "logger.h"
#include "assets.h"
#include "entity.h"

extern size_t Allocations;

#define NEW(X,Y) (X*) Y((X*)M_MemAlloc(sizeof(X)))

void* M_MemAlloc(size_t size);

void CheckAllocations();
