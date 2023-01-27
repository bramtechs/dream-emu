#include "magma.h"
#include "client.h"
#include "dreams.h"

#define RAYGUI_IMPLEMENTATION

bool LastLockFramerate = true;

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitMagmaWindow(WIDTH, HEIGHT, WIDTH * SCALE, HEIGHT * SCALE, "DREAM_EMU");
    SetWindowState(FLAG_WINDOW_MAXIMIZED);

    INFO("Launched at %s", GetWorkingDirectory());

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    SetTargetFPS(60);

    SetTraceLogLevel(LOG_DEBUG);

    if (LoadAssets()) {
        UserPrefs::Load();
        LoadMagmaSettings();

        //Scene* scene = dream_init_hub();
        auto scene = GardenDream();

        // SetWindowState(FLAG_WINDOW_MAXIMIZED);

        MainMenu menu = MainMenu({
            WIDTH,
            HEIGHT,
            {
                {
                    "gui_splash",
                    4.f,
                },
                {
                    "gui_save_warning",
                    4.f,
                },
            },
            "spr_sky",
            "Dream Emulator"
            }, Settings.skipIntro);

        //TestList();

        //    Shader shader = LoadShader(0, "../../../assets/shaders/gui/menu.fs");
        //    int shaderTime = GetShaderLocation(shader, "iTime");
        //    int shaderResolution = GetShaderLocation(shader, "iResolution");
        //
        //    Vector3 size = { WIDTH * 0.001, HEIGHT * 0.001, 1};
        //    SetShaderValue(shader, shaderResolution, &size, SHADER_UNIFORM_VEC3);

            // Main game loop
        while (!WindowShouldClose()) // Detect window close button or ESC key
        {
            if (LastLockFramerate != Settings.unlockFrameRate) {
                SetTargetFPS(Settings.unlockFrameRate ? 1000 : 60);
                LastLockFramerate = Settings.unlockFrameRate;
            }

            float delta = GetFrameTime();
            float time = GetTime();

            //        SetShaderValue(shader, shaderTime, &time, SHADER_UNIFORM_FLOAT);

            //        BeginShaderMode(shader);

            if (menu.UpdateAndDraw(delta)) {
                scene.update_and_render(delta);
            }

            //        EndShaderMode();
            //
        }

        UserPrefs::Save();

        SaveMagmaSettings();

        DisposeAssets();

    }
    else {
        // render fail screen until game close
        Assets::EnterFailScreen(WIDTH,HEIGHT);
    }
    CheckAllocations();

    CloseMagmaWindow();

    return 0;
}
