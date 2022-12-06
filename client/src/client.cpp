#define WIDTH 640
#define HEIGHT 480
#define SCALE 2

// TODO confusing filename

#include "raylib.h"

#include "linker.hpp"
#include "shared.hpp"

struct GameSession {
    Camera camera;
    bool isFlying;
};

#include "assets.cpp"
#include "drawing.cpp"

static LevelFeed *CurrentFeed = nullptr;
static GameSession *CurrentSession = nullptr;

void game_session_reset()
{
    if (CurrentSession == nullptr){
        CurrentSession = new GameSession();
    }

    // setup camera
    CurrentSession->camera = {{0.2f, 0.4f, 0.2f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 45.0f, 0};
    CurrentSession->isFlying = false;
    SetCameraMode(CurrentSession->camera, CAMERA_FIRST_PERSON);     // Set camera mode
}

void game_update_and_render()
{

    UpdateCamera(&CurrentSession->camera);

    if (IsKeyPressed(KEY_F3))
    {
        bool *flying = &CurrentSession->isFlying;
        *flying = !*flying;
        if (*flying)
        {
            SetCameraMode(CurrentSession->camera, CAMERA_FREE);     // Set camera mode
        } else
        {
            SetCameraMode(CurrentSession->camera, CAMERA_FIRST_PERSON);     // Set camera mode
        }
    }

    if (CurrentFeed != nullptr)
    {
        BeginMode3D(CurrentSession->camera);

        assert(level_update_and_stream != nullptr);
        level_update_and_stream(GetFrameTime());

        drawing_update_and_draw(CurrentFeed, CurrentSession);

        EndMode3D();

        DrawFPS(10, 10);
    } else
    {
        // no level
        ClearBackground(PURPLE);
        DrawText("No level, this is a bug... :(", 50, 50, 16, WHITE);
    }

}

void game_refresh(){
    // TOOD also reload assets
    level_load(Assets->levelLayouts.first(),CurrentFeed);
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

    SetTargetFPS(60);

    bool isLoaded = linker_lib_link();

    CurrentFeed = new LevelFeed();

    assets_load();
    game_session_reset();

    if (isLoaded)
    {
        game_refresh();
    }

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
        BeginTextureMode(target);

        if (isLoaded)
        {
            game_update_and_render();
            if (!IsWindowFocused())
            {
                linker_lib_free();
                isLoaded = false;
                TraceLog(LOG_INFO, "FREED DLL");
            }
        } else
        {
            // TODO do not load every frame
            if (IsWindowFocused())
            {
                if (linker_lib_link())
                {
                    game_refresh();
                    isLoaded = true;
                    TraceLog(LOG_INFO, "LOCKED DLL");
                }
            }
        }

        EndTextureMode();

        // TODO fix scuffed
        DrawTexturePro(target.texture, {0.0f, 0.0f, WIDTH, -HEIGHT}, {0.0f, 0.0f, WIDTH * SCALE, HEIGHT * SCALE},
                       {0.0f, 0.0f}, 0.0f, WHITE);

        EndDrawing();
    }

    delete CurrentFeed;
    linker_lib_free();
    // TODO assets_dispose();
    UnloadRenderTexture(target);
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
