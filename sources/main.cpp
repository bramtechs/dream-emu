#define WIDTH (640)
#define HEIGHT (480)
#include <iostream>

#include "raylib.h"
#include "raymath.h"
#include <cassert>

#include "tiles.cpp"
#include "game.cpp"
#include "levels.cpp"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(WIDTH, HEIGHT, "droomwereld");

    // Define the camera to look into our 3d world

    // Get map image data to be used for collision detection

    Vector3 mapPosition = {-16.0f, 0.0f, -8.0f};  // Set model position

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    GameLevel *level = game_level_load();
    level_init001(level);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        game_update_and_render();
    }

    level_dispose();
    game_dispose();
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
