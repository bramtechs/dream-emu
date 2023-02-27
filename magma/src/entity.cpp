#include "magma.h"

void SetEntityCenter(EntityID id, Vector2 pos);
void SetEntityCenter(EntityID id, Vector3 pos);
void SetEntityCenter(EntityID id, float x, float y);
void SetEntityCenter(EntityID id, float x, float y, float z);

void SetEntitySize(EntityID id, Vector2 pos);
void SetEntitySize(EntityID id, Vector3 pos);
void SetEntitySize(EntityID id, float x, float y);
void SetEntitySize(EntityID id, float x, float y, float z);

void ResetEntityTranslation(EntityID id);

#if defined(MAGMA_3D)

Base::Base(Vector3 pos, Color tint) {
    BoundingBox box = {
        pos,
        Vector3Add(pos,Vector3One())
    };
    this->bounds = box;
    this->tint = tint;
}

void Base::Translate(Vector3 offset) {
    bounds.min = Vector3Add(bounds.min, offset);
    bounds.max = Vector3Add(bounds.max, offset);
}

void Base::SetCenter(Vector3 pos) {
    bounds.min = Vector3Subtract(pos, halfSize());
    bounds.max = Vector3Add(pos, halfSize());
}

void Base::SetSize(Vector3 size) {
    bounds.max = Vector3Add(bounds.min, size);
}

Vector3 Base::center() {
    return Vector3Add(bounds.min, halfSize());
}

Vector3 Base::size() {
    return Vector3Subtract(bounds.max, bounds.min);
}

Vector3 Base::halfSize() {
    return Vector3Scale(size(), 0.5f);
}

RayCollision Base::GetMouseRayCollision(Camera3D camera) {
    Ray ray = GetWindowMouseRay(camera);
    return GetRayCollisionBox(ray, bounds);
}

#endif

// Sprite
Sprite::Sprite(Vector2 pos, Color tint, int zOrder) {
    BoundingBox2D box = {
        pos,
        Vector2Add(pos,Vector2One())
    };
    this->texture = {};
    this->zOrder = zOrder;
    this->bounds = box;
    this->tint = tint;
    this->hFlip = false;
    this->vFlip = false;
    this->isVisible = true;
}

void Sprite::Translate(Vector2 offset) {
    bounds.min = Vector2Add(bounds.min, offset);
    bounds.max = Vector2Add(bounds.max, offset);
}

RayCollision Sprite::GetMouseRayCollision(Camera2D camera) {
    //Ray ray = GetWindowMouseRay(camera);
    //return GetRayCollisionBox(ray, bounds);
    assert(false); // TODO not implemented
    return {};
}

void Sprite::SetCenter(Vector2 pos) {
    Vector2 hSize = halfSize();
    bounds.min = Vector2Subtract(pos, hSize);
    bounds.max = Vector2Add(pos, hSize);
}

void Sprite::SetTopLeft(Vector2 pos) {
    Vector2 size = this->size();
    bounds.min = pos;
    bounds.max = Vector2Add(pos, size);
}

void Sprite::SetSize(Vector2 size) {
    bounds.max = Vector2Add(bounds.min, size);
}

void Sprite::SetTexture(Texture texture, Rectangle srcRect) {
    this->texture = texture;
    if (srcRect.x == 0 && srcRect.y == 0 &&
        srcRect.width == 0 && srcRect.height == 0) {
        this->srcRect = { 0.f, 0.f,
                         (float)floorf(texture.width), (float)floorf(texture.height) };
    }
    else {
        this->srcRect = srcRect;
    }

    SetSize({ this->srcRect.width,this->srcRect.height });
}

void Sprite::SetFlipped(bool hFlip, bool vFlip) {
    this->hFlip = hFlip;
    this->vFlip = vFlip;
}

void Sprite::SetFlippedX(bool hFlip) {
    this->hFlip = hFlip;
}

void Sprite::SetFlippedY(bool vFlip) {
    this->vFlip = vFlip;
}

void Sprite::SetVisible(bool isVisible) {
    this->isVisible = isVisible;
}

Rectangle Sprite::region() {
    Vector2 size = this->size();
    return { bounds.min.x , bounds.min.y, size.x, size.y };
}

Vector2 Sprite::center() {
    return Vector2Add(bounds.min, halfSize());
}

Vector2 Sprite::size() {
    return Vector2Subtract(bounds.max, bounds.min);
}

Vector2 Sprite::halfSize() {
    return Vector2Scale(size(), 0.5f);
}

#if defined(MAGMA_3D)

// ModelRenderer
ModelRenderer::ModelRenderer(const char* modelPath, Base* base) {
    Model model = RequestModel(modelPath);

    // make the base big enough to hold the model
    BoundingBox modelBox = GetModelBoundingBox(model);

    Vector3 size = Vector3Subtract(modelBox.max, modelBox.min);
    base->bounds.max = Vector3Add(base->bounds.min, size);

    Vector3 modelCenter = Vector3Add(modelBox.min, Vector3Scale(size, 0.5f));
    Vector3 offset = Vector3Subtract(base->center(), modelCenter);

    this->model = modelPath;
    this->accurate = false;
    this->offset = offset;
}

RayCollision EntityGroup::GetRayCollision(Ray ray) {
    float closestDistance = 10000000;
    RayCollision hit = { 0 };

    for (const auto& comp : comps) {
        if (comp.first == COMP_MODEL_RENDERER) {
            auto render = (ModelRenderer*)comp.second.data;
            auto base = (Base*)GetEntityComponent(comp.first, COMP_BASE);
            Model model = RequestModel(render->model);

            if (render->accurate) { // do per triangle collisions
                Vector3 offset = Vector3Add(base->center(), render->offset);
                for (int j = 0; j < model.meshCount; j++) {
                    RayCollision col = GetRayCollisionMesh(ray, model.meshes[j],
                        MatrixTranslate(offset.x, offset.y, offset.z));

                    if (col.hit && col.distance < closestDistance) {
                        closestDistance = col.distance;
                        hit = col;
                    }
                }
            }
            else { // do bounds collision
                RayCollision col = GetRayCollisionBox(ray, base->bounds);
                if (col.hit && col.distance < closestDistance) {
                    closestDistance = col.distance;
                    hit = col;
                }
            }
        }
    }
    return hit;
}

#endif

#if defined(MAGMA_3D)
bool EntityGroup::GetMousePickedBase(Camera camera, Base** result) {
    RayCollision col = { 0 };
    return GetMousePickedBaseEx(camera, result, &col);
}

bool EntityGroup::GetMousePickedBaseEx(Camera camera, Base** result, RayCollision* col) {
    for (const auto& entry : comps) {
        auto base = (Base*)entry.second.data;
        RayCollision rayCol = base->GetMouseRayCollision(camera);

        if (rayCol.hit) {
            *result = base;
            *col = rayCol;
            return true;
        }
    }
    *result = NULL;
    return false;
}
#endif

void EntityGroup::ClearGroup() {
    entityCount = 0;
    for (const auto& comp : comps) {
        M_MemFree(comp.second.data);
    }
    comps.clear();
}

#define CHECK(C) if (!(C)) return false;
bool EntityGroup::LoadGroup(const char* fileName) {

    auto valid = true;
    auto buffer = std::ifstream(fileName, std::ofstream::binary);

    // TODO: implement proper versioning
    uint32_t version = 0;
    buffer.read((char*)&version, sizeof(uint32_t));
    CHECK(version < 30);

    uint32_t compCount = 0;
    buffer.read((char*)&compCount, sizeof(uint32_t));
    CHECK(version < 30000);

    uint32_t entityCount = 0;
    buffer.read((char*)&entityCount, sizeof(uint32_t));

    auto mappedEntities = std::multimap<EntityID, EntityID>();
    for (int i = 0; i < compCount; i++) {
        EntityID entityID = 0; // uint32_t
        buffer.read((char*)&entityID, sizeof(uint32_t));

        ItemType compType = 0; // uint32_t
        buffer.read((char*)&compType, sizeof(uint32_t));

        uint64_t compSize = 0;
        buffer.read((char*)&compSize, sizeof(uint64_t));

        void* compData = M_MemAlloc(compSize);
        buffer.read((char*)compData, compSize);

        // map saved ID's to new EntityID's
        bool exists = false;
        EntityID destID = 0;
        for (const auto& entry : mappedEntities) {
            if (entry.first == entityID) {
                exists = true;
                destID = entry.second;
                break;
            }
        }
        if (!exists) {
            EntityID id = AddEntity();
            mappedEntities.insert({ entityID,id });
            destID = id;
            DEBUG("Importing entity %d as %d", entityID, id);
        }

        // FIX: hack to not crash physics bodies
        if (compType == COMP_PHYS_BODY) {
            ((PhysicsBody*)compData)->initialized = false;
        }

        // copy over component
        // FIX: loading advanced components crashes the game
        if (compType != COMP_PLAT_PLAYER && compType != COMP_ANIM_PLAYER) {
            AddEntityComponent(compType, destID, compData, compSize);
        }

        // memfree readed data
        M_MemFree(compData);
    }
    buffer.close();
    return true;
}

bool EntityGroup::SaveGroup(const char* fileName, uint32_t version) {
    // create parent directory if any
    const char* path = GetDirectoryPath(fileName);
    CreateDirectory(path);

    auto buffer = std::ofstream(fileName, std::ofstream::binary);

    // version
    buffer.write((char*)&version, sizeof(uint32_t));

    // component count
    auto size = (uint32_t)comps.size();
    buffer.write((char*)&size, sizeof(uint32_t));

    // entity count for checking
    auto ecount = (uint32_t)entityCount;
    buffer.write((char*)&ecount, sizeof(uint32_t));

    // for each component
    for (const auto& comp : comps) {
        // entity id
        auto id = (uint32_t)comp.first;
        buffer.write((char*)&id, sizeof(uint32_t));

        CompContainer cont = comp.second;

        // comptype
        auto type = (uint32_t)cont.type;
        buffer.write((char*)&type, sizeof(uint32_t));

        // compsize
        auto csize = (uint64_t)cont.size;
        buffer.write((char*)&csize, sizeof(uint64_t));

        // raw comp data
        buffer.write((char*)cont.data, csize);
    }

    buffer.close();
    INFO("Exported entity component to %s", fileName);

    // TODO: return false if failed
    return true;
}

static std::string ParsePath(const char* file, const char* folder = NULL) {
    if (file == NULL || TextIsEqual(file, "")) {
        return "";
    }

    std::string output = "";

    // set extension
    auto path = std::filesystem::path(file);
    path.replace_extension("comps");

    // add map prefix
    auto newName = "map_" + path.filename().string();
    path.replace_filename(newName);

    // add folder before if any
    if (folder != NULL && !TextIsEqual(folder, "")) {
        path = std::filesystem::path(folder) / path;
    }

    return path.string();
}

// TODO: put in struct
static EntityGroup* InteractiveEntityGroup = NULL;
static const char* InteractiveFolder = NULL;
static uint InteractiveVersion = 0;
static void SaveGroupInteractivelyCallback(std::string& text) {
    // replace spaces with underscores
    std::replace(text.begin(), text.end(), ' ', '_');

    // make name always lowercase
    text = TextToLower(text.c_str());

    std::string outputStr = ParsePath(text.c_str(), InteractiveFolder);
    if (outputStr.empty()) {
        WARN("EntityGroup interactive save cancelled");
    }
    else {
        InteractiveEntityGroup->SaveGroup(outputStr.c_str(), InteractiveVersion);
    }
}

void EntityGroup::SaveGroupInteractively(const char* folder, uint version) {
    InteractiveFolder = folder;
    InteractiveEntityGroup = this;
    InteractiveVersion = version;
    if (!ShowInputBox("Save entity group", &SaveGroupInteractivelyCallback, "mylevel", 4, 32)) {
        ERROR("Another input box is already opened!");
    }
}

void CollectSavedGroups(std::vector<std::string>* groups) {
    std::vector<std::string> searchPaths = {
        ".",
        "./raw_assets",
        "./temple/raw_assets",
        "../../temple/raw_assets",
        "../../raw_assets",
    };

    for (const auto& path : searchPaths) {
        FilePathList list = LoadDirectoryFilesEx(path.c_str(), ".comps", true);
        for (int i = 0; i < list.count; i++) {
            auto path = list.paths[i];
            groups->push_back(path);
            DEBUG("Found map at %s.", path);
        }
        UnloadDirectoryFiles(list);
    }
}

static std::vector<std::string> InteractiveGroups;
static PopMenu InteractivePopMenu = PopMenu(FOCUS_CRITICAL);
void EntityGroup::LoadGroupInteractively(uint version) {
    // TODO: put in entity_extra maybe
    InteractiveVersion = version;
    InteractiveGroups.clear();
    CollectSavedGroups(&InteractiveGroups);
}

void EntityGroup::UpdateAndRenderInteractiveGroupLoader() {
    if (InteractiveGroups.empty()) {
        return;
    }

    auto& menu = InteractivePopMenu;
    menu.RenderPanel();

    for (const auto& group : InteractiveGroups) {
        // TODO: check versions
        std::string smaller = group;
        while (smaller.size() > 32){
            smaller.pop_back();
        }
        const char* displayName = TextFormat("v%d - %s", 0, smaller.c_str());
        menu.DrawPopButton(displayName);
    }

    menu.DrawPopButton("-- Cancel");
    menu.EndButtons();

    int index = 0;
    if (menu.IsButtonSelected(&index)) {
        if (index >= 0 && index < InteractiveGroups.size()) {
            auto path = InteractiveGroups[index].c_str();
            INFO("Interactively loading level %s...", path);
            LoadGroup(path);
        }
        InteractiveGroups.clear();
    }
}

EntityID EntityGroup::AddEntity() {
    EntityID id = entityCount;
    entityCount++;
    return id;
}

bool EntityGroup::EntityExists(EntityID id) {
    for (const auto& comp : GetComponents()) {
        if (comp.first == id) {
            return true;
        }
    }
    return false;
}

void EntityGroup::AddEntityComponent(ItemType type, EntityID id, void* data, size_t size) {
    // make data stick with a malloc
    CompContainer cont;
    cont.type = type;
    cont.data = M_MemAlloc(size);
    cont.size = size;
    memcpy(cont.data, data, size);

    // add component in system
    comps.insert({ id, cont });
}

bool EntityGroup::EntityHasComponent(EntityID id, ItemType type) {
    for (const auto& comp : comps) {
        if (comp.first == id && comp.second.type == type) {
            return true;
        }
    }
    return false;
}

void* EntityGroup::TryGetEntityComponent(EntityID id, ItemType filter) {
    for (const auto& comp : comps) {
        if (comp.first == id && comp.second.type == filter) {
            return comp.second.data;
        }
    }
    return NULL;
}

std::vector<CompContainer> EntityGroup::GetEntityComponents(EntityID id, ItemType type) {
    std::vector<CompContainer> conts;
    for (const auto& comp : comps) {
        if (comp.first == id) {
            if (type == COMP_ALL || comp.second.type == type) {
                CompContainer container = comp.second;
                conts.push_back(container);
            }
        }
    }
    return conts;
}

std::multimap<EntityID, void*> EntityGroup::GetComponents(ItemType type) {
    std::multimap<EntityID, void*> results;
    for (const auto& comp : comps) {
        if (type == COMP_ALL || comp.second.type == type) {
            results.insert({ comp.first, comp.second.data });
        }
    }
    return results;
}

static void UpdateComponent(EntityGroup& group, IteratedComp& comp, float delta) {
    //    switch (comp.second.type) {
    //    case COMP_BASE:
    //    {
    //        auto base = (Base*)comp.second.data;
    //    } break;
    //    default:
    //        break;
    //    }
}

#if defined(MAGMA_3D)
static void Draw3DComponent(EntityGroup& group, IteratedComp& comp) {
    switch (comp.second.type) {
    case COMP_MODEL_RENDERER:
    {
        // draw modelrenderers
        auto renderer = (ModelRenderer*)comp.second.data;
        auto base = (Base*)GetEntityComponent(comp.first, COMP_BASE);

        if (base == NULL) {
            assert(false); // model renderer has no base! TODO shouldn't crash
        }

        Model model = RequestModel(renderer->model);
        DrawModelEx(model, Vector3Add(base->center(), renderer->offset),
            Vector3Zero(), 0, Vector3One(), base->tint);

    } break;
    }
}
static void Draw3DComponentDebug(EntityGroup& group, IteratedComp& comp) {
    switch (comp.first) {
    case COMP_BASE:
    {
        auto base = (Base*)comp.second.data;
        RayCollision col = base->GetMouseRayCollision(camera);
        Color tint = col.hit ? WHITE : GRAY;
        DrawBoundingBox(base->bounds, tint);
        DrawPoint3D(base->center(), col.hit ? WHITE : GRAY);
    } break;
    }
}
#endif

static void DrawComponent(EntityGroup& group, IteratedComp& comp) {
    switch (comp.second.type) {
    case COMP_SPRITE:
    {
        auto sprite = (Sprite*)comp.second.data;
        if (sprite->isVisible &&
            sprite->texture.width > 0 &&
            sprite->texture.height > 0)
        {
            Rectangle dest = BoundingBoxToRect(sprite->bounds);
            Rectangle src = {
                sprite->srcRect.x,
                sprite->srcRect.y,
                sprite->hFlip ? -sprite->srcRect.width : sprite->srcRect.width,
                sprite->vFlip ? -sprite->srcRect.height : sprite->srcRect.height
            };
            DrawTexturePro(sprite->texture, src, dest, Vector2Zero(), 0.f, sprite->tint);
        }
    } break;
    }
}

static void DrawComponentDebug(IteratedComp& comp) {
    switch (comp.first) {
    case COMP_SPRITE:
    {
        auto sprite = (Sprite*)comp.second.data;
        // RayCollision col = base->GetMouseRayCollision(camera);
        // Color tint = col.hit ? WHITE : GRAY;
        Color tint = WHITE;
        DrawBoundingBox(sprite->bounds, tint);
        DrawCircleV(sprite->center(), 2.f, RED);
    } break;
    default:
        break;
    }
}

void EntityGroup::UpdateGroup(float delta) {
    float scaledDelta = delta * GetTimeScale();
    for (auto& comp : comps) {
        for (const auto& updater : updaters) {
            // run update 'hook'
            EntityGroup& group = *this;
            (*updater)(group, comp, scaledDelta);
        }
    }
}

void EntityGroup::DrawGroup() {
    for (auto& comp : comps) {
        for (const auto& drawer : drawers) {
            // run draw 'hook' if not debug
            EntityGroup& group = *this;
            bool isDebug = drawer.second;
            if (!isDebug) {
                (*drawer.first)(group, comp);
            }
        }
    }
}

void EntityGroup::DrawGroupDebug() {
    for (auto& comp : comps) {
        for (const auto& drawer : drawers) {
            // run draw 'hook' if debug
            EntityGroup& group = *this;
            bool isDebug = drawer.second;
            if (isDebug) {
                (*drawer.first)(group, comp);
            }
        }
    }
    UpdateAndRenderInteractiveGroupLoader();
}

EntityGroup::EntityGroup() {
    this->entityCount = 0;

    // add stock updators, drawers
    RegisterUpdater(UpdateComponent);
    RegisterDrawer(DrawComponent);
#ifdef MAGMA_3D
    RegisterDrawer(Update3DComponent);
#endif

    // add extended updaters, drawers
    RegisterUpdater(UpdateExtendedComponent);
}

void EntityGroup::RegisterUpdater(UpdateComponentFunc updateFunc) {
    updaters.push_back(updateFunc);
}

void EntityGroup::RegisterDrawer(DrawComponentFunc drawFunc, bool isDebug) {
    drawers.insert({ drawFunc,isDebug });
}
