#pragma once

#include "raylib.h"
#include "raymath.h"
#include "logger.h"
#include "assets.h"
#include "entity.h"

extern size_t Allocations;

#define constructor(X) X* _init_##X(X* X)
#define destructor(X) X* _destroy_##X(X* X)

#define new(X) _init_##X((X*)M_MemAlloc(sizeof(X)))
#define delete(X,Y) M_MemFree(_destroy_##X((X*)Y))

void* M_MemAlloc(size_t size);
void* M_MemFree(void* ptr);

void CheckAllocations();
