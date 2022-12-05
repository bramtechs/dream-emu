#pragma once

#include <iostream>
#include <cassert>

#include "mem_array.cpp"

#include "raylib.h"
#include "raymath.h"

#define WATER_DEPTH 0.3f

#define DIR_NORTH 0
#define DIR_EAST 1
#define DIR_SOUTH 2
#define DIR_WEST 3

#define TILE_NONE (-1)
#define TILE_FLOOR_GRASS 10
#define TILE_FLOOR_WATER 17
#define TILE_FLOOR_STONE 42

struct Block {
    int id;
    Vector3 pos;
    bool isBlock;
};

struct Environment {
    Color skyColor;
};

struct LevelLayout {
    Environment environment;
    MemoryArray<Block> blocks;
};