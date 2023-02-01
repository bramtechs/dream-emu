#include "magma.h"

struct EditorSession {
    float gridSize;
    EntityID subjectID;
    bool hasSubject;
    bool drawGrid;

    std::vector<EntityBuilderFunction> builders;

    EditorSession() : gridSize(64.f), drawGrid(true) {
    }
};

static EditorSession Session = EditorSession();

void RegisterEntityBuilder(EntityBuilderFunction func) {
    Session.builders.push_back(func);
}

// 3D
bool UpdateAndRenderEditor(Camera3D camera, EntityGroup& group, float delta){
    // TODO not implemented;

    // draw origin

    return false;
}

static void DrawGridCell(Vector2 worldPos, float size, float thick=1.f, Color tint=GRAY){
    Vector2 pos = Vector2Snap(worldPos, size);
    Rectangle cell = { pos.x, pos.y , size, size };
    DrawRectangleLinesEx(cell, thick, tint);
}

static void DrawAxis(Vector2 pos, float len=20.f, float thick=2.f){
    DrawLineEx(pos, {pos.x + len, pos.y}, thick, RED);
    DrawLineEx(pos, {pos.x, pos.y + len}, thick, GREEN);
}

// 2D
bool UpdateAndRenderEditor(Camera2D camera, EntityGroup& group, float delta){

    // draw origin
    DrawAxis(Vector2Zero());

    // draw cursor grid cell
    Vector2 mouse = GetWindowMousePosition(camera);
    DrawGridCell(mouse, Session.gridSize);

    // draw other cells
    // TODO fix shoddyness
    if (Session.drawGrid){
        Color col = ColorAlpha(LIGHTGRAY,0.5f);
        for (int y = -3; y < 3; y++){
            for (int x = -3; x < 3; x++){
                Vector2 offset = { x, y };
                Vector2 cellPos = Vector2Add(mouse,Vector2Add(Vector2Scale(offset,Session.gridSize),offset));
                DrawGridCell(cellPos, Session.gridSize, 1.f, col);
            }
        }
    }

    // make grid smaller or bigger
    if (IsKeyPressed(KEY_PAGE_UP)){
        Session.gridSize *= 2;
    }
    if (IsKeyPressed(KEY_PAGE_DOWN)){
        Session.gridSize *= 0.5f;
    }

    // index on click
    for (const auto &comp : group.comps){
        if (comp.second.type == COMP_SPRITE){

            auto sprite = (Sprite*) comp.second.data;
            Rectangle rect = sprite->region();

            if (comp.first == Session.subjectID && Session.hasSubject) {
                // overlay selected sprite
                Color overlay = ColorAlpha(GREEN,0.5f);
                DrawRectangleRec(rect, overlay);
                DrawAxis(sprite->center());

                static bool isDragging = false;
                static Vector2 dragPos = {};
                static Vector2 dragOffset = {};
                static bool horizontal = false;
                
                // start drag
                if (CheckCollisionPointRec(mouse,rect)){
                    if (IsMouseButtonPressed(0)){
                        isDragging = true;

                        // determine direction
                        dragOffset = Vector2Subtract(mouse,sprite->center());
                        dragPos = sprite->center();
                        horizontal = fabs(dragOffset.x) > fabs(dragOffset.y);
                        DEBUG("start drag %s", horizontal ? "horizontal":"vertical");
                    }
                }

                if (isDragging && IsMouseButtonDown(0)){
                    // move sprite around
                    Vector2 target;
                    if (horizontal){
                        target = {mouse.x-dragOffset.x,dragPos.y};
                        target.x = Vector2Snap(target,Session.gridSize).x + sprite->halfSize().x;
                    }
                    else{
                        target = {dragPos.x,mouse.y-dragOffset.y};
                        target.y = Vector2Snap(target,Session.gridSize).y + sprite->halfSize().y;
                    }
                    sprite->SetCenter(target);
                }
                else {
                    isDragging = false;
                }
            }
            else if (CheckCollisionPointRec(mouse,rect)) {
                float alpha = (sin(GetTime())+1.f)*0.25f+0.5f;
                Color overlay = ColorAlpha(GRAY,alpha);
                DrawRectangleRec(rect, overlay);
                if (IsMouseButtonPressed(0)){
                    Session.hasSubject = true;
                    Session.subjectID = comp.first;
                    break;
                }
            }
        }
    }

    return true;
}

constexpr int BAR_WIDTH = 280;
constexpr int FONT_SIZE = 18;
void UpdateAndRenderEditorGUI(EntityGroup& group, float delta){
    Color bgColor = ColorAlpha(RED,0.5f);

    int x = GetScreenWidth()-BAR_WIDTH;
    int y = 0;

    DrawRectangle(x,y,BAR_WIDTH,GetScreenHeight(),bgColor);

    x += 20;
    y += 20;
    if (Session.hasSubject){
        DrawText(TextFormat("Selected Entity: %d",Session.subjectID),x,y,FONT_SIZE,WHITE);
        y += FONT_SIZE + 4;

        auto sprite = (Sprite*) group.GetEntityComponent(Session.subjectID, COMP_SPRITE);
        BoundingBox2D b = sprite->bounds;
        DrawText(TextFormat("Bounds: %f %f\n       %f %f",b.min.x,b.min.y,b.max.x,b.max.y),x,y,FONT_SIZE,WHITE);
        y += FONT_SIZE + 4;
    }

}
