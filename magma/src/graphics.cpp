#include "magma.h"
#include "magma_entity.hpp"
#include <magma_gui.hpp>

#define Win Window
MagmaWindow Window = { 0 };

void InitMagmaWindowEx(int gameWidth, int gameHeight, int winWidth, int winHeight, const char* title) {
    Win.gameSize = { (float)gameWidth, (float)gameHeight };
    Win.winSize = { (float)winWidth, (float)winHeight };
    Win.unscaled = false;
    Win.timeScale = 1.f;

    assert(gameWidth > 0 && gameHeight > 0 &&
        winWidth > 0 && winHeight > 0);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(gameWidth, gameHeight, title);
    SetWindowMinSize(winWidth / 2, winHeight / 2);
    SetWindowPosition((GetMonitorWidth(0) - winWidth) / 2, (GetMonitorHeight(0) - winHeight) / 2);
    SetWindowSize(winWidth, winHeight);

    SetExitKey(KEY_DELETE);

    // Render texture initialization, used to hold the rendering result so we can easily resize it
    Win.renderTarget = LoadRenderTexture(gameWidth, gameHeight);
    SetTextureFilter(Win.renderTarget.texture, TEXTURE_FILTER_POINT);  // Texture scale filter to use
}

void InitMagmaWindow(int winWidth, int winHeight, const char* title) {
    InitMagmaWindowEx(winWidth, winHeight, winWidth, winHeight, title);
}

void BeginMagmaDrawing() {
    Win.scale = MIN((float)GetScreenWidth() / Win.gameSize.x,
        (float)GetScreenHeight() / Win.gameSize.y);

    // Update virtual mouse (clamped mouse value behind game screen)
    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse = { 0 };
    Win.scaledMouse.x = (mouse.x - (GetScreenWidth() - (Win.gameSize.x * Win.scale)) * 0.5f) / Win.scale;
    Win.scaledMouse.y = (mouse.y - (GetScreenHeight() - (Win.gameSize.y * Win.scale)) * 0.5f) / Win.scale;
    virtualMouse = Vector2Clamp(virtualMouse, { 0, 0 }, { (float)Win.gameSize.x, (float)Win.gameSize.y });

    BeginTextureMode(Win.renderTarget);
    ClearBackground(BLACK);

    // ....
}

void EndMagmaDrawing() {

    // ....

    UpdateAndRenderEditor();
    UpdateAndRenderEditorGUI();

    // TODO:
    // UpdateAndRenderPauseMenu({ 0,0,0,50 });
    // UpdateAndRenderInputBoxes();

    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);     // Clear screen background

    Vector2 topLeft = GetWindowTopLeft();

    // Draw render texture to screen, properly scaled
    DrawTexturePro(Win.renderTarget.texture, { 0.0f, 0.0f, (float)Win.renderTarget.texture.width, (float)-Win.renderTarget.texture.height },
        { topLeft.x, topLeft.y, (float)Win.gameSize.x * Win.scale, (float)Win.gameSize.y * Win.scale }, { 0, 0 }, 0.0f, WHITE);
}

float GetTimeScale() {
    return Window.timeScale;
}
void SetTimeScale(float scale) {
    Window.timeScale = scale;
}

void CloseMagmaWindow() {
    UnloadRenderTexture(Win.renderTarget);
    CloseSystemConsole();
    CloseWindow();
}

Rectangle GetScreenBounds() {
    return {
        0,0,
        (float)GetScreenWidth(),
        (float)GetScreenHeight()
    };
}

Rectangle GetWindowBounds() {
    return {
        0,0,
        (float)Win.gameSize.x,
        (float)Win.gameSize.y
    };
}

Vector2 GetWindowTopLeft() {
    return {
        (GetScreenWidth() - ((float)Win.gameSize.x * Win.scale)) * 0.5f,
        (GetScreenHeight() - ((float)Win.gameSize.y * Win.scale)) * 0.5f
    };
}

Vector2 GetWindowCenter() {
    return {
        Win.gameSize.x * 0.5f,
        Win.gameSize.y * 0.5f,
    };
}

Ray GetWindowMouseRay(Camera3D camera) {
    Vector2 mouse = Win.scaledMouse;
    mouse = Vector2Scale(mouse, Win.scale);
    mouse = Vector2Add(mouse, GetWindowTopLeft());
    return GetMouseRay(mouse, camera);
}

// no idea why this works but it does
Vector2 GetWindowMousePositionEx(Camera2D camera) {
    Vector2 mouse = Win.scaledMouse;
    Vector2 world = GetScreenToWorld2D(mouse, camera);
    //world = Vector2Subtract(world, GetWindowTopLeft());
    return world;
}

Vector2 GetWindowMousePosition() {
    Vector2 mouse = Win.scaledMouse;
    return mouse;
}

void DrawCheckeredBackground(int tileSize, const char* text, Color color, Color altColor, Color highlightColor, Color textColor) {
    int width = Win.unscaled ? GetScreenWidth() : Window.gameSize.x;
    int height = Win.unscaled ? GetScreenHeight() : Window.gameSize.y;

    float offset = GetTime() * tileSize;

    // draw live grid
    int xx = 0; int yy = 0;
    for (int x = -offset; x < width; x += tileSize) {
        for (int y = -offset; y < height; y += tileSize) {
            Color col = (xx++ + yy) % 2 == 0 ? color : altColor;
            DrawRectangle(x, y, tileSize, tileSize, col);
        }
        xx = 0;
        yy++;
    }
    DrawRectangleGradientV(0, 0, width, height + abs(sin(GetTime()) * 100), BLANK, highlightColor);

    // draw text
    if (text != NULL && text != "") {
        Vector2 pos = Vector2Subtract({ width * 0.5f,height * 0.5f }, Vector2Scale(MeasureTextEx(GetFontDefault(), text, 28, 2), 0.5f));
        DrawTextEx(GetRetroFont(), text, pos, 28, 2, textColor);
    }
}

void DrawBoundingBox2D(BoundingBox2D bounds, Color tint) {
    Rectangle rect = BoundingBox2DToRect(bounds);
    DrawRectangleLinesEx(rect, 1.f, tint);
}

#define DEFAULT_FONT_SIZE 18
void DrawRetroText(const char* text, int posX, int posY, Color color) {
    DrawRetroTextPro(text, { (float)posX, (float)posY }, DEFAULT_FONT_SIZE, color);
}

void DrawRetroTextV(const char* text, Vector2 pos, Color color) {
    DrawRetroTextPro(text, pos, DEFAULT_FONT_SIZE, color);
}

void DrawRetroTextEx(const char* text, int posX, int posY, int fontSize, Color color) {
    DrawRetroTextPro(text, { (float)posX, (float)posY }, fontSize, color);
}

void DrawRetroTextPro(const char* text, Vector2 pos, int fontSize, Color color) {
    Font font = GetRetroFont();
    DrawTextEx(font, text, pos, (float) DEFAULT_FONT_SIZE, 1.f, color);
}

Vector2 MeasureRetroText(const char* text) {
    return MeasureRetroTextEx(text, DEFAULT_FONT_SIZE);
}

Vector2 MeasureRetroTextEx(const char* text, int fontSize) {
    Font font = GetRetroFont();
    return MeasureTextEx(font, text, fontSize, 1.f);
}