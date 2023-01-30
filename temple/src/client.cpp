#include "magma.h"
#include "client.hpp"

// TODO implement palette shadering into engine
// TODO implement tilemaps into engine

struct TempleGame {
    EntityGroup group;
    Shader shader;
    Camera2D camera;

    TempleGame() {
        Image blockImage = RequestImage("spr_block_fg");

        camera = {};
        camera.offset = GetWindowCenter();          // Camera offset (displacement from target)
        camera.target = Vector2Zero();              // Camera target (rotation and zoom origin)
        camera.rotation = 0.f;                      // Camera rotation in degrees
        camera.zoom = 1.f;                          // Camera zoom (scaling), should be 1.0f by default

        // force texture into palette
        Palette palette = RequestPalette("pal_warm1");
        palette.MapImage(blockImage);

        Texture blockTexture = LoadTextureFromImage(blockImage);
        UnloadImage(blockImage);

        INFO("LOADING SHADER");
        shader = RequestShader("frag_palette_switch");

        int paletteLoc = GetShaderLocation(shader, "palette");
        SetShaderValueV(shader, paletteLoc, palette.colors, SHADER_UNIFORM_IVEC3, COLORS_PER_PALETTE);

        // add single block
        EntityID id = group.AddEntity();
        Sprite sprite = Sprite(id);
        group.AddEntityComponent(COMP_SPRITE, id, sprite);
    }

    void update_and_render(float delta) {
        BeginMagmaDrawing();

        ClearBackground(SKYBLUE);

        BeginShaderMode(shader);

        group.UpdateGroup(delta);
        group.DrawGroup();
        group.DrawGroupDebug(camera);

        EndShaderMode();

        EndMagmaDrawing();
        UpdateAndDrawLog();
        RequestPalette("pal_warm1.pal").DrawPreview({ (float)GetScreenWidth() - 150,(float)GetScreenHeight() - 150,150,150});
        EndDrawing();
    }
};

int main()
{
    SetTraceLogCallback(MagmaLogger);
    SetTraceLogLevel(LOG_DEBUG);
    SetTraceLogOpenLevel(LOG_ERROR);

    assert(ChangeDirectory("X:\\temple"));

    InitMagmaWindow(WIDTH, HEIGHT, WIDTH * SCALE, HEIGHT * SCALE, "Temple Mayhem");
    SetWindowState(FLAG_WINDOW_MAXIMIZED);

    INFO("Launched at %s", GetWorkingDirectory());

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    SetTargetFPS(60);

    if (LoadAssets()) {
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

        DisposeAssets();

    }
    else {
        // render fail screen until game close
        ShowFailScreen("Could not find 'assets.mga'.\nPlease extract your download.");
    }
    CheckAllocations();

    CloseMagmaWindow();

    return 0;
}
