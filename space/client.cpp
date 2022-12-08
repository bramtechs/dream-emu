#define WIDTH 640
#define HEIGHT 480
#define SCALE 2

#include <iostream>
#include <cassert>
#include "mem_array.cpp"
#include "arena.cpp"

#include "raylib.h"
#include "raymath.h"

void client_update_and_render()
{
    ClearBackground(BLACK);
    DrawText("Hello world!", 50, 50, 16, WHITE);
}

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(WIDTH * SCALE, HEIGHT * SCALE, "SPACEGAME");
    SetWindowPosition((GetMonitorWidth(0) - WIDTH * SCALE) / 2, (GetMonitorHeight(0) - HEIGHT * SCALE) / 2);

    // Define the camera to look into our 3d world

    // Get map image data to be used for collision detection

    TraceLog(LOG_INFO, "Launched at %s", GetWorkingDirectory());

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
        BeginTextureMode(target);

        client_update_and_render();

        EndTextureMode();

        // TODO fix scuffed
        DrawTexturePro(target.texture, {0.0f, 0.0f, WIDTH, -HEIGHT}, {0.0f, 0.0f, WIDTH * SCALE, HEIGHT * SCALE},
                       {0.0f, 0.0f}, 0.0f, WHITE);

        EndDrawing();
    }

    // TODO assets_dispose();
    UnloadRenderTexture(target);
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
