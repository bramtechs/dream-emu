#include "raylib.h"
#include "raymath.h"
#include "assets.hpp"

#include <entt/entt.hpp>

#define SCREEN_WIDTH (1280)
#define SCREEN_HEIGHT (720)

#define WINDOW_TITLE "Window title"


int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    assets_load();

    Camera3D camera = { 0 };
    camera.position = { 0.0f, 10.0f, 10.0f };  // Camera position
    camera.target = { 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 70.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    SetCameraMode(camera, CAMERA_FREE);

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera);

        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        DrawCube(Vector3Zero(),1,1,1,GREEN);
        DrawGrid(10, 1);

        EndMode3D();

        EndDrawing();
    }

    CloseWindow();

    assets_dispose();

    return 0;
}
