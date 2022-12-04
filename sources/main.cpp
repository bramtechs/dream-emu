#define WIDTH 640
#define HEIGHT 480
#define SCALE 2

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
    SetConfigFlags(FLAG_WINDOW_TOPMOST);
    InitWindow(WIDTH * SCALE, HEIGHT * SCALE, "droomwereld");
    SetWindowPosition((GetMonitorWidth(0) - WIDTH * SCALE) / 2, (GetMonitorHeight(0) - HEIGHT * SCALE) / 2);

    // Define the camera to look into our 3d world

    // Get map image data to be used for collision detection

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    Vector3 mapPosition = {-16.0f, 0.0f, -8.0f};  // Set model position

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    GameLevel *level = game_level_load();
    level_init001(level);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
        BeginTextureMode(target);

        game_update_and_render();

        EndTextureMode();

        // TODO fix scuffed
        DrawTexturePro(target.texture, {0.0f, 0.0f, WIDTH, -HEIGHT}, {0.0f, 0.0f, WIDTH * SCALE, HEIGHT * SCALE},
                       {0.0f, 0.0f}, 0.0f, WHITE);

        EndDrawing();
    }

    UnloadRenderTexture(target);
    level_dispose();
    game_dispose();
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
