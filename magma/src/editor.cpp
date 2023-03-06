#include "magma.h"
#include "magma_gui.hpp"
#include "magma_entity.hpp"

typedef void (*UpdateAndDrawMode)(EntityGroup& group, Camera* camera, float delta);
typedef void (*UpdateAndDrawModeGUI)(EntityGroup& group, Camera* camera, float delta);

typedef EntityID(*EntityBuilderFunction)(EntityGroup& group, Vector3 pos);

struct Description {
    std::string typeName;
    std::string info;
    Color color = WHITE;
};
typedef Description(*ComponentDescriptor)(void* data);

enum EditorMode {
    MODE_NORMAL,
    MODE_SPAWN,
    MODE_HITBOX,
    MODE_TILE,
    MODE_TEXTURE,
    MODE_DELAY, // hack for pop-up menus to work
};

struct EditorModeInfo {
    std::string description;
    UpdateAndDrawMode func;
    UpdateAndDrawModeGUI gui;
};

struct EditorSession {
    bool isOpen = false;
    float gridSize = PIXELS_PER_UNIT;
    EntityID subjectID = 0;
    bool hasSubject = false;
    bool drawGrid = true;
    bool removalMode = false;
    bool includeHitbox = false;
    EditorMode mode = MODE_NORMAL;
    Texture tileBeingDrawn = {};

    std::pair<const char*, EntityBuilderFunction> builderBeingUsed = { "", NULL };

    std::unordered_map<EditorMode, EditorModeInfo> modes;
    std::unordered_map<const char*, EntityBuilderFunction> builders;
    std::unordered_map<ItemType, ComponentDescriptor> descriptors;

    EditorSession();

private:
    inline void LinkMode(EditorMode mode, UpdateAndDrawMode func = NULL,
        UpdateAndDrawModeGUI gui = NULL, std::string desc = "") {
        EditorModeInfo info = {
            desc,
            func,
            gui
        };
        modes.insert({ mode,info });
    }

    void LinkModes();
};

static void DrawAxis(Vector2 pos, float len = 20.f, float thick = 2.f);
static void DrawBox2DBody(PhysicsBody* phys, Color color = GRAY, bool fill = false);
static void SwitchMode(EditorMode mode);
static void DrawGridCell(Vector2 pos, float size, float thick = 1.f, Color tint = GRAY, bool snap = true);
static void DrawGrid(Camera2D camera);
static EntityID SpawnWallBrush(EntityGroup& group, Vector3 pos);
static Description DescribeComponent(CompContainer cont);

static EditorSession Session = EditorSession();
static bool EditorIs3D = false;

static void ProcNormalMode(EntityGroup& group, Camera* camera, float delta) {

    Vector2 mouse = GetWindowMousePositionEx(*(Camera2D*)camera);

    for (const auto& comp : group.comps) {
        if (comp.second.type == COMP_SPRITE) {
            auto sprite = (Sprite*)comp.second.data;

            // HACK: if a sprite has physics body and it is invisible,
            // don't make it interactable in the editor's NORMAL_MODE
            if (group.EntityHasComponent(comp.first, COMP_PHYS_BODY) && !sprite->isVisible) {
                continue;
            }

            Rectangle rect = sprite->region();
            if (comp.first == Session.subjectID && Session.hasSubject) {
                // overlay selected sprite
                Color overlay = ColorAlpha(GREEN, 0.5f);
                DrawRectangleRec(rect, overlay);
                DrawAxis(sprite->center());

                static bool isDragging = false;
                static Vector2 dragPos = {};
                static Vector2 dragOffset = {};
                static bool horizontal = false;

                // start drag
                if (CheckCollisionPointRec(mouse, rect)) {
                    if (IsMouseButtonPressed(0)) {
                        isDragging = true;

                        // determine direction
                        dragOffset = Vector2Subtract(mouse, sprite->center());
                        dragPos = sprite->center();
                        horizontal = fabs(dragOffset.x) > fabs(dragOffset.y);
                        DEBUG("start drag %s", horizontal ? "horizontal" : "vertical");
                    }
                }

                if (isDragging && IsMouseButtonDown(0)) {
                    // move sprite around
                    Vector2 target;
                    if (horizontal) {
                        target = { mouse.x - dragOffset.x,dragPos.y };
                        target.x = Vector2Snap(target, Session.gridSize).x + sprite->halfSize().x;
                    }
                    else {
                        target = { dragPos.x,mouse.y - dragOffset.y };
                        target.y = Vector2Snap(target, Session.gridSize).y + sprite->halfSize().y;
                    }
                    SetEntityCenterV(Session.subjectID, target);
                }
                else {
                    isDragging = false;
                }

                PhysicsBody* phys = NULL;
                if (group.TryGetEntityComponent(comp.first, COMP_PHYS_BODY, &phys)) {
                    // brake sprite when pressing spacebar
                    if (IsKeyPressed(KEY_BACKSPACE) && phys->body) {
                        phys->body->SetLinearVelocity({ 0.f,0.f });
                        DEBUG("Killed velocity");
                    }

                    Color col = ColorAlpha(RED, 0.5f);
                    if (!phys->dynamic) {
                        // draw cross if static
                        DrawLineV(sprite->bounds.min, sprite->bounds.max, col);
                        DrawLineV(Vector2Add(sprite->bounds.min, { 0.f,sprite->size().y }),
                            Vector2Subtract(sprite->bounds.max, { 0.f,sprite->size().y }),
                            col);
                    }
                }

                // draw text if sprite hidden
                if (!sprite->isVisible) {
                    DrawRetroTextEx("hidden", sprite->bounds.min.x, sprite->bounds.min.y, 12, GRAY);
                }
            }
            else if (CheckCollisionPointRec(mouse, rect)) {
                float alpha = (sin(GetTime()) + 1.f) * 0.25f + 0.5f;
                Color overlay = ColorAlpha(GRAY, alpha * 0.5f);
                DrawRectangleRec(rect, overlay);
                if (IsMouseButtonPressed(0)) {
                    Session.hasSubject = true;
                    Session.subjectID = comp.first;
                    break;
                }
            }
        }
        else if (comp.second.type == COMP_PHYS_BODY) {
            auto phys = (PhysicsBody*)comp.second.data;

            if (phys->initialized) {
                Color col = ColorAlpha(RED, 0.5f);
                DrawBox2DBody(phys, col);
            }
        }
    }
}

static void ProcNormalModeGUI(EntityGroup& group, Camera* camera, float delta) {
    // action menu
    static PopMenu menu = PopMenu(FOCUS_LOW);

    Vector2 panelPos = {
        GetScreenWidth() - menu.size.x * 0.5f,
        GetScreenHeight() - menu.size.y * 0.5f
    };

    ButtonTable buttons;
    if (Session.hasSubject) {
        buttons.AddButton("Delete", [&group]() {
            group.DestroyEntity(Session.subjectID);
        Session.hasSubject = false;
            });
        buttons.AddButton("Change texture", []() {
            SwitchMode(MODE_TEXTURE);
            });
    }

    // editor modes
    for (const auto& mode : Session.modes) {
        if (mode.first != MODE_DELAY && mode.first != MODE_NORMAL) {
            buttons.AddButton(mode.second.description.c_str(), [&mode]() {
                SwitchMode(mode.first);
                });
        }
    }

    menu.RenderPanel();
    menu.DrawPopButtons(buttons);
    menu.ProcessSelectedButton(buttons);
    menu.EndButtons(panelPos);
}

static void ProcTextureModeGUI(EntityGroup& group, Camera* camera, float delta) {

    static PopMenu menu = PopMenu();

    auto names = GetFilteredAssetNames(ASSET_TEXTURE);
    menu.RenderPanel();
    for (int i = 0; i < names.count; i++) {
        menu.DrawPopButton(names.entries[i]);
    }
    menu.EndButtons();

    // process selection
    int index = 0;
    if (menu.IsButtonSelected(&index)) {
        // change texture of subject
        assert(Session.hasSubject);

        Sprite* sprite = NULL;
        group.GetEntityComponent(Session.subjectID, COMP_SPRITE, &sprite);

        Texture newTexture = RequestTexture(names.entries[index]);

        sprite->SetTexture(newTexture);
        DEBUG("Switched entity texture to %s", names.entries[index]);

        // go back to default mode
        SwitchMode(MODE_NORMAL);
    }
}

static void ProcHitboxMode(EntityGroup& group, Camera* camera, float delta) {
    // start dragging mouse to place hitbox objects
    Vector2 mouse = GetWindowMousePositionEx(*(Camera2D*)camera);

    Vector2 snapPos = Vector2Snap(mouse, Session.gridSize);
    Vector2 size = { Session.gridSize, Session.gridSize };

    // draw marker at mouse position
    float alpha = (sin(GetTime()) + 1.f) * 0.25f + 0.5f;
    Color color = ColorAlpha(PURPLE, alpha * 0.5f);
    DrawRectangleV(snapPos, size, color);

    if (IsMouseButtonDown(0)) {
        // prevent placing hitbox ontop of an existing one
        // HACK: weird offsetting
        Vector2 checkPos = {
            snapPos.x + Session.gridSize * 0.5f,
            snapPos.y + Session.gridSize * 0.5f
        };

        EntityID touchedID = 0;
        if (group.IsHitboxAtPos(checkPos, &touchedID)) {
            if (Session.removalMode) {
                // remove the hitbox
                group.DestroyEntity(touchedID);
            }
        }
        else if (!Session.removalMode) {
            // HACK: weird offsetting
            Vector2 spawnPos = {
                snapPos.x + Session.gridSize,
                snapPos.y + Session.gridSize
            };
            SpawnWallBrush(group, Vector2ToVector3(spawnPos));
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        SwitchMode(MODE_NORMAL);
    }

    // draw the physics shapes
    Color col = ColorAlpha(ORANGE, 0.8f);
    std::multimap<EntityID, CompContainer> physBodies = group.GetComponents(COMP_PHYS_BODY);
    for (auto& phys : physBodies) {
        auto physBody = (PhysicsBody*)phys.second.data;
        Color color = Session.removalMode ? RED : ORANGE;
        DrawBox2DBody(physBody, color, true);
    }
}

static void ProcHitboxModeGUI(EntityGroup& group, Camera* camera, float delta) {
    static PopMenu menu = PopMenu();

    Vector2 panelPos = {
        GetScreenWidth() - menu.size.x * 0.5f,
        GetScreenHeight() - menu.size.y * 0.5f
    };

    ButtonTable buttons;
    buttons.AddButton(Session.removalMode ? "Draw" : "Delete", []() {
        Session.removalMode = !Session.removalMode;
        });
    buttons.AddButton("Simplify", [&group]() {
        //SimplifyHitboxes(group);
        });
    buttons.AddButton("Exit", [&group]() {
        SwitchMode(MODE_NORMAL);
        });

    menu.RenderPanel(Session.removalMode ? RED : WHITE);
    menu.DrawPopButtons(buttons);
    menu.ProcessSelectedButton(buttons);
    menu.EndButtons(panelPos);
}

static void BuildEntity(EntityGroup& group, Camera* camera, EntityBuilderFunction func) {
    // spawn new entity
    Camera2D cam2d = *(Camera2D*)camera;

    // get snapped pos to place entity
    Vector2 camPos = GetWindowMousePositionEx(cam2d);
    Vector2 snapPos = Vector2Snap(camPos, Session.gridSize);

    (*func)(group, { snapPos.x,snapPos.y,0 });
}

static void BuildTile(EntityGroup& group, Camera* camera, Texture texture, bool includeHitbox) {
    // spawn new entity
    Camera2D cam2d = *(Camera2D*)camera;

    // get snapped pos to place entity
    Vector2 camPos = GetWindowMousePositionEx(cam2d);
    Vector2 snapPos = Vector2Snap(camPos, Session.gridSize);

    if (includeHitbox) {
        Vector2 hitboxPos = {
            snapPos.x + texture.width * 1.0f,
            snapPos.y + texture.height * 1.0f
        };
        SpawnWallBrush(group, Vector2ToVector3(hitboxPos));
    }

    // spawn a bland tile
    EntityID id = group.AddEntity();
    Sprite sprite = Sprite({ snapPos.x, snapPos.y });
    sprite.SetTexture(texture);
    group.AddEntityComponent(id, COMP_SPRITE, sprite, true);
}

static void ProcTileMode(EntityGroup& group, Camera* camera, float delta) {
    // get snapped pos to render ghost
    Camera2D cam2d = *(Camera2D*)camera;
    Vector2 camPos = GetWindowMousePositionEx(cam2d);
    Vector2 snapPos = Vector2Snap(camPos, Session.gridSize);

    bool isHolding = IsMouseButtonDown(0);

    if (Session.tileBeingDrawn.id > 0) { // if any tile texture selected
        Session.gridSize = Session.tileBeingDrawn.width;
        DrawTextureEx(Session.tileBeingDrawn, snapPos, 0.f, 1.f,
            ColorAlpha(WHITE, 0.6f));

        if (isHolding) {
            EntityID touchedID = 0;
            Vector2 checkPos = {
                snapPos.x + Session.tileBeingDrawn.width * 0.5f,
                snapPos.y + Session.tileBeingDrawn.height * 0.5f
            };
            if (group.IsEntityAtPos(checkPos, &touchedID)) {
                if (Session.removalMode) {
                    // remove the tile
                    group.DestroyEntity(touchedID);
                }
            }
            else if (!Session.removalMode) {
                BuildTile(group, camera, Session.tileBeingDrawn, Session.includeHitbox);
            }
        }
    }
    else if (Session.builderBeingUsed.second != NULL) {
        Rectangle rect = {
            snapPos.x, snapPos.y,
            Session.gridSize, Session.gridSize
        };
        const char* text = Session.builderBeingUsed.first;
        Vector2 textPos = MeasureRetroTextEx(text, 12);
        DrawRetroTextEx(text, rect.x + textPos.x * 0.5f, rect.y + textPos.y * 0.5f, 12, BLUE);
        DrawRectangleLinesEx(rect, 3.f, BLUE);
    }
}

static void ProcTileModeGUI(EntityGroup& group, Camera* camera, float delta) {
    static PopMenu menu = PopMenu();

    Vector2 panelPos = {
        GetScreenWidth() - menu.size.x * 0.5f,
        GetScreenHeight() - menu.size.y * 0.5f
    };

    ButtonTable buttons;

    // label all spawners
    buttons.AddSpacer("=== Spawners ===");
    for (const auto& builder : Session.builders) {
        const char* name = builder.first;
        buttons.AddButton(name, [&builder]() {
            Session.builderBeingUsed = builder;
        Session.tileBeingDrawn = {};
            });
    }

    // label all raw tiles
    buttons.AddSpacer("=== Tiles ===");
    static auto tiles = GetTileNames();
    for (int i = 0; i < tiles.count; i++) {
        const char* tile = tiles.entries[i];
        buttons.AddButton(tiles.entries[i], [&tile]() {
            Session.builderBeingUsed = { "", NULL };
        Session.tileBeingDrawn = RequestTexture(tile);
            });
    }

    // extra utilities
    buttons.AddSpacer();
    buttons.AddButton(Session.removalMode ? "Draw" : "Delete", []() {
        Session.removalMode = !Session.removalMode;
        });
    buttons.AddButton(Session.includeHitbox ? "Including hitboxes" : "Not including hitboxes", []() {
        Session.includeHitbox = !Session.includeHitbox;
        });
    buttons.AddButton("Exit", []() {
        SwitchMode(MODE_NORMAL);
        });

    menu.RenderPanel(Session.removalMode ? RED : WHITE);
    menu.DrawPopButtons(buttons);
    menu.ProcessSelectedButton(buttons);

    menu.EndButtons(panelPos);
}

static void ProcSpawnModeGUI(EntityGroup& group, Camera* camera, float delta) {
    static PopMenu menu = PopMenu();

    menu.RenderPanel();

    // cache builder functions
    static std::vector<EntityBuilderFunction> builders;
    if (builders.empty()) {
        for (const auto& builder : Session.builders) {
            builders.push_back(builder.second);
        }
    }

    for (const auto& builder : Session.builders) {
        menu.DrawPopButton(builder.first);
    }

    menu.DrawPopButton("", false, true);
    menu.DrawPopButton("Close");

    int index = 0;
    if (menu.IsButtonSelected(&index))
    {
        // check if last
        if (index >= Session.builders.size())
        {
            SwitchMode(MODE_NORMAL);
        }
        else
        {
            try {
                // spawn new entity
                EntityBuilderFunction func = builders.at(index);
                BuildEntity(group, camera, func);
            }
            catch (const std::out_of_range& e) {
                ERROR("Can't find valid spawn function!");
            }
        }
    }

    menu.EndButtons();
}

void UpdateAndRenderEditor() {
    if (IsKeyPressed(KEY_F3)) { // TODO: Debug build only
        ToggleEditor();
    }

    if (!Session.isOpen) return;

    if (IsKeyPressed(KEY_KP_ADD)) {
        SetTimeScale(GetTimeScale() * 1.1f);
    }
    if (IsKeyPressed(KEY_KP_SUBTRACT)) {
        SetTimeScale(GetTimeScale() * 0.9f);
    }
    if (IsKeyPressed(KEY_HOME)) {
        SetTimeScale(1.f);
    }

    if (EditorIs3D) {
        // TODO: implement
    }
    else {
        // draw origin
        DrawAxis(Vector2Zero());

        // draw other cells
        if (Session.drawGrid) {
            DrawGrid(Group.camera2D);
        }
    }

    // make grid smaller or bigger
    if (IsKeyPressed(KEY_PAGE_UP)) {
        Session.gridSize *= 2;
    }
    if (IsKeyPressed(KEY_PAGE_DOWN)) {
        Session.gridSize *= 0.5f;
    }

    // update and draw editor mode (world pos)
    try {
        EditorModeInfo info = Session.modes.at(Session.mode);
        if (info.func != NULL) {
            float delta = GetFrameTime();
            (*info.func)(Group, &Group.camera3D, delta);
        }
    }
    catch (const std::out_of_range& e) {
    }
}

// HACK: We need to add an artificial delay when switching editor modes 
// to avoid insta-clicking newly opened popup-menus
// Instead of improving the popup code, this hack is used instead.
static float NextModeTime = 0.f;
static EditorMode NextMode = MODE_NORMAL;

static void SwitchMode(EditorMode mode) {
    Session.mode = MODE_DELAY;
    NextMode = mode;
    NextModeTime = GetTime() + 0.1f;
    DEBUG("Editor switched mode");
}

void UpdateAndRenderEditorGUI() {
    const int BAR_WIDTH = 420;
    const int FONT_SIZE = 18;

    if (!Session.isOpen) return;

    // Process what mode to be in
    if (NextMode != Session.mode && GetTime() > NextModeTime) {
        Session.mode = NextMode;
    }

    DrawFPS(20, 20);

    // draw timescale if non-standard
    float scale = GetTimeScale();
    if (abs(scale - 1.f) > EPSILON) {
        DrawRetroTextEx(TextFormat("TIME SCALE %.3f\nDELTA %f", scale, GetFrameTime()), 20, 40, 18, RED);
    }

    Color bgColor = ColorAlpha(BLACK, 0.5f);

    int x = GetScreenWidth() - BAR_WIDTH;
    int y = 0;

    DrawRectangle(x, y, BAR_WIDTH, GetScreenHeight(), bgColor);

    x += 20;
    y += 20;

    // draw selected sprite properties
    const char* header = TextFormat("Entity count: %d\nNext entity: %d\nSelected Entity: %d\n=== Components ====", Group.entityCount, Group.nextEntity, Session.subjectID);
    DrawRetroTextEx(header, x, y, FONT_SIZE, WHITE);
    y += MeasureRetroTextEx(header, FONT_SIZE).y + 20;

    if (Session.hasSubject) {
        // collect all components of id
        for (const auto& cont : Group.GetEntityComponents(Session.subjectID)) {
            // call descriptor to describe the component where dealing with, (fancy toString() function)
            auto desc = DescribeComponent(cont);
            const char* format = TextFormat("--> %s %s\n%s", desc.typeName.c_str(), cont.persistent ? "(PERSIST)" : "", desc.info.c_str());
            DrawRetroTextEx(format, x, y, FONT_SIZE, desc.color);
            y += MeasureRetroTextEx(format, FONT_SIZE).y + 10;
        }
    }

    // update and draw editor mode (gui elements)
    try {
        EditorModeInfo info = Session.modes.at(Session.mode);
        if (info.gui != NULL) {
            (*info.gui)(Group, &Group.camera3D, GetFrameTime());
        }
    }
    catch (const std::out_of_range& e) {
        DrawText("Invalid editor mode!", 50, 50, 16, RED);
    }

    // toggle framerate
    static bool isFPSLocked = false;
    if (IsKeyPressed(KEY_F8)) {
        isFPSLocked = !isFPSLocked;
        SetTargetFPS(isFPSLocked ? 60 : 1000);
    }
}

void EditorSession::LinkModes() {
    LinkMode(MODE_DELAY);
    LinkMode(MODE_NORMAL, ProcNormalMode, ProcNormalModeGUI);
    LinkMode(MODE_TEXTURE, NULL, ProcTextureModeGUI, "Change texture");
    LinkMode(MODE_SPAWN, NULL, ProcSpawnModeGUI, "Spawn entity");
    LinkMode(MODE_HITBOX, ProcHitboxMode, ProcHitboxModeGUI, "Draw hitboxes");
    LinkMode(MODE_TILE, ProcTileMode, ProcTileModeGUI, "Draw tiles");
}

// descriptors

#define REGCOMP(C,F) RegisterComponentDescriptor(C,F)
#define RegisterStockEntityBuilder(F) RegisterEntityBuilderEx(#F,F,true)

void RegisterComponentDescriptor(ItemType type, ComponentDescriptor func) {
    Session.descriptors.insert({ type, func });
}

static Description DescribeComponent(CompContainer cont) {
    Description desc;
    try
    {
        ComponentDescriptor descriptor = Session.descriptors.at(cont.type);
        desc = (*descriptor)(cont.data);
    }
    catch (const std::out_of_range& e)
    {
        desc.typeName = TextFormat("Component %d", cont.type);
        desc.info = "No further info.";
        desc.color = LIGHTGRAY;
    }
    return desc;
}

void RegisterEntityBuilderEx(const char* name, EntityBuilderFunction func,
    bool isStock) {
    // prevent adding same function twice
    for (const auto& builder : Session.builders) {
        if (TextIsEqual(builder.first, name)) {
            ERROR("Already added entity builder function");
            return;
        }
    }

    DEBUG("Registered entity builder with name %s", name);
    Session.builders.insert({ name, func });
}

static Description DescribeComponentSprite(void* data) {
    auto sprite = (Sprite*)data;
    Vector2 center = sprite->center();
    BoundingBox2D b = sprite->bounds;
    bool isVisible = sprite->isVisible;
    return { STRING(Sprite), TextFormat("Center: %f %f\nBounds: %f %f\n %f %f\nVisible: %d",center.x,center.y,b.min.x,b.min.y,b.max.x,b.max.y,isVisible), SKYBLUE };
}

static Description DescribeComponentPhysicsBody(void* data) {
    auto phys = (PhysicsBody*)data;
    if (phys->initialized && phys->body) {
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

static Description DescribeComponentAnimationPlayer(void* data) {
    auto anim = (AnimationPlayer*)data;
    if (anim->curAnim) {
        return { STRING(AnimationPlayer), TextFormat("Frame: %d\nAnim: %s\nFPS: %f",
                abs(anim->curFrame),anim->curAnim->name,anim->curAnim->fps), YELLOW
        };
    }
    else {
        return { STRING(AnimationPlayer), TextFormat("Frame: %d - NO ANIMATION",
                abs(anim->curFrame)), YELLOW
        };
    }
}

static Description DescribeComponentPlatformerPlayer(void* data) {
    auto player = (PlatformerPlayer*)data;
    const char* poseName = GetPlayerPoseNames()[player->pose];
    return { STRING(PlatformerPlayer), TextFormat("Pose: %s",poseName), GREEN };
}

static Description DescribeComponentBase(void* data) {
    auto base = (Base*)data;
    BoundingBox b = base->bounds;
    return { STRING(Base), TextFormat("Bounds: %f %f %f\n %f %f %f",b.min.x,b.min.y,b.min.z,b.max.x,b.max.y,b.max.z), RED };
}

static Description DescribeComponentModelRenderer(void* data) {
    auto renderer = (ModelRenderer*)data;
    return { STRING(ModelRenderer), TextFormat("Model: %s\nAccurate: %d\nOffset: %f %f %f",
                renderer->model,renderer->accurate,
                renderer->offset.x,renderer->offset.y,renderer->offset.z), PINK
    };
}

// spawners
static EntityID SpawnWallBrush(EntityGroup& group, Vector3 pos) {
    EntityID id = group.AddEntity();

    Sprite sprite = Sprite();
    Texture texture = RequestPlaceholderTexture();
    sprite.SetTexture(texture);
    sprite.SetSize(Session.gridSize, Session.gridSize);
    sprite.SetCenter({ pos.x,pos.y });
    sprite.Hide();
    group.AddEntityComponent(id, COMP_SPRITE, sprite);

    PhysicsBody body = PhysicsBody(false);
    group.AddEntityComponent(id, COMP_PHYS_BODY, body);

    return id;
}

// session contructor
EditorSession::EditorSession() {
    REGCOMP(COMP_SPRITE, DescribeComponentSprite);
    REGCOMP(COMP_ANIM_PLAYER, DescribeComponentAnimationPlayer);
    REGCOMP(COMP_PLAT_PLAYER, DescribeComponentPlatformerPlayer);
    REGCOMP(COMP_PHYS_BODY, DescribeComponentPhysicsBody);
    REGCOMP(COMP_BASE, DescribeComponentBase);
    REGCOMP(COMP_MODEL_RENDERER, DescribeComponentModelRenderer);

    // declare stock builder-functions
    RegisterStockEntityBuilder(SpawnWallBrush);

    LinkModes();
}

// utils

static void DrawGridCell(Vector2 pos, float size, float thick, Color tint, bool snap) {
    if (snap) {
        pos = Vector2Snap(pos, size);
    }
    Rectangle cell = { pos.x, pos.y , size, size };
    DrawRectangleLinesEx(cell, thick, tint);
}

static void DrawGrid(Camera2D camera) {
    Vector2 mouse = Vector2Snap(GetWindowMousePositionEx(camera), Session.gridSize);

    // draw cursor grid cell
    DrawGridCell(mouse, Session.gridSize, 1.f, RED, false);

    Color col = fabs(Session.gridSize - PIXELS_PER_UNIT) < EPSILON ? ColorAlpha(WHITE, 0.5f) : ColorAlpha(LIGHTGRAY, 0.5f);
    for (int y = -3; y <= 3; y++) {
        for (int x = -3; x <= 3; x++) {
            Vector2 cellPos = {
                mouse.x + (x * Session.gridSize),
                mouse.y + (y * Session.gridSize),
            };
            DrawGridCell(cellPos, Session.gridSize, 1.f, col, false);
        }
    }
}

static void DrawAxis(Vector2 pos, float len, float thick) {
    DrawLineEx(pos, { pos.x + len, pos.y }, thick, RED);
    DrawLineEx(pos, { pos.x, pos.y + len }, thick, GREEN);
}

static void DrawBox2DBody(PhysicsBody* phys, Color color, bool fill) {
    assert(phys);
    // only draw if box2d body exists
    if (!phys->initialized) {
        return;
    }

    // draw each fixture's shape
    b2Fixture* next = phys->body->GetFixtureList();
    while (next) {
        b2Shape* shape = next->GetShape();
        Vector2 worldPos = *(Vector2*)&phys->body->GetPosition();
        switch (shape->GetType()) {
        case b2Shape::Type::e_polygon:
        {
            assert(sizeof(b2Vec2) == sizeof(Vector2));
            auto poly = (b2PolygonShape*)shape;
            size_t vertexCount = poly->m_count;
            std::vector<Vector2> vertCpy(vertexCount);
            for (int i = 0; i < vertexCount; i++) {
                Vector2 origVert = *(Vector2*)&poly->m_vertices[i];
                Vector2 scled = Vector2Add(origVert, worldPos);
                scled = Vector2Scale(scled, PIXELS_PER_UNIT);
                vertCpy[i] = scled;
            }
            if (fill) {
                // quick and dirty wireframe drawing
                for (int i = 0; i < vertexCount; i++) {
                    for (int j = 0; j < vertexCount; j++) {
                        if (i == j) continue;
                        DrawLineEx(vertCpy[i], vertCpy[j], 1.f, color);
                    }
                }

                b2Vec2 center = phys->body->GetWorldCenter();
                DrawCircle(center.x * PIXELS_PER_UNIT, center.y * PIXELS_PER_UNIT, 4.f, color);
            }
            DrawLineStrip(&vertCpy[0], vertexCount, color);
        }
        break;
        default:
            // TODO: implement other shapes
            break;
        }
        next = next->GetNext();
    }
}

// smaller functions
bool EditorIsOpen() {
    return Session.isOpen;
}

void OpenEditor() {
    Session.isOpen = true;
}

void CloseEditor() {
    Session.isOpen = false;
}

bool ToggleEditor() {
    Session.isOpen = !Session.isOpen;
    return Session.isOpen;
}
