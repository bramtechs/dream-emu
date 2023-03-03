#include "magma.h"

typedef void (*UpdateAndDrawMode)(EntityGroup& group, Camera* camera, float delta);
typedef void (*UpdateAndDrawModeGUI)(EntityGroup& group, Camera* camera, float delta);

static void DoUpdateAndRenderEditor(EntityGroup& group, Camera* camera, float delta);

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
    std::pair<std::string, EntityBuilderFunction> builderBeingUsed = { "", NULL };

    std::unordered_map<EditorMode, EditorModeInfo> modes;
    std::unordered_map<std::string, EntityBuilderFunction> builders;
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

static EditorSession Session = EditorSession();
static bool EditorIs3D = false;

#include "editor_utils.cpp"

static void ProcNormalMode(EntityGroup& group, Camera* camera, float delta) {

    Vector2 mouse = GetWindowMousePosition(*(Camera2D*)camera);

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
                    SetEntityCenter(Session.subjectID, target);
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
                    DrawRetroText("hidden", sprite->bounds.min.x, sprite->bounds.min.y, 12, GRAY);
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
        buttons.AddButton("Delete",[&group](){
            group.DestroyEntity(Session.subjectID);
            Session.hasSubject = false;
        });
        buttons.AddButton("Change texture",[](){
            SwitchMode(MODE_TEXTURE);
        });
    }

    // editor modes
    for (const auto& mode : Session.modes) {
        if (mode.first != MODE_DELAY && mode.first != MODE_NORMAL) {
            buttons.AddButton(mode.second.description.c_str(), [&mode](){
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

    auto names = GetAssetNames(ASSET_TEXTURE);
    menu.RenderPanel();
    for (int i = 0; i < names.size(); i++) {
        menu.DrawPopButton(names[i].c_str());
    }
    menu.EndButtons();

    // process selection
    int index = 0;
    if (menu.IsButtonSelected(&index)) {
        // change texture of subject
        assert(Session.hasSubject);

        Sprite* sprite = NULL;
        group.GetEntityComponent(Session.subjectID, COMP_SPRITE, &sprite);

        Texture newTexture;
        if (HasDefaultPalette()) {
            newTexture = RequestIndexedTexture(names[index]);
        }
        else {
            newTexture = RequestTexture(names[index]);
        }

        sprite->SetTexture(newTexture);
        DEBUG("Switched entity texture to %s", names[index].c_str());

        // go back to default mode
        SwitchMode(MODE_NORMAL);
    }
}

static void ProcHitboxMode(EntityGroup& group, Camera* camera, float delta) {
    // start dragging mouse to place hitbox objects
    Vector2 mouse = GetWindowMousePosition(*(Camera2D*)camera);

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
        if (((AdvEntityGroup*)&group)->IsHitboxAtPos(checkPos, &touchedID)) {
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
    buttons.AddButton(Session.removalMode ? "Draw":"Delete", []() {
        Session.removalMode = !Session.removalMode;
    });
    buttons.AddButton("Simplify", [&group](){
        SimplifyHitboxes(group);
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
    Vector2 camPos = GetWindowMousePosition(cam2d);
    Vector2 snapPos = Vector2Snap(camPos, Session.gridSize);

    (*func)(group, { snapPos.x,snapPos.y,0 });
}

static void BuildTile(EntityGroup& group, Camera* camera, Texture texture, bool includeHitbox) {
    // spawn new entity
    Camera2D cam2d = *(Camera2D*)camera;

    // get snapped pos to place entity
    Vector2 camPos = GetWindowMousePosition(cam2d);
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
    Sprite sprite = Sprite({snapPos.x, snapPos.y});
    sprite.SetTexture(texture);
    group.AddEntityComponent(id, COMP_SPRITE, sprite, true);
}

static void ProcTileMode(EntityGroup& group, Camera* camera, float delta) {
    // get snapped pos to render ghost
    Camera2D cam2d = *(Camera2D*)camera;
    Vector2 camPos = GetWindowMousePosition(cam2d);
    Vector2 snapPos = Vector2Snap(camPos, Session.gridSize);

    bool isHolding = IsMouseButtonDown(0);

    if (Session.tileBeingDrawn.id > 0) { // if any tile texture selected
        Session.gridSize = Session.tileBeingDrawn.width;
        DrawTextureEx(Session.tileBeingDrawn, snapPos, 0.f, 1.f,
                                        ColorAlpha(WHITE, 0.6f));

        if (isHolding) {
            EntityID touchedID = 0;
            Vector2 checkPos = {
                snapPos.x + Session.tileBeingDrawn.width*0.5f,
                snapPos.y + Session.tileBeingDrawn.height*0.5f
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
        const char* text = Session.builderBeingUsed.first.c_str();
        Vector2 textPos = MeasureRetroText(text, 12);
        DrawRetroText(text, rect.x + textPos.x * 0.5f, rect.y + textPos.y * 0.5f, 12, BLUE);
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
        const char* name = builder.first.c_str();
        buttons.AddButton(name, [&builder](){
            Session.builderBeingUsed = builder;
            Session.tileBeingDrawn = {};
        });
    }

    // label all raw tiles
    buttons.AddSpacer("=== Tiles ===");
    static auto tiles = GetTileNames();
    for (const auto& tile : tiles) {
        buttons.AddButton(tile.c_str(), [&tile](){
            Session.builderBeingUsed = { "", NULL };
            Session.tileBeingDrawn = RequestTexture(tile);
        });
    }

    // extra utilities
    buttons.AddSpacer();
    buttons.AddButton(Session.removalMode ? "Draw" : "Delete", [](){
        Session.removalMode = !Session.removalMode;
    });
    buttons.AddButton(Session.includeHitbox ? "Including hitboxes" : "Not including hitboxes", [](){
        Session.includeHitbox = !Session.includeHitbox;
    });
    buttons.AddButton("Exit", [](){
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
        menu.DrawPopButton(builder.first.c_str());
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

static void DoUpdateAndRenderEditor(EntityGroup& group, Camera* camera, float delta) {
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
            DrawGrid(*(Camera2D*)camera);
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
            (*info.func)(group, camera, delta);
        }
    }
    catch (const std::out_of_range& e) {
    }
}

void UpdateAndRenderEditorGUI(EntityGroup& group, Camera* camera, float delta) {
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
        DrawRetroText(TextFormat("TIME SCALE %.3f\nDELTA %f", scale, GetFrameTime()), 20, 40, 18, RED);
    }

    Color bgColor = ColorAlpha(BLACK, 0.5f);

    int x = GetScreenWidth() - BAR_WIDTH;
    int y = 0;

    DrawRectangle(x, y, BAR_WIDTH, GetScreenHeight(), bgColor);

    x += 20;
    y += 20;

    // draw selected sprite properties
    const char* header = TextFormat("Entity count: %d\nNext entity: %d\nSelected Entity: %d\n=== Components ====", group.entityCount, group.nextEntity, Session.subjectID);
    DrawRetroText(header, x, y, FONT_SIZE, WHITE);
    y += MeasureRetroText(header, FONT_SIZE).y + 20;

    if (Session.hasSubject) {
        // collect all components of id
        for (const auto& cont : group.GetEntityComponents(Session.subjectID)) {
            // call descriptor to describe the component where dealing with, (fancy toString() function)
            auto desc = DescribeComponent(cont);
            const char* format = TextFormat("--> %s %s\n%s", desc.typeName.c_str(), cont.persistent ? "(PERSIST)" : "", desc.info.c_str());
            DrawRetroText(format, x, y, FONT_SIZE, desc.color);
            y += MeasureRetroText(format, FONT_SIZE).y + 10;
        }
    }

    // update and draw editor mode (gui elements)
    try {
        EditorModeInfo info = Session.modes.at(Session.mode);
        if (info.gui != NULL) {
            (*info.gui)(group, camera, delta);
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

void RegisterComponentDescriptor(ItemType type, ComponentDescriptor func) {
    Session.descriptors.insert({ type, func });
}

Description DescribeComponent(CompContainer cont) {
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

void EditorSession::LinkModes() {
    LinkMode(MODE_DELAY);
    LinkMode(MODE_NORMAL, ProcNormalMode, ProcNormalModeGUI);
    LinkMode(MODE_TEXTURE, NULL, ProcTextureModeGUI, "Change texture");
    LinkMode(MODE_SPAWN, NULL, ProcSpawnModeGUI, "Spawn entity");
    LinkMode(MODE_HITBOX, ProcHitboxMode, ProcHitboxModeGUI, "Draw hitboxes");
    LinkMode(MODE_TILE, ProcTileMode, ProcTileModeGUI, "Draw tiles");
}
