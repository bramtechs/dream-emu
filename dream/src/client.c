#include "magma.h"
#include "client.h"
#include "scene.h"
#include "dreams.h"

#define RAYGUI_IMPLEMENTATION

bool LockFramerate = true;
bool LastLockFramerate = true;

int main()
{

    // Initialization
    //--------------------------------------------------------------------------------------
    InitMagmaWindow(WIDTH, HEIGHT, WIDTH*SCALE, HEIGHT*SCALE, "DREAM_EMU");

    INFO("Launched at %s", GetWorkingDirectory());

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    SetTargetFPS(60);


    SetTraceLogLevel(LOG_INFO);

    InitAssets("assets");

    //TestArrays();
    // TODO move camera into scene

    Scene* scene = dream_init_hub();

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        if (LastLockFramerate != LockFramerate){
            SetTargetFPS(LockFramerate ? 60:3000);
            LastLockFramerate = LockFramerate;
        }

        float delta = GetFrameTime();
        scene_update_and_render(scene,delta);
    }

    UnloadAssets();

    scene_dispose(scene);

    CheckAllocations();

    CloseMagmaWindow();

    return 0;
}
