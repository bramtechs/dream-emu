#include "magma.h"
#include "client.hpp"

struct TempleGame {
    Texture blockTexture;

    TempleGame() {
        blockTexture = Assets::RequestTexture("spr_block_2");
    }

    void update_and_render(float delta) {
		BeginMagmaDrawing();

        ClearBackground(SKYBLUE);

        DrawTexture(blockTexture, 20, 20, WHITE);

		EndMagmaDrawing();
        UpdateAndDrawLog();
		EndDrawing();
    }
};

int main()
{
    SetTraceLogCallback(MagmaLogger);
    SetTraceLogLevel(LOG_DEBUG);
    assert(ChangeDirectory("X:\\temple"));

    InitMagmaWindow(WIDTH, HEIGHT, WIDTH * SCALE, HEIGHT * SCALE, "Temple Mayhem");
    SetWindowState(FLAG_WINDOW_MAXIMIZED);

    INFO("Launched at %s", GetWorkingDirectory());

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    SetTargetFPS(60);

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
            }, true);

        TempleGame game;

        while (!WindowShouldClose()) // Detect window close button or ESC key
        {
            float delta = GetFrameTime();
            float time = GetTime();

            if (menu.UpdateAndDraw(delta)) {
                // draw scene here
                game.update_and_render(delta);
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
