#define WIDTH 639
#define HEIGHT 480
#define SCALE 2

// TODO confusing filename

#include "shared.hpp"

// #include "session.cpp"
//
// #include "assets.cpp"
// #include "drawing.cpp"
// #include "gizmos.cpp"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(WIDTH * SCALE, HEIGHT * SCALE, "DREAM_EMU");
    SetWindowPosition((GetMonitorWidth(0) - WIDTH * SCALE) / 2, (GetMonitorHeight(0) - HEIGHT * SCALE) / 2);

    // Define the camera to look into our 3d world

    // Get map image data to be used for collision detection

    TraceLog(LOG_INFO, "Launched at %s", GetWorkingDirectory());

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    SetTargetFPS(60);

    // assets_load();
    // session_reset();

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        BeginDrawing();
        BeginTextureMode(target);

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
        DrawTexturePro(target.texture, {0.0f, 0.0f, WIDTH, -HEIGHT}, {0.0f, 0.0f, WIDTH * SCALE, HEIGHT * SCALE},
                       {0.0f, 0.0f}, 0.0f, WHITE);

        EndDrawing();
    }

    // TODO assets_dispose();
    UnloadRenderTexture(target);
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
