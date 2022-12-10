#define WIDTH 639
#define HEIGHT 480
#define SCALE 2

#include "raylib.h"

#include <stdio.h>
#include "logger.c"
#include "assets.c"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(WIDTH * SCALE, HEIGHT * SCALE, "DREAM_EMU");
    SetWindowPosition((GetMonitorWidth(0) - WIDTH * SCALE) / 2, (GetMonitorHeight(0) - HEIGHT * SCALE) / 2);

    // Define the camera to look into our 3d world

    // Get map image data to be used for collision detection

    INFO("Launched at %s", GetWorkingDirectory());

    assets_load("assets");

    Texture texture = assets_texture("palette");
    for (int i = 0; i < 100; i++){
        Texture texture2 = assets_texture("palette");
    }

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    SetTargetFPS(60);

    // session_reset();

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        BeginDrawing();
        BeginTextureMode(target);

        DrawTexture(texture,10,10,WHITE);

        //        UpdateCamera(&CurrentSession->camera);
        //
        //        if (IsKeyPressed(KEY_F3))
        //        {
        //            bool *flying = &CurrentSession->isFlying;
        //            *flying = !*flying;
        //            if (*flying)
        //            {
        //                SetCameraMode(CurrentSession->camera, CAMERA_FREE);     // Set camera mode
        //            } else
        //            {
        //                SetCameraMode(CurrentSession->camera, CAMERA_FIRST_PERSON);     // Set camera mode
        //            }
        //        }
        //
        //        BeginMode3D(CurrentSession->camera);
        //
        //        //assert(level_update_and_stream != nullptr);
        //
        //        level_update_and_stream(GetFrameTime());
        //        session_update_and_render(GetFrameTime());
        //
        //        EndMode3D();
        //
        //        DrawFPS(10, 10);

        EndTextureMode();

        // TODO fix scuffed
        DrawTexturePro(target.texture, (Rectangle) {0.0f, 0.0f, WIDTH, -HEIGHT},
                       (Rectangle) {0.0f, 0.0f, WIDTH * SCALE, HEIGHT * SCALE},
                       (Vector2) {0.0f, 0.0f}, 0.0f, WHITE);

        EndDrawing();
    }

    assets_dispose();
    UnloadRenderTexture(target);
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
