#include "magma.h"

#define FADE_IN  0
#define DISPLAY  1
#define FADE_OUT 2

MainMenu::MainMenu(MainMenuConfig config, bool skipSplash) 
    : skipSplash(skipSplash), config(config) {

    // initialize session
    state = FADE_IN;
    curSplash = 0;
    timer = 0.f;
    alpha = 0.f;
    isDone = false;

    // load all textures
    bgTexture = RequestTexture(config.bgPath);
    for (const auto &splash : config.splashes) {
        Texture img = RequestTexture(splash.imgPath);
        splashTextures.push_back(img);
    }

    INFO("Booting main menu!");
}

void MainMenu::DrawScreenSaver(float delta) {
    ClearBackground(saveCol);
    Color* c = &saveCol;
    c->r += delta * 10.f;
    c->g -= delta * 10.f;
    c->b += delta * 20.f;
    c->a = 255;
}

void MainMenu::DrawBackground(Texture texture, Color tint) {
    Rectangle src = { 0, 0, texture.width, texture.height };
    Rectangle dest = { 0, 0, Window.winSize.x, Window.winSize.y };
    DrawTexturePro(texture, src, dest, Vector2Zero(), 0.f, tint);
}

bool MainMenu::UpdateAndDraw(float delta) {
    if (skipSplash || isDone || config.width == 0) { // skip if not booted
        return true;
    }

    float waitTime = FADE_DURATION;
    Texture texture = {};

    if (curSplash < config.splashes.size()) {
        texture = splashTextures[curSplash];
        switch (state) {
        case FADE_IN:
            alpha += delta / FADE_DURATION;
            break;
        case DISPLAY: {
                SplashScreen splash = config.splashes[curSplash];
                waitTime = splash.duration;
                alpha = 1.f;
            }
            break;
        case FADE_OUT:
            alpha -= delta / FADE_DURATION;
            break;
        default:
            assert(false);
            break;
        }

        if (timer > waitTime) {
            timer -= waitTime;
            state++;
            if (state > FADE_OUT) {
                state = FADE_IN;
                curSplash++;
            }
        }
        timer += delta;
    }
    else {
        alpha = 1.f;
        texture = bgTexture;
    }

    BeginMagmaDrawing();

    char alphaByte = Clamp(alpha * 255, 0, 255);
    Color tint = { 255, 255, 255, alphaByte };
    DrawBackground(texture, tint);

    if (curSplash >= config.splashes.size()) {
        DrawText(config.title, 20, 20, 36, WHITE);
    }

    if (IsKeyPressed(KEY_ENTER)) {
        isDone = true;
    }

    EndMagmaDrawing();
    EndDrawing();

    return false;
}

struct PopMenuConfig {
    bool isOpened;
};

static PopMenuConfig PopConfig = {};

bool GameShouldPause(){
    return PopConfig.isOpened;
}

// TODO make this stuff reusable
// TODO make more customizable with 'MenuTheme' config struct

void DrawMenuTriangle(Vector2 center, float scale=30.f, float oscil=0.f, bool tumble=false, Color color=WHITE){

    Vector2 vertices[3] = {
        {-1,-1},
        {-1, 1},
        { 1, 0},
    };

    float offsetX = 0.f;
    if (!tumble){
        offsetX = (sinf(GetTime()*oscil)+1)*0.5f*oscil;
    }

    for (int i = 0; i < 3; i++){
        if (tumble) {
            vertices[i].y *= (sinf(GetTime()*3.f)+1)*0.5f;
        }
        vertices[i].x = center.x+offsetX+(vertices[i].x*scale);
        vertices[i].y = center.y+(vertices[i].y*scale);
    }

    DrawTriangleStrip(vertices,3,color);
}

constexpr int PADDING = 10;
constexpr int ARROW_SIZE = 8;
void DrawPopButton(Vector2 topLeft, const char* text, bool isSelected,
                   Vector2* totalSize, bool selectable=true, int fontSize=16, Color color=WHITE){
    Vector2 textPos = {
        topLeft.x+PADDING+ARROW_SIZE*3.f,
        topLeft.y+(*totalSize).y + PADDING,
    };

    Color actualColor = color;
    if (!selectable){
        actualColor = ColorBrightness(color,-0.3);
    }

    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.f);    // Measure string size for Font
    textSize.x += PADDING * 2 + ARROW_SIZE;

    if (isSelected){
        Vector2 triPos = {
            topLeft.x+PADDING+ARROW_SIZE,
            topLeft.y+(*totalSize).y + PADDING + textSize.y * 0.5f,
        };
        DrawMenuTriangle(triPos, ARROW_SIZE);
    }

    DrawText(text,textPos.x,textPos.y,fontSize,actualColor);

    if ((*totalSize).x < textSize.x){
        (*totalSize).x = textSize.x;
    }
    (*totalSize).y += textSize.y;
}

void UpdateAndRenderPopMenu(float delta, Color bgColor){
    if (IsKeyPressed(KEY_ESCAPE)){
        PopConfig.isOpened = !PopConfig.isOpened;
    }

    // draw bg (if any)
    DrawRectangleRec(GetWindowBounds(),bgColor);

    static Vector2 topLeft = {};
    static Vector2 size    = {};

    Rectangle menuTarget = {
        topLeft.x,topLeft.y,
        size.x, size.y
    };
    DrawRectangleRec(menuTarget, BLACK);
    DrawRectangleLinesEx(menuTarget, 4.f, DARKGRAY);

    size = {};
    DrawPopButton(topLeft,"Reload",true,&size);
    DrawPopButton(topLeft,"Quit",false,&size);
    DrawPopButton(topLeft,"== DEV-TOOLS ==",false,&size,false);
    DrawPopButton(topLeft,"Show/hide console",false,&size);
    DrawPopButton(topLeft,"Show/hide editor",false,&size);
    DrawPopButton(topLeft,"oh god, please change this default font",false,&size,false);

    // apply bottom padding also
    size.y += PADDING * 2;

    // calculate bounds for next frame
    topLeft.x = Window.gameSize.x * 0.5f - size.x * 0.5f;
    topLeft.y = Window.gameSize.y * 0.5f - size.y * 0.5f;
}
