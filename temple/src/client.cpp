#include "magma.h"
#include "client.hpp"

// TODO implement palette shadering into engine
// TODO implement tilemaps into engine

EntityID spawn_block(EntityGroup& group, Vector3 pos){
    // TODO remove this ====================
    Image blockImage = RequestImage("spr_block_fg");

    // force texture into palette
    Palette palette = RequestPalette("pal_warm1");
    palette.MapImage(blockImage);

    Texture blockTexture = LoadTextureFromImage(blockImage);
    UnloadImage(blockImage);
    // | ====================================

    EntityID id = group.AddEntity();
    Sprite sprite = Sprite(id);
    sprite.SetCenter(pos.x, pos.y);
    sprite.SetTexture(blockTexture);
    group.AddEntityComponent(COMP_SPRITE, id, sprite);
    return id;
}

struct TempleGame {
    EntityGroup group;
    Shader shader;
    Camera2D camera;

    TempleGame() {

        // setup camera
        camera = {};
        camera.offset = Vector2Zero();          // Camera offset (displacement from target)
        camera.target = Vector2Zero();              // Camera target (rotation and zoom origin)
        camera.rotation = 0.f;                      // Camera rotation in degrees
        camera.zoom = 1.f;                          // Camera zoom (scaling), should be 1.0f by default

        INFO("LOADING SHADER");
        shader = RequestShader("frag_palette_switch");

        // setup shader palette
        Palette palette = RequestPalette("pal_warm1");

        int paletteLoc = GetShaderLocation(shader, "palette");
        SetShaderValueV(shader, paletteLoc, palette.colors, SHADER_UNIFORM_IVEC3, COLORS_PER_PALETTE);

        for (int y = 0; y < 3; y++){
            for (int x = 0; x < 3; x++){
                spawn_block(group,{x*64.f,y*64.f});
            }
        }

        // setup editor
        RegisterEntityBuilder(spawn_block);

    }

    void update_and_render(float delta) {
        BeginMagmaDrawing();

        ClearBackground(SKYBLUE);

        BeginMode2D(camera);

        BeginShaderMode(shader);

        if (!GameIsPaused()){
            group.UpdateGroup(delta);
        }
        group.DrawGroup();
        //group.DrawGroupDebug(camera);
        
        EndShaderMode();

        UpdateAndRenderEditor(camera, group, delta);
        UpdateAndRenderPauseMenu(delta,{0,0,0,50});

        EndMode2D();

        EndMagmaDrawing();
        UpdateAndDrawLog();
        RequestPalette("pal_warm1.pal").DrawPreview({ (float)GetScreenWidth() - 150,(float)GetScreenHeight() - 150,150,150});

        UpdateAndRenderEditorGUI(group, delta);

        EndDrawing();

        if (IsKeyPressed(KEY_ENTER)){
            // play test sound
            Sound sound = RequestSound("sfx_core_confirm");
            PlaySound(sound);
        }
    }
};

int main()
{
    SetTraceLogCallback(MagmaLogger);
    SetTraceLogLevel(LOG_DEBUG);
    SetTraceLogOpenLevel(LOG_ERROR);

    assert(ChangeDirectory("X:\\temple"));

    InitMagmaWindow(WIDTH, HEIGHT, WIDTH * SCALE, HEIGHT * SCALE, "Temple Mayhem");
    InitAudioDevice();
    //SetWindowState(FLAG_WINDOW_MAXIMIZED);

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
    CloseAudioDevice();

    return 0;
}
