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

    LoadUserSettings(GAME_NAME);

    InitAssets("assets");

    //Scene* scene = dream_init_hub();
    Scene* scene = scene_init(NULL);

    // SetWindowState(FLAG_WINDOW_MAXIMIZED);

    MainMenuConfig config = { 
        WIDTH,
        HEIGHT,
        {
            {
                "gui_splash.png",
                4.f,
            },
            {
                "gui_save_warning.png",
                4.f,
            },
        },
        2,
        "spr_sky.png",
        "Dream Emulator"
    };

    //TestList();

    if (!Settings.extra_skipIntro) {
        BootMainMenu(config,false);
    }

//    Shader shader = LoadShader(0, "../../../assets/shaders/gui/menu.fs");
//    int shaderTime = GetShaderLocation(shader, "iTime");
//    int shaderResolution = GetShaderLocation(shader, "iResolution");
//
//    Vector3 size = { WIDTH * 0.001, HEIGHT * 0.001, 1};
//    SetShaderValue(shader, shaderResolution, &size, SHADER_UNIFORM_VEC3);

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        if (LastLockFramerate != BOOL(Settings.engine_unlockFrameRate)){
            SetTargetFPS(BOOL(Settings.engine_unlockFrameRate) ? 1000:60);
            LastLockFramerate = BOOL(Settings.engine_unlockFrameRate);
        }

        float delta = GetFrameTime();
        float time = GetTime();

//        SetShaderValue(shader, shaderTime, &time, SHADER_UNIFORM_FLOAT);

//        BeginShaderMode(shader);

        if (UpdateAndDrawMainMenu(delta)){
            scene_update_and_render(scene,delta);
        }

//        EndShaderMode();
    }

    SaveUserSettings(GAME_NAME);

    DisposeAssets();

    scene_dispose(scene);

    CheckAllocations();

    CloseMagmaWindow();

    return 0;
}
