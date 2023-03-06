#pragma once

#define IMPLEMENT_TEMPLE_LEVEL_DATA
#include "level_data.h"

typedef struct {
    uint score;
    uint time;
    uint crystals;
} LevelScores;

const char* MAP_SPRITE = "spr_map_placeholder";

bool UpdateAndDrawLevelSelect(float delta, LevelInfo* selectedLevel);

#ifdef IMPLEMENT_TEMPLE_LEVEL_SELECT

bool UpdateAndDrawLevelSelect(float delta, LevelInfo* selectedLevel){

    // draw main stuff
    BeginMagmaDrawing();

    ClearBackground(GOLD);

    // draw map
    Texture texture = RequestTexture(MAP_SPRITE);
    DrawTexture(texture, 0.f,0.f, WHITE);

    // draw mouse coords
    Vector2 mouse = GetWindowMousePosition();
    DrawRetroTextEx(TextFormat("x: %d\ny: %d",(int)mouse.x,(int)mouse.y),50,250,18,WHITE);

    EndMagmaDrawing();

    EndDrawing();

    *selectedLevel = GetAllLevels()[0];

    return false;
}

#endif
