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

// factory functions
EntityID spawn_block(EntityGroup& group, Vector3 pos){
    EntityID id = group.AddEntity();

    Sprite sprite = Sprite({pos.x, pos.y});
    Texture blockTexture = RequestTexture("spr_block");
    sprite.SetTexture(blockTexture);
    group.AddEntityComponent(id, COMP_SPRITE, sprite, true);

    return id;
}

EntityID spawn_player(EntityGroup& group, Vector3 pos) {
    EntityID id = group.AddEntity();

    Sprite sprite = Sprite({pos.x, pos.y});
    Texture foxTexture = RequestTexture("spr_player_fox");
    sprite.SetTexture(foxTexture);
    group.AddEntityComponent(id, COMP_SPRITE, sprite, true);

    AnimationPlayer animPlayer = AnimationPlayer(ANIM_FOX_IDLE);
    group.AddEntityComponent(id, COMP_ANIM_PLAYER,animPlayer);

    PhysicsBody body = PhysicsBody(30.f,0.5f);
    group.AddEntityComponent(id, COMP_PHYS_BODY,body);

    PlatformerPlayer player = PlatformerPlayer();
    group.AddEntityComponent(id, COMP_PLAT_PLAYER,player, true);

    FoxPlayer fox = FoxPlayer();
    group.AddEntityComponent(id, COMP_FOX_PLAYER,fox, true);

    return id;
}

static void update_custom_component(EntityGroup& group, IteratedComp& comp, float delta){
    switch (comp.second.type) {
        case COMP_FOX_PLAYER:
        {
            auto fox = (FoxPlayer*) comp.second.data;

            Sprite* sprite = NULL;
            group.GetEntityComponent(comp.first, COMP_SPRITE, &sprite);

            AnimationPlayer* anim = NULL;
            group.GetEntityComponent(comp.first, COMP_ANIM_PLAYER, &anim);

            PlatformerPlayer* plat = NULL;
            group.GetEntityComponent(comp.first, COMP_PLAT_PLAYER, &plat);

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

static bool layout_menu(float delta){
    static auto menu = PopMenu();


    ButtonTable buttons;
    buttons.AddButton("New game",NULL);
    buttons.AddButton("Load game",NULL);
    buttons.AddButton("Options",NULL);
    buttons.AddButton("Quit",[](){
        CloseWindow();
    });

    menu.RenderPanel();
    menu.DrawPopButtons(buttons);
    menu.ProcessSelectedButton(buttons);
    menu.EndButtons({150,250});

    return false;
}

int main(int argc, char** argv)
{
    SetTraceLogCallback(MagmaLogger);
    SetTraceLogLevel(LOG_DEBUG);
    SetTraceLogOpenLevel(LOG_ERROR);

    SetTraceLogLevel(LOG_DEBUG);
    RegisterArguments(argc,argv);
    InitMagmaWindow(WIDTH, HEIGHT, WIDTH * SCALE, HEIGHT * SCALE, "Temple Mayhem");
    InitAudioDevice();
    //SetWindowState(FLAG_WINDOW_MAXIMIZED);

    INFO("Launched at %s", GetWorkingDirectory());

    // TempleGame
    AdvEntityGroup group(9.8f);
    Shader shader;

    Camera2D camera = {};
    camera.offset = Vector2Zero();          // Camera offset (displacement from target)
    camera.target = Vector2Zero();              // Camera target (rotation and zoom origin)
    camera.rotation = 0.f;                      // Camera rotation in degrees
    camera.zoom = 1.f;                          // Camera zoom (scaling), should be 1.0f by default

    Palette palette;

    // vsync
    SetTargetFPS(60);

    // setup editor
    RegisterEntityBuilder(spawn_block);
    RegisterEntityBuilder(spawn_player);

    Video video;

    // register custom entity callbacks
    group.RegisterUpdater(update_custom_component);

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    if (LoadAssets()) {
        LoadMagmaSettings();

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

        while (!WindowShouldClose()) // Detect window close button or ESC key
        {
            float delta = GetFrameTime();
            float time = GetTime();

            if (menu.UpdateAndDraw(delta)) {

                // draw main stuff
                BeginMagmaDrawing();

                ClearBackground(SKYBLUE);

                BeginMode2D(camera);

                if (!GameIsPaused()){
                    group.UpdateGroup(delta);
                }
                group.DrawGroup();
                group.DrawGroupDebug();

                // EndPaletteMode();

                UpdateAndRenderEditor(camera, group, delta);
                UpdateAndRenderPauseMenu(delta,{0,0,0,50}, &group);
                UpdateAndRenderInputBoxes(delta);

                EndMode2D();

                EndMagmaDrawing();
                DrawRetroText("Move with AD, jump with Space\nPress Escape for menu\nPlatforming movement is still very early.", 50, 50, 18, RED);
                UpdateAndDrawLog();

                UpdateAndRenderEditorGUI(group, (Camera*)&camera, delta);

                EndDrawing();

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
