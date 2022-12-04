#pragma once

#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <cassert>

struct Environment {
    Color skyColor;
};

struct LevelLayout {
    Environment environment;
};