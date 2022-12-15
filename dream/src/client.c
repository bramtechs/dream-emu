#include "magma.h"
#include "client.h"
#include "scene.h"

#define RAYGUI_IMPLEMENTATION

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitMagmaWindow(WIDTH, HEIGHT, WIDTH*SCALE, HEIGHT*SCALE, "DREAM_EMU");

    INFO("Launched at %s", GetWorkingDirectory());

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    SetTargetFPS(1000);

    Camera cam = { 0 };
    cam.position = Vector3Zero();
    cam.target = Vector3One();
    cam.fovy = 80;
    cam.projection = CAMERA_PERSPECTIVE;
    cam.up = (Vector3) {0.0f, 1.0f, 0.f};
    SetCameraMode(cam, CAMERA_FREE);

    InitAssets("assets");

    // TODO move camera into scene

    SetTraceLogLevel(LOG_ALL);
    Scene* scene = scene_init(&cam);

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        BeginMagmaDrawing();

            UpdateCamera(&cam);

            BeginMode3D(cam);

            ClearBackground(BLACK);

            float delta = GetFrameTime();
            scene_update_and_render(scene,delta);

            EndMode3D();

            // ui drawing
            //DrawCircleV(GetScaledMousePosition(), 4.f, RED);                              // Draw a color-filled circle

            DrawFPS(10, 10);

            if (((int)GetTime()) % 2 == 0) {
                DrawText("DEMO DISC", WIDTH - MeasureText("DEMO DISC ", 20), HEIGHT - 20, 20, WHITE);
            }

        EndMagmaDrawing();

        // fullscreen debug ui here

        scene_update_and_render_gui(scene,delta);

        EndDrawing();
    }

    UnloadAssets();

    scene_dispose(scene);

    CheckAllocations();

    CloseMagmaWindow();

    return 0;
}
