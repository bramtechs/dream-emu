#include "magma.h"

constexpr int BAR_WIDTH = 250;

static float GridSize = 64.f;

static std::vector<EntityBuilderFunction> Builders;

void RegisterEntityBuilder(EntityBuilderFunction func) {
    Builders.push_back(func);
}

// 3D
bool UpdateAndRenderEditor(Camera3D camera, float delta){
    // TODO not implemented;

    // draw origin

    return false;
}

static void DrawGridCell(Vector2 worldPos, float size, float thick=1.f, Color tint=GRAY){
    Rectangle cellRect = {
        (int) (worldPos.x / size) * (float) size,
        (int) (worldPos.y / size) * (float) size,
        size, size,
    };
    if (worldPos.x < 0) cellRect.x -= size;
    if (worldPos.y < 0) cellRect.y -= size;

    DrawRectangleLinesEx(cellRect, thick, tint);
}

// 2D
bool UpdateAndRenderEditor(Camera2D camera, float delta){

    { // draw origin
        const float LEN = 25.f;
        const float THIC = 2.f;
        DrawLineEx({0, 0}, {LEN, 0}, THIC, RED);
        DrawLineEx({0, 0}, {0, LEN}, THIC, GREEN);
    }

    // draw cursor grid cell
    Vector2 mouse = GetWindowMousePosition(camera);
    DrawGridCell(mouse,GridSize);

    // make grid smaller or bigger
    if (IsKeyPressed(KEY_PAGE_UP)){
        GridSize *= 2;
    }
    if (IsKeyPressed(KEY_PAGE_DOWN)){
        GridSize *= 0.5f;
    }

    return true;
}

void UpdateAndRenderEditorGUI(float delta){
    Color bgColor = ColorAlpha(RED,0.5f);
    DrawRectangle(GetScreenWidth()-BAR_WIDTH,0,BAR_WIDTH,GetScreenHeight(),bgColor);
}
