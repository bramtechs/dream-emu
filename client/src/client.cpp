#define WIDTH 640
#define HEIGHT 480
#define SCALE 2

#include "raylib.h"
#include "linker.hpp"
#include "shared.hpp"

struct GameLevel {
    Camera camera;
    bool isFlying;

    LevelLayout layout;
    LevelFeed feed;
};

#include "assets.cpp"
#include "drawing.cpp"

static GameLevel *CurrentLevel = nullptr;

void game_update_and_render()
{
    UpdateCamera(&CurrentLevel->camera);

    if (IsKeyPressed(KEY_F3))
    {
        bool *flying = &CurrentLevel->isFlying;
        *flying = !*flying;
        if (*flying)
        {
            SetCameraMode(CurrentLevel->camera, CAMERA_FREE);     // Set camera mode
        } else
        {
            SetCameraMode(CurrentLevel->camera, CAMERA_FIRST_PERSON);     // Set camera mode
        }
    }

    if (CurrentLevel != nullptr)
    {
        BeginMode3D(CurrentLevel->camera);

        assert(level_update_and_stream != nullptr);
        level_update_and_stream(GetFrameTime());

        drawing_scene_draw(CurrentLevel);

        EndMode3D();

        DrawFPS(10, 10);
    } else
    {
        // no level
        ClearBackground(PURPLE);
        DrawText("No level, this is a bug... :(", 50, 50, 16, WHITE);
    }

}

void load_or_reload()
{
    CurrentLevel = new GameLevel();

    // setup camera
    CurrentLevel->camera = {{0.2f, 0.4f, 0.2f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 45.0f, 0};
    CurrentLevel->isFlying = false;
    CurrentLevel->layout = *Assets->levelLayouts.begin();
    CurrentLevel->feed = {};
    SetCameraMode(CurrentLevel->camera, CAMERA_FIRST_PERSON);     // Set camera mode

    // load level
    assert(level_load != nullptr);

    level_load(&CurrentLevel->layout,&CurrentLevel->feed);
}

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

    Vector3 mapPosition = {-16.0f, 0.0f, -8.0f};  // Set model position

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    bool isLoaded = linker_lib_link();

    assets_load();

    if (isLoaded)
    {
        load_or_reload();
    }

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
        BeginTextureMode(target);

        if (isLoaded)
        {
            game_update_and_render();
            //if (!IsWindowFocused())
            //{
            //    linker_lib_free();
            //    isLoaded = false;
            //    TraceLog(LOG_INFO, "FREED DLL");
            //}
        } else
        {
            // TODO do not load every frame
            //if (IsWindowFocused())
            //{
            //    if (linker_lib_link())
            //    {
            //        load_or_reload();
            //        isLoaded = true;
            //        TraceLog(LOG_INFO, "LOCKED DLL");
            //    }
            //}
        }

        EndTextureMode();

        // TODO fix scuffed
        DrawTexturePro(target.texture, {0.0f, 0.0f, WIDTH, -HEIGHT}, {0.0f, 0.0f, WIDTH * SCALE, HEIGHT * SCALE},
                       {0.0f, 0.0f}, 0.0f, WHITE);

        EndDrawing();
    }

    delete CurrentLevel;
    linker_lib_free();
    // TODO assets_dispose();
    UnloadRenderTexture(target);
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
