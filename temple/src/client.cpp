#include "magma.h"
#include "client.hpp"

// fox player sprite
constexpr Vector2 FOX_CELL_SIZE = {17,32};
constexpr float FOX_ANIM_FPS = 15.f;

static SheetAnimation ANIM_IDLE = {
    STRING(ANIM_IDLE),
    "spr_player_fox",
    {69,65},
    FOX_CELL_SIZE,
    1,
    PLAY_LOOP,
    FOX_ANIM_FPS,
};

static SheetAnimation ANIM_FOX_WALK = {
    STRING(ANIM_FOX_WALK),
    "spr_player_fox",
    {0,0},
    FOX_CELL_SIZE,
    3,
    PLAY_PING_PONG,
    FOX_ANIM_FPS,
};

static SheetAnimation ANIM_STOP = {
    STRING(ANIM_STOP),
    "spr_player_fox",
    {35, 0},
    FOX_CELL_SIZE,
    2,
    PLAY_ONCE,
    FOX_ANIM_FPS * 0.5,
};

static SheetAnimation ANIM_JUMP = {
    STRING(ANIM_JUMP),
    "spr_player_fox",
    {0, 32},
    FOX_CELL_SIZE,
    1,
    PLAY_LOOP,
    0.f,
};

static SheetAnimation ANIM_LAND = {
    STRING(ANIM_LAND),
    "spr_player_fox",
    {0, 32},
    FOX_CELL_SIZE,
    1,
    PLAY_LOOP,
    0.f,
};

// TODO: add other animations

// factory functions
EntityID spawn_block(EntityGroup& group, Vector3 pos){
    EntityID id = group.AddEntity();

    Sprite sprite = Sprite({pos.x, pos.y});
    Texture blockTexture = RequestIndexedTexture("spr_block");
    sprite.SetTexture(blockTexture);
    group.AddEntityComponent(COMP_SPRITE, id, sprite);

    PhysicsBody body = PhysicsBody(false);
    group.AddEntityComponent(COMP_PHYS_BODY, id, body);

    return id;
}

EntityID spawn_player(EntityGroup& group, Vector3 pos) {
    EntityID id = group.AddEntity();

    Sprite sprite = Sprite({pos.x, pos.y});
    Texture foxTexture = RequestIndexedTexture("spr_player_fox");
    sprite.SetTexture(foxTexture);
    group.AddEntityComponent(COMP_SPRITE, id, sprite);

    AnimationPlayer animPlayer = AnimationPlayer(ANIM_FOX_WALK);
    group.AddEntityComponent(COMP_ANIM_PLAYER,id,animPlayer);

    PhysicsBody body = PhysicsBody(30.f,0.5f);
    group.AddEntityComponent(COMP_PHYS_BODY,id,body);

    PlatformerPlayer player = PlatformerPlayer();
    group.AddEntityComponent(COMP_PLAT_PLAYER,id,player);

    return id;
}

#define PAL_WARM        "pal_warm"
#define PAL_BROWN       "pal_brown"
#define PAL_DUSK        "pal_dusk"
#define PAL_NIGHT       "pal_night"

#define PAL_DEFAULT    PAL_WARM 

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
        // palette = RequestPalette(PAL_DEFAULT);
        // SetDefaultPalette(palette);

        for (int y = 0; y < 2; y++){
            for (int x = 0; x < 10; x++){
                spawn_block(group,{x*64.f,64*4.f+y*64.f});
            }
        }
        spawn_player(group, {GetWindowCenter().x,GetWindowCenter().y,0});

        // setup editor
        RegisterEntityBuilder(spawn_block);
    }

    void update_and_render(float delta) {
        BeginMagmaDrawing();

        ClearBackground(SKYBLUE);

        BeginMode2D(camera);

        // BeginPaletteMode(palette);

        if (!GameIsPaused()){
            group.UpdateGroup(delta);
        }
        group.DrawGroup();
        //group.DrawGroupDebug(camera);
        
        // EndPaletteMode();

        UpdateAndRenderEditor(camera, group, delta);
        UpdateAndRenderPauseMenu(delta,{0,0,0,50});

        EndMode2D();

        EndMagmaDrawing();
        UpdateAndDrawLog();

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

    SetTargetFPS(1000);

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
