#include "magma.h"
#include "client.h"
#include "level_data.h"

#define IMPLEMENT_TEMPLE_LEVEL_SELECT
#include "level_select.h"

// fox player sprite
#define FOX_CELL_SIZE {17,32}

#define FOX_ANIM_FPS        15.f
#define COMP_FOX_PLAYER     100

static const SheetAnimation ANIM_FOX_IDLE = {
    STRING(ANIM_FOX_IDLE),
    "spr_player_fox",
    {68,64},
    FOX_CELL_SIZE,
    1,
    PLAY_LOOP,
    FOX_ANIM_FPS,
};

static const SheetAnimation ANIM_FOX_WALK = {
    STRING(ANIM_FOX_WALK),
    "spr_player_fox",
    {0,0},
    FOX_CELL_SIZE,
    3,
    PLAY_PING_PONG,
    FOX_ANIM_FPS,
};

static const SheetAnimation ANIM_FOX_STOP = {
    STRING(ANIM_FOX_STOP),
    "spr_player_fox",
    {68, 0},
    FOX_CELL_SIZE,
    2,
    PLAY_ONCE,
    FOX_ANIM_FPS * 0.5,
};

static const SheetAnimation ANIM_FOX_JUMP = {
    STRING(ANIM_FOX_JUMP),
    "spr_player_fox",
    {0, 32},
    FOX_CELL_SIZE,
    1,
    PLAY_LOOP,
    0.f,
};

static const SheetAnimation ANIM_FOX_LAND = {
    STRING(ANIM_FOX_LAND),
    "spr_player_fox",
    {17, 32},
    FOX_CELL_SIZE,
    1,
    PLAY_LOOP,
    0.f,
};

typedef struct {
    uint score;
    uint lives;
} FoxPlayer;

static EntityID SpawnFoxPlayer(Vector2 pos) {
   // spawn player
   PlatformerPlayerConfig config;
   config.moveSpeed = 3000.f;
   config.jumpForce = 800.f;
   config.animations[POSE_IDLE]  = &ANIM_FOX_IDLE;
   config.animations[POSE_WALK]  = &ANIM_FOX_WALK;
   config.animations[POSE_SLIDE] = &ANIM_FOX_STOP;
   config.animations[POSE_JUMP]  = &ANIM_FOX_JUMP;
   config.animations[POSE_FALL]  = &ANIM_FOX_LAND;
   config.animations[POSE_DUCK]  = &ANIM_FOX_IDLE;
   return SpawnPlatformerPlayer(pos, config);
}

int main(int argc, char** argv)
{
    SetTraceLogCallback(MagmaLogger);
    SetTraceLogLevel(LOG_DEBUG);
    SetTraceLogOpenLevel(LOG_ERROR);

    SetTraceLogLevel(LOG_DEBUG);
    RegisterArguments(argc,argv);
    InitMagmaWindowEx(WIDTH, HEIGHT, WIDTH * SCALE, HEIGHT * SCALE, "Temple Mayhem");
    InitAudioDevice();
    //SetWindowState(FLAG_WINDOW_MAXIMIZED);

    INFO("Launched at %s", GetWorkingDirectory());

    // TempleGame
    Shader shader;

    Camera2D camera = { 0 };
    camera.offset = Vector2Zero();              // Camera offset (displacement from target)
    camera.target = Vector2Zero();              // Camera target (rotation and zoom origin)
    camera.rotation = 0.f;                      // Camera rotation in degrees
    camera.zoom = 1.f;                          // Camera zoom (scaling), should be 1.0f by default

    // vsync
    SetTargetFPS(60);

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    if (LoadAssets()) {
        LoadMagmaSettings();

#if 0
        MainMenu menu = MainMenu({
            WIDTH,
            HEIGHT,
            {
                {
                    "gui_core_splash",
                    2.f,
                },
            },
            "spr_block",
            true,
            "Temple Mayhem",
            "basically a mario clone, pls don't sue",
            layout_menu,
            }, true);
#endif

        SpawnFoxPlayer((Vector2){300.f,300.f});

        while (!WindowShouldClose()) // Detect window close button or ESC key
        {
            float delta = GetFrameTime();
            float time = GetTime();

            // if (menu.UpdateAndDraw(delta)) {

                // draw level select
                LevelInfo* selectedLevel = NULL;
                if (UpdateAndDrawLevelSelect(delta, &selectedLevel)){
                    // draw main stuff
                    BeginMagmaDrawing();

                    ClearBackground(SKYBLUE);

                    BeginMode2D(camera);

                    EndMode2D();

                    EndMagmaDrawing();
                    DrawRetroTextEx("Move with AD, jump with Space\nPress Escape for menu\nPlatforming movement is still very early.", 50, 50, 18, RED);

                    EndDrawing();
                }
            // }

        }

        SaveMagmaSettings();

        DisposeAssets();

    }
    else {
        // render fail screen until game close
        ShowFailScreen("Could not find 'assets.mga'.\nPlease extract your download.");
    }

    CloseMagmaWindow();
    CloseAudioDevice();

    return 0;
}
