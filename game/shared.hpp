#pragma once

#include <iostream>
#include <cassert>
#include <vector>

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

#ifndef RAYLIB_H
struct Color {
    unsigned char r;        // Color red value
    unsigned char g;        // Color green value
    unsigned char b;        // Color blue value
    unsigned char a;        // Color alpha value
};
#endif

struct Block {
    int id;
    Vector3 pos;
    bool isBlock;
};

struct Environment {
    Color skyColor;
};

struct LevelLayout {
    int width;
    int height;
    std::vector<Color> colors;
    std::vector<Color> paletteColors;
};

struct LevelFeed {
    Environment environment;
    std::vector<Block> blocks;
};