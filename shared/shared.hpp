#pragma once

#include "mem_array.cpp"

#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <cassert>

typedef int TextureID;

struct Block {
    Vector3 pos;
    TextureID texture;
};

struct Environment {
    Color skyColor;
};

struct LevelLayout {
    Environment environment;
    MemoryArray<Block> blocks;
};