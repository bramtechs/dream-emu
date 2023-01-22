#include "magma.h"
#include "client.hpp"

void update_and_render(float delta){

}

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

    if (Assets::Init("assets.mga") != NULL) {
        LoadMagmaSettings();

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

        while (!WindowShouldClose()) // Detect window close button or ESC key
        {
            float delta = GetFrameTime();
            float time = GetTime();

            if (menu.UpdateAndDraw(delta)) {
                // draw scene here
                update_and_render(delta);
            }
        }

        SaveMagmaSettings();

        Assets::Dispose();

    }
    else {
        // render fail screen until game close
        Assets::EnterFailScreen(WIDTH,HEIGHT);
    }
    CheckAllocations();

    CloseMagmaWindow();

    return 0;
}
