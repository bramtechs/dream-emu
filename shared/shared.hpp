#pragma once

#include <iostream>
#include <cassert>

#include "mem_array.cpp"

#define IS_DEBUG true

#define WATER_DEPTH 0.3f

#define DIR_NORTH 0
#define DIR_EAST 1
#define DIR_SOUTH 2
#define DIR_WEST 3

#define TILE_NONE (-1)
#define TILE_FLOOR_GRASS 10
#define TILE_FLOOR_WATER 17
#define TILE_FLOOR_STONE 42

#include "raymath.h"

template<class T>
T *NN(T *ptr) // pointer passed was null
{
    if (ptr == nullptr && IS_DEBUG)
    {
        int *i = nullptr;
        *i = -1;
    }
    return ptr;
}

template<class T>
T POS(T val) // value passed was not positive
{
    if (val <= 0 && IS_DEBUG)
    {
        int *i = nullptr;
        *i = -1;
    }
    return val;
}

struct Lamp {
    Vector3 pos;
    Color color;
    float brightness;
    bool disabled;
};

struct Block {
    int id;
    Vector3 pos;
    bool isBlock;
};

struct Environment {
    Color skyColor;
    Color sunColor;
    float fogDensity;
    Vector3 sunDirection;
};

// TODO free memory!
struct LevelLayout {
    int width;
    int height;
    Color *colors;

    Color *paletteColors;
    int paletteColorCount;
};

struct LevelFeed {
    Environment environment;
    MemoryArray<Block> blocks;
    SmallMemoryArray<Lamp> lamps;
};