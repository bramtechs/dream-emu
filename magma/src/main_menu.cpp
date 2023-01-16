#include "main_menu.h"

#define FADE_IN  0
#define DISPLAY  1
#define FADE_OUT 2

typedef struct {
    bool skipSplash;

    float alpha;
    size_t state;
    size_t curSplash;

    Texture splashTextures[MAX_SPLASHES];
    Texture bgTexture;

    bool isDone;

    float timer;
} MainMenuSession;

static MainMenuSession Session = { 0 };
static MainMenuConfig MenuConfig = { 0 };

static Color saveCol = { 0, 0, 0, 255 };

void BootMainMenu(MainMenuConfig config, bool skipSplash){

    memcpy(&MenuConfig, &config, sizeof(MainMenuConfig));

    // initialize session
    Session.skipSplash = skipSplash;
    Session.state = FADE_IN;
    Session.curSplash = 0;
    Session.timer = 0.f;
    Session.alpha = 0.f;
    Session.isDone = false;

    // load all textures
    Session.bgTexture = RequestTexture(config.bgPath);
    for (int i = 0; i < config.splashCount; i++){
        Session.splashTextures[i] = RequestTexture(config.splashes[i].imgPath);
    }

    INFO("Booting main menu!");
}

void DrawScreenSaver(float delta){
    ClearBackground(saveCol);
    Color* c = &saveCol;
    c->r += delta*10.f;
    c->g -= delta*10.f;
    c->b += delta*20.f;
    c->a = 255;
}

void DrawBackground(Texture texture, Color tint){
    Rectangle src = { 0, 0, texture.width, texture.height };
    Vector2 winSize = GetMagmaGameSize();
    Rectangle dest = { 0, 0, winSize.x, winSize.y }; 
    DrawTexturePro(texture, src, dest, Vector2Zero(), 0.f, tint);
}

bool UpdateAndDrawMainMenu(float delta) {
    if (Session.isDone || MenuConfig.width == 0){ // skip if not booted
        return true;
    }

    float waitTime = FADE_DURATION;
    Texture texture = Session.splashTextures[Session.curSplash];

    if (Session.curSplash < MenuConfig.splashCount) {
        switch (Session.state){
            case FADE_IN:
                Session.alpha += delta / FADE_DURATION;
                break;
            case DISPLAY:
                SplashScreen splash = MenuConfig.splashes[Session.curSplash];
                waitTime = splash.duration;
                Session.alpha = 1.f;
                break;
            case FADE_OUT:
                Session.alpha -= delta / FADE_DURATION;
                break;
            default:
                assert(false);
        }

        if (Session.timer > waitTime){
            Session.timer -= waitTime;
            Session.state++;
            if (Session.state > FADE_OUT){
                Session.state = FADE_IN;
                Session.curSplash++;
            }
        }
        Session.timer += delta;
    }
    else {
        Session.alpha = 1.f;
        texture = Session.bgTexture;
    }

    BeginMagmaDrawing();
    
    char alphaByte = Clamp(Session.alpha*255,0,255);
    Color tint = { 255, 255, 255, alphaByte };
    DrawBackground(texture,tint);

    if (Session.curSplash >= MenuConfig.splashCount){
        DrawText(MenuConfig.title,20,20,36,WHITE);
    }

    if (IsKeyPressed(KEY_ENTER)){
        Session.isDone = true;
    }

    EndMagmaDrawing();
    EndDrawing();

    return false;
}
