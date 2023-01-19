#pragma once

#include "raylib.h"
#include "raymath.h"
#include "logger.h"
#include "window.h"
#include "assets.h"

#include <vector>

#define MAX_SPLASHES 8
#define FADE_DURATION 1

struct SplashScreen{
    const char imgPath[128];
    float duration;
};

struct MainMenuConfig {
    int width;
    int height;

    std::vector<SplashScreen> splashes;

    const char bgPath[128]; // background image or shader!
    const char title[128];
};

void BootMainMenu(MainMenuConfig config, bool skipSplash);
bool UpdateAndDrawMainMenu(float delta); // returns 'true' if done
