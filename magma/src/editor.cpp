#include "magma.h"
// writing this was a mistake, I hate GUI programming, it's so bad
// TODO: invent serialization/reflection or something idk.

static Description DescribeComponentBase(void* data){
    auto base = (Base*) data;
    BoundingBox b = base->bounds;
    return { STRING(Base), TextFormat("Bounds: %f %f %f\n %f %f %f",b.min.x,b.min.y,b.min.z,b.max.x,b.max.y,b.max.z), RED };
}

static Description DescribeComponentSprite(void* data){
    auto sprite = (Sprite*) data;
    Vector2 center = sprite->center();
    BoundingBox2D b = sprite->bounds;
    return { STRING(Sprite), TextFormat("Center: %f %f\nBounds: %f %f\n %f %f",center.x,center.y,b.min.x,b.min.y,b.max.x,b.max.y), SKYBLUE };
}

static Description DescribeComponentModelRenderer(void* data){
    auto renderer = (ModelRenderer*) data;
    return { STRING(ModelRenderer), TextFormat("Model: %s\nAccurate: %d\nOffset: %f %f %f",
                renderer->model,renderer->accurate,
                renderer->offset.x,renderer->offset.y,renderer->offset.z), PINK
    };
}

static Description DescribeComponentPhysicsBody(void* data){
    auto phys = (PhysicsBody*) data;
    if (phys->body){
        b2Vec2 pos = phys->body->GetPosition();
        b2Vec2 vel = phys->body->GetLinearVelocity();
        float mass = phys->body->GetMass();
        float inertia = phys->body->GetInertia();

        return { STRING(PhysicsBody), TextFormat("Phys Pos: %f %f\nVel: %f %f\nDynamic: %d\nMass: %f kg\nInertia: %f",
                    pos.x,pos.y,vel.x,vel.y,phys->dynamic,mass,inertia), PURPLE
        };
    }
    return { STRING(PhysicsBody), "Not initialized!" };
}

static Description DescribeComponentAnimationPlayer(void* data){
    auto anim = (AnimationPlayer*) data;
    return { STRING(AnimationPlayer), TextFormat("Frame: %d\nAnim: %s\nFPS: %f",
            abs(anim->curFrame),anim->curAnim.name.c_str(),anim->curAnim.fps), YELLOW 
    };
}

static Description DescribeComponentPlatformerPlayer(void* data){
    auto player = (PlatformerPlayer*) data;
    const char* poseName = PlayerPoseNames[player->pose];
    return { STRING(PlatformerPlayer), TextFormat("Pose: %s",poseName), GREEN };
}

enum EditorMode {
    MODE_NORMAL,
    MODE_SPAWN,
    MODE_TEXTURE,
    MODE_DELAY, // hack for pop-up menus to work
};

struct EditorSession {
    bool isOpen = false;
    float gridSize = PIXELS_PER_UNIT;
    EntityID subjectID = 0;
    bool hasSubject = false;
    bool drawGrid = true;
    EditorMode mode = MODE_NORMAL;

    PopMenu actionMenu = PopMenu(FOCUS_LOW);
    PopMenu textureMenu = PopMenu(FOCUS_CRITICAL);
    PopMenu spawnMenu = PopMenu(FOCUS_CRITICAL);

    std::vector<EntityBuilderFunction> builders;
    std::unordered_map<ItemType, ComponentDescriptor> descriptors;

    EditorSession(){
        // TODO: handle duplicates
        // TODO: turn into macro
        RegisterComponentDescriptor(COMP_BASE, DescribeComponentBase);
        RegisterComponentDescriptor(COMP_SPRITE, DescribeComponentSprite);
        RegisterComponentDescriptor(COMP_MODEL_RENDERER, DescribeComponentModelRenderer);
        RegisterComponentDescriptor(COMP_ANIM_PLAYER, DescribeComponentAnimationPlayer);
        RegisterComponentDescriptor(COMP_PLAT_PLAYER, DescribeComponentPlatformerPlayer);
        RegisterComponentDescriptor(COMP_PHYS_BODY, DescribeComponentPhysicsBody);
    }
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

    if (IsKeyPressed(KEY_KP_ADD)){
        SetTimeScale(GetTimeScale()*1.1f);
    }
    if (IsKeyPressed(KEY_KP_SUBTRACT)){
        SetTimeScale(GetTimeScale()*0.9f);
    }
    if (IsKeyPressed(KEY_HOME)){
        SetTimeScale(1.f);
    }

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
            Color col = fabs(Session.gridSize - PIXELS_PER_UNIT) < EPSILON ? ColorAlpha(RED,0.5f) : ColorAlpha(LIGHTGRAY,0.5f); 
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
                    SetEntityCenter(Session.subjectID,target);
                }
                else {
                    isDragging = false;
                }

                auto phys = (PhysicsBody*) group.GetEntityComponent(comp.first, COMP_PHYS_BODY);
                if (phys != NULL){
                    // brake sprite when pressing spacebar
                    if (IsKeyPressed(KEY_BACKSPACE) && phys->body) {
                        phys->body->SetLinearVelocity({0.f,0.f});
                        DEBUG("Killed velocity");
                    }

                    Color col = ColorAlpha(RED,0.5f);
                    if (!phys->dynamic){
                        // draw cross if static
                        DrawLineV(sprite->bounds.min, sprite->bounds.max, col);
                        DrawLineV(Vector2Add(sprite->bounds.min,{0.f,sprite->size().y}),
                                  Vector2Subtract(sprite->bounds.max,{0.f,sprite->size().y}),
                                  col);
                    }
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
        else if (comp.second.type == COMP_PHYS_BODY){
            auto phys = (PhysicsBody*) comp.second.data;

            if (phys->initialized){
                Color col = ColorAlpha(RED,0.5f);

                // draw each fixture's shape
                b2Fixture* next = phys->body->GetFixtureList();
                while (next) {
                    b2Shape* shape = next->GetShape();
                    Vector2 worldPos = *(Vector2*)&phys->body->GetPosition();
                    switch (shape->GetType()){
                        case b2Shape::Type::e_polygon:
                            {
                                assert(sizeof(b2Vec2) == sizeof(Vector2));
                                auto poly = (b2PolygonShape*) shape;
                                Vector2 vertCpy[8];
                                for (int i = 0; i < 8; i++) {
                                    Vector2 origVert = *(Vector2*) & poly->m_vertices[i];
                                    Vector2 scled = Vector2Add(origVert, worldPos);
                                    scled = Vector2Scale(scled,PIXELS_PER_UNIT);
                                    vertCpy[i] = scled;
                                }
                                DrawLineStrip(vertCpy, poly->m_count, col);
                            }
                            break;
                        default:
                            // TODO: implement other shapes
                            break;
                    }
                    next = next->GetNext();
                }
            }
        }
    }
}

void RegisterEntityBuilder(EntityBuilderFunction func) {
    Session.builders.push_back(func);
}

void RegisterComponentDescriptor(ItemType type, ComponentDescriptor func) {
    Session.descriptors.insert({type, func});
}

Description DescribeComponent(CompContainer cont) {
    Description desc;
    try
    {
        ComponentDescriptor descriptor = Session.descriptors.at(cont.type);
        desc = (*descriptor)(cont.data);
    }
    catch(const std::out_of_range &e)
    {
        desc.typeName = TextFormat("Component %d",cont.type);
        desc.info = "No further info.";
        desc.color = LIGHTGRAY;
    }
    return desc;
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
    const int BAR_WIDTH = 420;
    const int FONT_SIZE = 18;

    if (!Session.isOpen) return;

    // Process what mode to be in
    if (NextMode != Session.mode && GetTime() > NextModeTime){
        Session.mode = NextMode;
    }

    DrawFPS(20,20);

    // draw timescale if non-standard
    float scale = GetTimeScale();
    if (abs(scale - 1.f) > EPSILON){
        DrawRetroText(TextFormat("TIME SCALE %.3f\nDELTA %f",scale,GetFrameTime()),20,40,18,RED);
    }

    Color bgColor = ColorAlpha(BLACK,0.5f);

    int x = GetScreenWidth()-BAR_WIDTH;
    int y = 0;

    DrawRectangle(x,y,BAR_WIDTH,GetScreenHeight(),bgColor);

    x += 20;
    y += 20;

    // draw selected sprite properties
    const char* header = TextFormat("Selected Entity: %d\n=== Components ====",Session.subjectID);
    DrawRetroText(header,x,y,FONT_SIZE,WHITE);
    y += MeasureRetroText(header,FONT_SIZE).y + 20;

    if (Session.hasSubject){
        // collect all components of id
        for (const auto &cont : group.GetEntityComponents(Session.subjectID)){
            // call descriptor to describe the component where dealing with, (fancy toString() function)
            auto desc = DescribeComponent(cont);
            const char* format = TextFormat("--> %s\n%s", desc.typeName.c_str(), desc.info.c_str());
            DrawRetroText(format,x,y,FONT_SIZE,desc.color);
            y += MeasureRetroText(format,FONT_SIZE).y + 10;
        }
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
                                SetEntityCenter(Session.subjectID,-9999.f,-9999.f);
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
