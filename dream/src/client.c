#include "magma.h"
#include "client.h"
#include "scene.h"
#include "dreams.h"
#include "settings.h"
#include "main_menu.h"

#define RAYGUI_IMPLEMENTATION

bool LastLockFramerate = true;

int main()
{

    // Initialization
    //--------------------------------------------------------------------------------------
    InitMagmaWindow(WIDTH, HEIGHT, WIDTH*SCALE, HEIGHT*SCALE, "DREAM_EMU");
    SetWindowState(FLAG_WINDOW_MAXIMIZED);

    INFO("Launched at %s", GetWorkingDirectory());

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    SetTargetFPS(60);

    SetTraceLogLevel(LOG_DEBUG);

    InitAssets("assets");

    //Scene* scene = dream_init_hub();
    Scene* scene = dream_init_garden();

    // SetWindowState(FLAG_WINDOW_MAXIMIZED);

    MainMenuConfig config = { 
        WIDTH,
        HEIGHT,
        {
            {
                "gui/splash.png",
            },
            {
                "gui/save-warning.png",
            },
        },
        2,
        "sky/sky.png"
    };

    // BootMainMenu(config,false);

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        if (LastLockFramerate != Settings.unlockFrameRate){
            SetTargetFPS(Settings.unlockFrameRate ? 1000:60);
            LastLockFramerate = Settings.unlockFrameRate;
        }

        float delta = GetFrameTime();

        if (UpdateAndDrawMainMenu(delta)){
            scene_update_and_render(scene,delta);
        }
    }

    DisposeAssets();

    scene_dispose(scene);

    CheckAllocations();

    CloseMagmaWindow();

    return 0;
}
