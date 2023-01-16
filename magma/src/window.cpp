#include "window.h"

// adapted from core_window_letterbox, thank you ~!

typedef struct {
    Vector2 gameSize;
    Vector2 winSize;
    RenderTexture2D renderTarget;

    float scale;
    Vector2 virtualMouse;

} MagmaWindow;

static MagmaWindow Win = { 0 };

void InitMagmaWindow(int gameWidth, int gameHeight, int winWidth, int winHeight, const char* title){
    Win.gameSize = (Vector2) {gameWidth, gameHeight};
    Win.winSize = (Vector2) { winWidth, winHeight};

    assert(gameWidth > 0 && gameHeight > 0 &&
           winWidth > 0 && winHeight > 0);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(gameWidth,gameHeight,title);
    SetWindowMinSize(winWidth/2,winHeight/2);
    SetWindowPosition((GetMonitorWidth(0) - winWidth) / 2, (GetMonitorHeight(0) - winHeight) / 2);
    SetWindowSize(winWidth,winHeight);

    SetExitKey(KEY_DELETE);

    // Render texture initialization, used to hold the rendering result so we can easily resize it
    Win.renderTarget = LoadRenderTexture(gameWidth, gameHeight);
    SetTextureFilter(Win.renderTarget.texture, TEXTURE_FILTER_POINT);  // Texture scale filter to use
}

void BeginMagmaDrawing(){
    Win.scale = MIN((float)GetScreenWidth()/Win.gameSize.x,
               (float)GetScreenHeight()/Win.gameSize.y);

    // Update virtual mouse (clamped mouse value behind game screen)
    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse = { 0 };
    Win.virtualMouse.x = (mouse.x - (GetScreenWidth() - (Win.gameSize.x*Win.scale))*0.5f)/Win.scale;
    Win.virtualMouse.y = (mouse.y - (GetScreenHeight() - (Win.gameSize.y*Win.scale))*0.5f)/Win.scale;
    virtualMouse = Vector2Clamp(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float)Win.gameSize.x, (float)Win.gameSize.y });

    BeginTextureMode(Win.renderTarget);
        ClearBackground(BLACK);

        // ....
}

void EndMagmaDrawing(){

    // ....

    EndTextureMode();
    
    BeginDrawing();
        ClearBackground(BLACK);     // Clear screen background

        float left = GetLeftMagmaWindowOffset();
        float top = GetTopMagmaWindowOffset();

        // Draw render texture to screen, properly scaled
        DrawTexturePro(Win.renderTarget.texture, (Rectangle){ 0.0f, 0.0f, (float)Win.renderTarget.texture.width, (float)-Win.renderTarget.texture.height },
                       (Rectangle){ left, top, (float)Win.gameSize.x*Win.scale, (float)Win.gameSize.y*Win.scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

void CloseMagmaWindow(){
    UnloadRenderTexture(Win.renderTarget);
    CloseWindow();
}

float GetMagmaScaleFactor(){
    return Win.scale;
}

float GetLeftMagmaWindowOffset(){
    return (GetScreenWidth() - ((float)Win.gameSize.x*Win.scale))*0.5f;
}

float GetTopMagmaWindowOffset(){
    return (GetScreenHeight() - ((float)Win.gameSize.y*Win.scale))*0.5f;
}

Vector2 GetMagmaGameSize(){
    return Win.gameSize;
}

Vector2 GetScaledMousePosition(){
    return Win.virtualMouse;
}

Ray GetWindowMouseRay(Camera camera){
    Vector2 mouse = GetScaledMousePosition();

    // TODO do some terribleness for this to work with letterboxing
    // TODO turn into own api function
    mouse = Vector2Scale(mouse,GetMagmaScaleFactor());
    mouse.x += GetLeftMagmaWindowOffset();
    mouse.y += GetTopMagmaWindowOffset();

    return GetMouseRay(mouse,camera);
}
