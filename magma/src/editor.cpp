#include "magma.h"

struct EditorSession {
    bool isOpen;
    float gridSize;
    EntityID subjectID;
    bool hasSubject;
    bool drawGrid;

    std::vector<EntityBuilderFunction> builders;

    EditorSession() : isOpen(false), gridSize(64.f), drawGrid(true) {
    }
};

static EditorSession Session = EditorSession();

void RegisterEntityBuilder(EntityBuilderFunction func) {
    Session.builders.push_back(func);
}

// 3D
void UpdateAndRenderEditor(Camera3D camera, EntityGroup& group, float delta){
    if (IsKeyPressed(KEY_F3)){ // TODO: Debug build only
        ToggleEditor();
    }
    if (!Session.isOpen) return;
    // TODO not implemented;

    // draw origin

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
void UpdateAndRenderEditor(Camera2D camera, EntityGroup& group, float delta){
    if (IsKeyPressed(KEY_F3)){ // TODO: Debug build only
        ToggleEditor();
    }
    if (!Session.isOpen) return;

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
}

bool DrawTextureSelector(){
    static auto names = GetAssetNames(ASSET_TEXTURE);
    static PopMenu menu = PopMenu(FOCUS_CRITICAL);
    menu.RenderPanel();
    for (int i = 0; i < names.size(); i++){
        menu.DrawPopButton(names[i].c_str());
    }
    menu.EndButtons();
    return true;
}

constexpr int BAR_WIDTH = 280;
constexpr int FONT_SIZE = 18;
void UpdateAndRenderEditorGUI(EntityGroup& group, float delta){
    if (!Session.isOpen) return;

    Color bgColor = ColorAlpha(RED,0.5f);

    int x = GetScreenWidth()-BAR_WIDTH;
    int y = 0;

    DrawRectangle(x,y,BAR_WIDTH,GetScreenHeight(),bgColor);

    x += 20;
    y += 20;

    Sprite* sprite = NULL;
    if (Session.hasSubject){
        DrawRetroText(TextFormat("Selected Entity: %d",Session.subjectID),x,y,FONT_SIZE,WHITE);
        y += FONT_SIZE + 4;

        sprite = (Sprite*) group.GetEntityComponent(Session.subjectID, COMP_SPRITE);
        BoundingBox2D b = sprite->bounds;
        DrawRetroText(TextFormat("Bounds: %f %f\n       %f %f",b.min.x,b.min.y,b.max.x,b.max.y),x,y,FONT_SIZE,WHITE);
        y += FONT_SIZE + 4;
    }

    static PopMenu menu = PopMenu(FOCUS_LOW);
    Vector2 panelPos = {
        GetScreenWidth()-menu.size.x*0.5f,
        GetScreenHeight()-menu.size.y*0.5f
    };

    menu.RenderPanel();
    if (Session.hasSubject) {
        menu.DrawPopButton("Delete");
        menu.DrawPopButton("Change texture");
    }
    menu.DrawPopButton("Spawn entity");
    menu.EndButtons(panelPos);

    int index = 0;
    if (menu.IsButtonSelected(&index)){
        if (Session.hasSubject){
            index -= 2;
        }
        switch (index){
            case -2:
                // TODO: implement actual deletion
                if (sprite) sprite->SetCenter(-9999.f,-9999.f);
                break;
            case -1:
                // change texture
                break;
            case 0:
                INFO("spawn subject");
                break;
        }
    }
    DrawTextureSelector();
}

bool EditorIsOpen(){
    return Session.isOpen;
}

void OpenEditor(){
    Session.isOpen = true;
}

void CloseEditor(){
    Session.isOpen = false;
}

bool ToggleEditor(){
    Session.isOpen = !Session.isOpen;
    return Session.isOpen;
}
