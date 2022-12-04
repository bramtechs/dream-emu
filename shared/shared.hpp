#pragma once

#include <iostream>
#include <cassert>

#include "mem_array.cpp"

#include "raylib.h"
#include "raymath.h"

#define TILE_FLOOR_GRASS 10
#define TILE_FLOOR_WATER 17

struct Block {
    int id;
    Vector3 pos;
};

struct Environment {
    Color skyColor;
};

struct LevelLayout {
    Environment environment;
    MemoryArray<Block> blocks;
};