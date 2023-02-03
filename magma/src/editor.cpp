#include "magma.h"
// writing this was a mistake, I hate GUI programming, it's so bad

enum EditorMode {
    MODE_NORMAL,
    MODE_SPAWN,
    MODE_TEXTURE,
    MODE_DELAY, // hack for pop-up menus to work
};

struct EditorSession {
    bool isOpen = false;
    float gridSize = 64.f;
    EntityID subjectID = 0;
    bool hasSubject = false;
    bool drawGrid = true;
    EditorMode mode = MODE_NORMAL;

    PopMenu actionMenu = PopMenu(FOCUS_LOW);
    PopMenu textureMenu = PopMenu(FOCUS_CRITICAL);
    PopMenu spawnMenu = PopMenu(FOCUS_CRITICAL);

    std::vector<EntityBuilderFunction> builders;
};

// TODO: move to editor_utils.h
static void DrawGridCell(Vector2 worldPos, float size, float thick=1.f, Color tint=GRAY){
    Vector2 pos = Vector2Snap(worldPos, size);
    Rectangle cell = { pos.x, pos.y , size, size };
    DrawRectangleLinesEx(cell, thick, tint);
}

static void DrawAxis(Vector2 pos, float len=20.f, float thick=2.f){
    DrawLineEx(pos, {pos.x + len, pos.y}, thick, RED);
    DrawLineEx(pos, {pos.x, pos.y + len}, thick, GREEN);
}

static bool EditorIs3D = false;
static EditorSession Session = EditorSession();

static float NextModeTime = 0.f;
static EditorMode NextMode = MODE_NORMAL;

// HACK: We need to add an artificial delay when switching editor modes 
// to avoid insta-clicking newly opened popup-menus
// Instead of improving the popup code, this hack is used instead.
static void SwitchMode(EditorMode mode){
    Session.mode = MODE_DELAY;
    NextMode = mode;
    NextModeTime = GetTime() + 0.1f;
    DEBUG("Editor switched mode");
}

static void DoUpdateAndRenderEditor(void* camera, EntityGroup& group, float delta){
    if (IsKeyPressed(KEY_F3)){ // TODO: Debug build only
        ToggleEditor();
    }
    if (!Session.isOpen) return;


    Vector2 mouse = {};
    if (EditorIs3D){
        // TODO: implement
    }else{
        // draw origin
        DrawAxis(Vector2Zero());

        // draw cursor grid cell
        mouse = GetWindowMousePosition(*(Camera2D*)camera);
        DrawGridCell(mouse, Session.gridSize);

        // draw other cells
        // TODO: fix shoddyness
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
        if (EditorIs3D){

        }
        else if (comp.second.type == COMP_SPRITE){
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

void RegisterEntityBuilder(EntityBuilderFunction func) {
    Session.builders.push_back(func);
}

void UpdateAndRenderEditor(Camera3D camera, EntityGroup& group, float delta){
    EditorIs3D = true;
    DoUpdateAndRenderEditor(&camera, group,delta);
}

void UpdateAndRenderEditor(Camera2D camera, EntityGroup& group, float delta){
    EditorIs3D = false;
    DoUpdateAndRenderEditor(&camera, group,delta);
}

void UpdateAndRenderEditorGUI(EntityGroup& group, float delta){
    const int BAR_WIDTH = 350;
    const int FONT_SIZE = 18;

    if (!Session.isOpen) return;

    // Process what mode to be in
    if (NextMode != Session.mode && GetTime() > NextModeTime){
        Session.mode = NextMode;
    }

    Color bgColor = ColorAlpha(RED,0.5f);

    int x = GetScreenWidth()-BAR_WIDTH;
    int y = 0;

    DrawRectangle(x,y,BAR_WIDTH,GetScreenHeight(),bgColor);

    x += 20;
    y += 20;

    // draw selected sprite properties
    if (Session.hasSubject){
        DrawRetroText(TextFormat("Selected Entity: %d",Session.subjectID),x,y,FONT_SIZE,WHITE);
        y += FONT_SIZE + 4;

        if (EditorIs3D) {
            auto base = (Base*) group.GetEntityComponent(Session.subjectID, COMP_BASE);
            BoundingBox b = base->bounds;
            DrawRetroText(TextFormat("Bounds: %f %f %f\n %f %f %f",b.min.x,b.min.y,b.min.z,b.max.x,b.max.y,b.max.z),x,y,FONT_SIZE,WHITE);
        }
        else{ // 2d
            auto sprite = (Sprite*) group.GetEntityComponent(Session.subjectID, COMP_SPRITE);
            BoundingBox2D b = sprite->bounds;
            DrawRetroText(TextFormat("Bounds: %f %f\n %f %f",b.min.x,b.min.y,b.max.x,b.max.y),x,y,FONT_SIZE,WHITE);
        }
        y += FONT_SIZE + 4;
    }

    switch (Session.mode){
        case MODE_NORMAL:
            {
                // action menu
                PopMenu& menu = Session.actionMenu;

                Vector2 panelPos = {
                    GetScreenWidth()-menu.size.x*0.5f,
                    GetScreenHeight()-menu.size.y*0.5f
                };
                menu.RenderPanel();
                if (Session.hasSubject) {
                    menu.DrawPopButton("Delete");

                    const char* suffix = HasDefaultPalette() ? "(paletted)":"";
                    menu.DrawPopButton(TextFormat("Change texture %s",suffix), !EditorIs3D); // 2d only
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
                            {
                                // TODO: temporary
                                auto sprite = (Sprite*) group.GetEntityComponent(Session.subjectID, COMP_SPRITE);
                                sprite->SetCenter(-9999.f,-9999.f);
                            }
                            break;
                        case -1:
                            {
                                // change texture
                                SwitchMode(MODE_TEXTURE);
                            }
                            break;
                        case 0:
                            {
                                SwitchMode(MODE_SPAWN);
                            }
                            break;
                    }
                }
            }
            break;
        case MODE_TEXTURE:
            {
                if (EditorIs3D){
                    SwitchMode(MODE_NORMAL);
                    break;
                }

                PopMenu &menu = Session.textureMenu;

                auto names = GetAssetNames(ASSET_TEXTURE);
                menu.RenderPanel();
                for (int i = 0; i < names.size(); i++){
                    menu.DrawPopButton(names[i].c_str());
                }
                menu.EndButtons();

                // process selection
                int index = 0;
                if (menu.IsButtonSelected(&index)){
                    // change texture of subject
                    assert(Session.hasSubject); 

                    auto sprite = (Sprite*) group.GetEntityComponent(Session.subjectID, COMP_SPRITE);

                    Texture newTexture;
                    if (HasDefaultPalette()){
                        newTexture = RequestIndexedTexture(names[index]);
                    }else{
                        newTexture = RequestTexture(names[index]);
                    }

                    sprite->SetTexture(newTexture);
                    DEBUG("Switched entity texture to %s", names[index].c_str());

                    // go back to default mode
                    SwitchMode(MODE_NORMAL);
                }
            }
            break;
        case MODE_SPAWN:
            {
                PopMenu &menu = Session.spawnMenu;

                // TODO: implement
                static auto names = GetAssetNames(ASSET_TEXTURE);
                menu.RenderPanel();
                menu.DrawPopButton("not implemented");
                menu.EndButtons();

                SwitchMode(MODE_NORMAL);
            }
            break;
        default:
            break;
    }
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
