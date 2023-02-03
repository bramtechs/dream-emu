#include "magma.h"
#include "client.hpp"

EntityID spawn_block(EntityGroup& group, Vector3 pos){
    EntityID id = group.AddEntity();
    Sprite sprite = Sprite(id);
    sprite.SetCenter(pos.x, pos.y);
    Texture blockTexture = RequestIndexedTexture("spr_block");
    sprite.SetTexture(blockTexture);
    group.AddEntityComponent(COMP_SPRITE, id, sprite);
    return id;
}

#define PAL_WARM        "pal_warm"
#define PAL_BROWN       "pal_brown"
#define PAL_DUSK        "pal_dusk"
#define PAL_NIGHT       "pal_night"

#define PAL_DEFAULT    PAL_NIGHT 

struct TempleGame {
    EntityGroup group;
    Shader shader;
    Camera2D camera;
    Palette palette;

    TempleGame() {

        // setup camera
        camera = {};
        camera.offset = Vector2Zero();          // Camera offset (displacement from target)
        camera.target = Vector2Zero();              // Camera target (rotation and zoom origin)
        camera.rotation = 0.f;                      // Camera rotation in degrees
        camera.zoom = 1.f;                          // Camera zoom (scaling), should be 1.0f by default

        // setup shader palette
        palette = RequestPalette(PAL_DEFAULT);
        SetDefaultPalette(palette);

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

        BeginPaletteMode(palette);

        if (!GameIsPaused()){
            group.UpdateGroup(delta);
        }
        group.DrawGroup();
        //group.DrawGroupDebug(camera);
        
        EndPaletteMode();

        UpdateAndRenderEditor(camera, group, delta);
        UpdateAndRenderPauseMenu(delta,{0,0,0,50});

        EndMode2D();

        EndMagmaDrawing();
        UpdateAndDrawLog();
        RequestPalette("pal_warm1.pal").DrawPreview({ (float)GetScreenWidth() - 150,(float)GetScreenHeight() - 150,150,150});

        UpdateAndRenderEditorGUI(group, delta);

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
