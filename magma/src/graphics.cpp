#include "magma.h"

#define Win Window
MagmaWindow Window = {0};

void InitMagmaWindow(int gameWidth, int gameHeight, int winWidth, int winHeight, const char* title) {
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
    Win.winSize = { (float)winWidth, (float)winHeight };
    Win.gameSize = Win.winSize;
    Win.unscaled = true;

    assert(winWidth > 0 && winHeight > 0);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(winWidth, winHeight, title);
    SetWindowMinSize(winWidth / 2, winHeight / 2);
    SetWindowPosition((GetMonitorWidth(0) - winWidth) / 2, (GetMonitorHeight(0) - winHeight) / 2);
    SetWindowSize(winWidth, winHeight);

    SetExitKey(KEY_DELETE);

    // Render texture initialization, used to hold the rendering result so we can easily resize it
    Win.renderTarget = LoadRenderTexture(winWidth, winHeight);
    SetTextureFilter(Win.renderTarget.texture, TEXTURE_FILTER_POINT);  // Texture scale filter to use
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

    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);     // Clear screen background

    Vector2 topLeft = GetWindowTopLeft();

    // Draw render texture to screen, properly scaled
    DrawTexturePro(Win.renderTarget.texture, { 0.0f, 0.0f, (float)Win.renderTarget.texture.width, (float)-Win.renderTarget.texture.height },
        { topLeft.x, topLeft.y, (float)Win.gameSize.x * Win.scale, (float)Win.gameSize.y * Win.scale }, { 0, 0 }, 0.0f, WHITE);
}

void SetDefaultPalette(Palette palette) {
    Window.defaultPalette = palette;
    Window.hasDefaultPalette = true;
}

void ClearDefaultPalette() {
    Window.hasDefaultPalette = false;
}

static std::string CurrentPaletteName = "";
void BeginPaletteMode(Palette palette){
    Shader shader = RequestShader("frag_core_palette_switch");

    std::string palName = std::string(palette.name,64);
    if (CurrentPaletteName != palName){
        // put colors into shader if needed
        int paletteLoc = GetShaderLocation(shader, "palette");
        SetShaderValueV(shader, paletteLoc, palette.colors, SHADER_UNIFORM_IVEC3, COLORS_PER_PALETTE);

        CurrentPaletteName = palName;
    }

    BeginShaderMode(shader);

    // ------
}

void EndPaletteMode(){
    // ------
    EndShaderMode();
}

float GetTimeScale(){
    return Window.timeScale;
}
void SetTimeScale(float scale){
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

Vector2 GetWindowCenter(){
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
Vector2 GetWindowMousePosition(Camera2D camera) {
    Vector2 mouse = Win.scaledMouse;
    Vector2 world = GetScreenToWorld2D(mouse, camera);
    //world = Vector2Subtract(world, GetWindowTopLeft());
    return world;
}

Color Palette::GetIndexColor(int index) {
    if (index == -1){
        return BLANK;
    }
    Color result = {
        (unsigned char) Clamp(index, 0, 255),
        0,
        0,
        255
    };
    return result;
}

Color Palette::GetColor(int index) {
    if (index < 0 || index >= COLORS_PER_PALETTE) {
        WARN("Could not get color indexed %d from palette %s", index, name);
        return {255,0,255,255};
    }
    Color result = {
        (unsigned char) colors[index * 3 + 0],
        (unsigned char) colors[index * 3 + 1],
        (unsigned char) colors[index * 3 + 2],
        255
    };
    return result;
}

int Palette::MapColor(Color color) {
    if (color.a == 0){
        return -1;
    }

    for (int i = 0; i < COLORS_PER_PALETTE; i++) {
        if (colors[i * 3 + 0] == color.r &&
            colors[i * 3 + 1] == color.g &&
            colors[i * 3 + 2] == color.b) {
            return i;
        }
    }
    WARN("Could not map color into palette %s", name);
    return 0;
}

int Palette::MapColorLoosely(Color color) {
    if (color.a == 0){
        return -1;
    }

    Vector3 inCol = {
        (float) color.r,
        (float) color.g,
        (float) color.b
    };

    int closestID = 0;
    float closestDiff = FLT_MAX;
    for (int i = 0; i < COLORS_PER_PALETTE; i++) {
        Vector3 palCol{
            (float) colors[i * 3 + 0],
            (float) colors[i * 3 + 1],
            (float) colors[i * 3 + 2]
        };
        float diff = Vector3DistanceSqr(palCol, inCol);
        if (diff < closestDiff) {
            closestDiff = diff;
            closestID = i;
        }
    }
    return closestID;
}

void Palette::MapImage(Image img) {
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Color origColor = GetImageColor(img, x, y);
            int i = MapColorLoosely(origColor);
            Color newColor = GetIndexColor(i);
            ImageDrawRectangle(&img, x,y,1,1, newColor);
        }
    }
}

void Palette::DrawPreview(Rectangle region) {
    const int DEV = 16;

    int size = region.width / DEV;
    int i = 0;
    for (int y = 0; y < DEV; y++) {
        for (int x = 0; x < DEV; x++) {
            Color col = GetColor(i);
            DrawRectangle(region.x+x*size, region.y+y*size, size, size, col);
            i++;
        }
    }
}

void DrawCheckeredBackground(int tileSize, const char* text, Color color, Color altColor, Color highlightColor, Color textColor) {
    int width = Win.unscaled ? GetScreenWidth() : Window.gameSize.x;
    int height = Win.unscaled ? GetScreenHeight() : Window.gameSize.y;

    float offset = GetTime()*tileSize;

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
    DrawRectangleGradientV(0, 0, width, height+abs(sin(GetTime())*100), BLANK, highlightColor);

    // draw text
    if (text != NULL && text != "") {
        Vector2 pos = Vector2Subtract({ width * 0.5f,height * 0.5f }, Vector2Scale(MeasureTextEx(GetFontDefault(), text, 28, 2), 0.5f));
        DrawTextEx(GetRetroFont(), text, pos, 28, 2, textColor);
    }
}

void DrawBoundingBox(BoundingBox2D bounds, Color tint) {
    Rectangle rect = BoundingBoxToRect(bounds);
    DrawRectangleLinesEx(rect, 1.f, tint);
}

void DrawRetroText(const char* text, int posX, int posY, int fontSize, Color color){
    Font font = GetRetroFont();
    DrawTextEx(font, text, {(float) posX, (float) posY}, (float) fontSize, 1.f, color);
}

Vector2 MeasureRetroText(const char* text, int fontSize){
    Font font = GetRetroFont();
    return MeasureTextEx(font, text, fontSize, 1.f);
}
