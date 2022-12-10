#define WIDTH 639
#define HEIGHT 480
#define SCALE 2

#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION

#include "raygui.h"

#include <stdio.h>
#include "logger.c"
#include "assets.c"
#include "session.c"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(WIDTH * SCALE, HEIGHT * SCALE, "DREAM_EMU");
    SetWindowPosition((GetMonitorWidth(0) - WIDTH * SCALE) / 2, (GetMonitorHeight(0) - HEIGHT * SCALE) / 2);

    // Define the cam to look into our 3d world

    // Get map image data to be used for collision detection

    INFO("Launched at %s", GetWorkingDirectory());

    assets_load("assets");

    Texture texture = assets_texture("palette");
    for (int i = 0; i < 100; i++)
    {
        Texture texture2 = assets_texture("palette");
    }

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    SetTargetFPS(60);

    // session_reset();

    Camera cam = {};
    cam.position = Vector3Zero();
    cam.target = Vector3One();
    cam.fovy = 90;
    cam.projection = CAMERA_PERSPECTIVE;
    cam.up = (Vector3) {0.0f, 1.0f, 0.f};
    SetCameraMode(cam, CAMERA_FREE);

    session_init();

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        BeginDrawing();
        BeginTextureMode(target);

        UpdateCamera(&cam);

        BeginMode3D(cam);

        ClearBackground(BLACK);

        float delta = GetFrameTime();
        session_update_and_render(delta);

        EndMode3D();

        DrawFPS(10, 10);

        EndTextureMode();

        // TODO fix scuffed
        DrawTexturePro(target.texture, (Rectangle) {0.0f, 0.0f, WIDTH, -HEIGHT},
                       (Rectangle) {0.0f, 0.0f, WIDTH * SCALE, HEIGHT * SCALE},
                       (Vector2) {0.0f, 0.0f}, 0.0f, WHITE);

        session_update_and_render_gui(delta);

        EndDrawing();
    }

    session_dispose();
    assets_dispose();

    UnloadRenderTexture(target);
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
