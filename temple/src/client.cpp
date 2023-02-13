#include "magma.h"
#include "client.hpp"

// fox player sprite
constexpr Vector2 FOX_CELL_SIZE = {17,32};
constexpr float FOX_ANIM_FPS = 15.f;

constexpr int COMP_FOX_PLAYER = 100;

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

struct FoxPlayer {
    uint score;
    uint lives;

    FoxPlayer(){
        this->score = 0;
        this->lives = 0;
    }
};

// TODO: add other animations

// factory functions
EntityID spawn_block(EntityGroup& group, Vector3 pos){
    EntityID id = group.AddEntity();

    Sprite sprite = Sprite({pos.x, pos.y});
    Texture blockTexture = RequestTexture("spr_block");
    sprite.SetTexture(blockTexture);
    group.AddEntityComponent(COMP_SPRITE, id, sprite);

    return id;
}

EntityID spawn_player(EntityGroup& group, Vector3 pos) {
    EntityID id = group.AddEntity();

    Sprite sprite = Sprite({pos.x, pos.y});
    Texture foxTexture = RequestTexture("spr_player_fox");
    sprite.SetTexture(foxTexture);
    group.AddEntityComponent(COMP_SPRITE, id, sprite);

    AnimationPlayer animPlayer = AnimationPlayer(ANIM_FOX_IDLE);
    group.AddEntityComponent(COMP_ANIM_PLAYER,id,animPlayer);

    PhysicsBody body = PhysicsBody(30.f,0.5f);
    group.AddEntityComponent(COMP_PHYS_BODY,id,body);

    PlatformerPlayer player = PlatformerPlayer();
    group.AddEntityComponent(COMP_PLAT_PLAYER,id,player);

    FoxPlayer fox = FoxPlayer();
    group.AddEntityComponent(COMP_FOX_PLAYER,id,fox);

    return id;
}

// TODO: callback system instead of this manual stuff
static void update_custom(EntityGroup& group){
    for (const auto &comp : group.comps){
        switch (comp.second.type) {
            case COMP_FOX_PLAYER:
            {
                auto fox = (FoxPlayer*) comp.second.data;
                auto sprite = (Sprite*) group.GetEntityComponent(comp.first, COMP_SPRITE);
                auto anim = (AnimationPlayer*) group.GetEntityComponent(comp.first, COMP_ANIM_PLAYER);
                auto plat = (PlatformerPlayer*) group.GetEntityComponent(comp.first, COMP_PLAT_PLAYER);

                // look left or right
                sprite->SetFlippedX(!plat->isLookingRight);

                // set animation according to pose
                switch (plat->pose){
                    case POSE_WALK:
                        anim->SetAnimation(ANIM_FOX_WALK);
                        break;
                    case POSE_SLIDE:
                        anim->SetAnimation(ANIM_FOX_STOP);
                        break;
                    case POSE_JUMP:
                        anim->SetAnimation(ANIM_FOX_JUMP);
                        break;
                    case POSE_FALL:
                        anim->SetAnimation(ANIM_FOX_LAND);
                        break;
                    default:
                        anim->SetAnimation(ANIM_FOX_IDLE);
                        break;
                }

                if (abs(sprite->center().x) > WIDTH*2 || abs(sprite->center().y) > HEIGHT*2) {
                    SetEntityCenter(comp.first, GetWindowCenter().x,GetWindowCenter().y);
                    WARN("Player fell off bounds!");
                }
            }
        }
    }
}

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

        // for (int y = 0; y < 2; y++){
        //     for (int x = 0; x < 10; x++){
        //         spawn_block(group,{x*64.f,64*4.f+y*64.f});
        //     }
        // }
        //spawn_player(group, {GetWindowCenter().x,GetWindowCenter().y,0});

        // fake floor
        //spawn_wall_brush(group,{240.f,346.f,0.f});

        // setup editor
        RegisterEntityBuilder(spawn_block);
        RegisterEntityBuilder(spawn_player);
    }

    void update_and_render(float delta) {
        BeginMagmaDrawing();

        ClearBackground(SKYBLUE);

        BeginMode2D(camera);

        // BeginPaletteMode(palette);

        if (!GameIsPaused()){
            group.UpdateGroup(delta);
            update_custom(group);
        }
        group.DrawGroup();
        //group.DrawGroupDebug(camera);
        
        // EndPaletteMode();

        UpdateAndRenderEditor(camera, group, delta);
        UpdateAndRenderPauseMenu(delta,{0,0,0,50});

        EndMode2D();

        EndMagmaDrawing();
        DrawRetroText("Move with AD, jump with Space\nPress Escape for menu\nPlatforming movement is still very early.", 50, 50, 18, RED);
        UpdateAndDrawLog();

        UpdateAndRenderEditorGUI(group, (Camera*)&camera, delta);

        EndDrawing();
    }
};

int main()
{
    SetTraceLogCallback(MagmaLogger);
    SetTraceLogLevel(LOG_DEBUG);
    SetTraceLogOpenLevel(LOG_ERROR);

    assert(ChangeDirectory("X:\\temple"));

    SetTraceLogLevel(LOG_DEBUG);
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
