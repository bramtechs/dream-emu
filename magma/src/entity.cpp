#include "magma.h"
#include "magma_entity.hpp"

#include <fstream>
#include <filesystem>
#include <magma_gui.hpp>

EntityGroup Group = {};

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
            ModelRenderer* render = (ModelRenderer*)comp.second.data;

            Base* base = NULL;
            GetEntityComponent(comp.first, COMP_BASE, &base);

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

void EntityGroup::ClearGroup() {
    entityCount = 0;
    nextEntity = 0;
    for (const auto& comp : comps) {
        free(comp.second.data);
    }
    comps.clear();

    delete this->world;

    // make box2d world
    b2Vec2 gravVec2(0.f, this->gravity);
    this->world = new b2World(gravVec2);
    DEBUG("Allocated Box2D world");
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

        void* compData = malloc(compSize);
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
        AddEntityComponent(destID, compType, compData, compSize, true);

        // memfree readed data
        free(compData);
    }
    buffer.close();
    return true;
}

bool EntityGroup::SaveGroup(const char* fileName, uint32_t version) {
    // collect saveable/persisting components
    auto persComps = GetComponents(COMP_PERSISTENT);
    if (persComps.empty()) {
        ERROR("There is nothing worth saving!");
        return false;
    }

    // create parent directory if any
    const char* path = GetDirectoryPath(fileName);
    CreateDirectory(path);

    auto buffer = std::ofstream(fileName, std::ofstream::binary);

    // version
    buffer.write((char*)&version, sizeof(uint32_t));

    // component count
    auto size = (uint32_t)persComps.size();
    buffer.write((char*)&size, sizeof(uint32_t));

    // entity count for checking
    auto ecount = (uint32_t)entityCount;
    buffer.write((char*)&ecount, sizeof(uint32_t));

    // for each component
    for (const auto& comp : persComps) {
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

// TODO: put in struct
EntityID EntityGroup::AddEntity() {
    EntityID id = nextEntity;
    entityCount++;
    nextEntity++;
    return id;
}

void EntityGroup::DestroyEntity(EntityID id) {
    // manually free memory
    size_t count = 0;
    auto items = comps.equal_range(id); // get all results
    for (auto it = items.first; it != items.second; ++it) {
        free(it->second.data);
        count++;
    }

    if (count > 0) {
        // remove all components with id
        comps.erase(id);

        entityCount--;
        DEBUG("Removed entity %d with %d components.", id, count);
    }
    else {
        WARN("Entity %d can't be removed as it doesn't exist!", id);
    }
}

bool EntityGroup::EntityExists(EntityID id) {
    for (const auto& comp : GetComponents()) {
        if (comp.first == id) {
            return true;
        }
    }
    return false;
}

bool EntityGroup::IsEntityAtPos(Vector2 centerPos, EntityID* found) {
    std::multimap<EntityID, CompContainer> sprites = GetComponents(COMP_SPRITE);
    for (auto& comp : sprites) {
        auto sprite = (Sprite*)comp.second.data;
        Vector2 sprCenter = sprite->center();
        if (FloatEquals(sprCenter.x, centerPos.x) && FloatEquals(sprCenter.y, centerPos.y)) {
            if (found) {
                *found = comp.first;
            }
            return true;
        }
    }
    if (found) {
        *found = -1;
    }
    return false;
}

void* EntityGroup::AddEntityComponent(EntityID id, ItemType type, void* data, size_t size, bool persistent) {
    // make data stick with a malloc
    CompContainer cont;
    cont.type = type;
    cont.data = malloc(size);
    cont.size = size;
    cont.persistent = persistent;
    memcpy(cont.data, data, size);

    // add component in system
    comps.insert({ id, cont });

    return cont.data;
}

bool EntityGroup::EntityHasComponent(EntityID id, ItemType type) {
    for (const auto& comp : comps) {
        if (comp.first == id && comp.second.type == type) {
            return true;
        }
    }
    return false;
}

std::vector<CompContainer> EntityGroup::GetEntityComponents(EntityID id, ItemType type) {
    std::vector<CompContainer> conts;

    auto items = comps.equal_range(id); // get all results
    for (auto it = items.first; it != items.second; ++it) {
        if (type == COMP_ALL || it->second.type == type) {
            CompContainer container = it->second;
            conts.push_back(container);
        }
    }
    return conts;
}

std::multimap<EntityID, CompContainer> EntityGroup::GetComponents(ItemType type) {
    std::multimap<EntityID, CompContainer> results;
    if (type == COMP_ALL) { // pass everything
        results = comps;
    }
    else if (type == COMP_PERSISTENT) { // pass saveable
        for (const auto& comp : comps) {
            if (comp.second.persistent) {
                results.insert({ comp.first, comp.second });
            }
        }
    }
    else { // all other types
        for (const auto& comp : comps) {
            if (comp.second.type == type) {
                results.insert({ comp.first, comp.second });
            }
        }
    }
    return results;
}

static void UpdateComponent(EntityGroup& group, IteratedComp& comp, float delta) {

    switch (comp.second.type) {
    case COMP_ANIM_PLAYER:
    {
        auto animPlayer = (AnimationPlayer*)comp.second.data;

        Sprite* sprite = NULL;
        group.GetEntityComponent(comp.first, COMP_SPRITE, &sprite);

        // advance current animation
        if (!animPlayer->curAnim) {
            break;
        }
        const SheetAnimation& anim = *animPlayer->curAnim;

        // check if not playing outside of sheet
        // TODO: FIX CRITICAL MEMORY BUG
        //assert(abs(animPlayer->curFrame) < anim.count);

        // set current frame
        Texture sheetTexture = RequestTexture(anim.sheetName);
        int absFrame = abs(animPlayer->curFrame);
        if (animPlayer->curFrame > anim.count) absFrame = 0; // TODO: hack
        Rectangle src = {
            anim.origin.x + anim.cellSize.x * absFrame,
            anim.origin.y,
            anim.cellSize.x,
            anim.cellSize.y,
        };

        sprite->SetTexture(sheetTexture, src);

        if (animPlayer->timer > 1.f / anim.fps) {
            animPlayer->timer = 0.f;
            animPlayer->curFrame++;

            switch (anim.mode) {
            case PLAY_LOOP:
                animPlayer->curFrame = Wrap(animPlayer->curFrame, 0, anim.count - 1);
                break;
            case PLAY_ONCE:
                animPlayer->curFrame = Clamp(animPlayer->curFrame, 0, anim.count - 1);
                break;
            case PLAY_PING_PONG:
                if (animPlayer->curFrame >= (int)anim.count) {
                    animPlayer->curFrame = -anim.count + 1;
                }
                break;
            }
        }
        animPlayer->timer += delta;
    } break;
    case COMP_PHYS_BODY:
    {
        PhysicsBody* phys = (PhysicsBody*)comp.second.data;
        Sprite* sprite = NULL;
        group.GetEntityComponent(comp.first, COMP_SPRITE, &sprite);

        // init if newly created
        if (!phys->initialized) {
            b2BodyDef bodyDef;
            bodyDef.position.Set(sprite->bounds.min.x / PIXELS_PER_UNIT,
                sprite->bounds.min.y / PIXELS_PER_UNIT);

            if (phys->dynamic) {
                bodyDef.type = b2_dynamicBody;
            }

            phys->body = group.world->CreateBody(&bodyDef);
            phys->body->SetFixedRotation(true);
            phys->body->SetSleepingAllowed(false);

            b2PolygonShape boxShape;
            boxShape.SetAsBox(sprite->halfSize().x / PIXELS_PER_UNIT,
                sprite->halfSize().y / PIXELS_PER_UNIT);

            if (phys->dynamic) {
                b2FixtureDef fixtureDef;
                fixtureDef.shape = &boxShape;
                fixtureDef.density = 30.0f;
                fixtureDef.friction = .8f;
                phys->body->CreateFixture(&fixtureDef);
            }
            else {
                phys->body->CreateFixture(&boxShape, 0.f);
            }

            phys->initialized = true;
        }

        // place sprite at PhysicsBody location
        Vector2 newPos = Vector2Scale(phys->position(), PIXELS_PER_UNIT);
        sprite->SetCenter(newPos);

    } break;
    case COMP_PLAT_PLAYER:
    {
        auto plat = (PlatformerPlayer*)comp.second.data;

        PhysicsBody* phys = NULL;
        group.GetEntityComponent(comp.first, COMP_PHYS_BODY, &phys);
        if (!phys->initialized) {
            break;
        }

        bool isMoving = false;
        b2Vec2 force = { 0.f, -0.01f };
        if (IsKeyDown(KEY_A)) {
            force.x = -plat->moveSpeed;
            plat->pose = POSE_WALK;
            plat->isLookingRight = false;
            isMoving = true;
        }
        if (IsKeyDown(KEY_D)) {
            force.x = plat->moveSpeed;
            plat->pose = POSE_WALK;
            plat->isLookingRight = true;
            isMoving = true;
        }
        if (IsKeyPressed(KEY_SPACE)) { // jump
            b2Vec2 jumpForce = { 0.f, -plat->jumpForce };
            phys->body->ApplyLinearImpulseToCenter(jumpForce, true);
            plat->pose = POSE_JUMP;
        }
        phys->body->ApplyForceToCenter(force, true);

        PlayerPose* pose = &plat->pose;
        auto curVel = phys->body->GetLinearVelocity();

        if (curVel.y > EPSILON) { // dermine if falling
            plat->pose = POSE_FALL;
        }
        else if (curVel.y < -EPSILON) { // determine if jumping
            plat->pose = POSE_JUMP;
        }
        else if (!isMoving && abs(curVel.x) > 0.f) { // determine if sliding
            plat->pose = POSE_SLIDE;
        }
        else if (!isMoving) { // determine if idling
            plat->pose = POSE_IDLE;
        }

        // ==== Animation ===
        Sprite* sprite = NULL;
        group.GetEntityComponent(comp.first, COMP_SPRITE, &sprite);

        AnimationPlayer* anim = NULL;
        group.GetEntityComponent(comp.first, COMP_ANIM_PLAYER, &anim);

        // look left or right
        sprite->SetFlippedX(!plat->isLookingRight);

        // set animation according to pose
        const SheetAnimation& sheet = *plat->animations[plat->pose];
        if (sheet.name){
            anim->SetAnimation(sheet);
        }else{
            ERROR("No animation registered for pose %s",GetPlayerPoseNames()[(int)plat->pose]);
        }

    } break;
    }
}

static void Draw3DComponent(EntityGroup& group, IteratedComp& comp) {
    switch (comp.second.type) {
    case COMP_MODEL_RENDERER:
    {
        // draw modelrenderers
        auto renderer = (ModelRenderer*)comp.second.data;

        Base* base = NULL;
        group.GetEntityComponent(comp.first, COMP_BASE, &base);

        Model model = RequestModel(renderer->model);
        DrawModelEx(model, Vector3Add(base->center(), renderer->offset),
            Vector3Zero(), 0, Vector3One(), base->tint);

    } break;
    }
}

static void Draw3DComponentDebug(EntityGroup& group, IteratedComp& comp, void* data) {
    switch (comp.first) {
    case COMP_BASE:
    {
        auto base = (Base*)comp.second.data;
        RayCollision col = base->GetMouseRayCollision(*(Camera*)data);
        Color tint = col.hit ? WHITE : GRAY;
        DrawBoundingBox(base->bounds, tint);
        DrawPoint3D(base->center(), col.hit ? WHITE : GRAY);
    } break;
    }
}

static void DrawComponent(EntityGroup& group, IteratedComp& comp) {
    switch (comp.second.type) {
    case COMP_SPRITE:
    {
        auto sprite = (Sprite*)comp.second.data;
        if (sprite->isVisible &&
            sprite->texture.width > 0 &&
            sprite->texture.height > 0)
        {
            Rectangle dest = BoundingBox2DToRect(sprite->bounds);
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
        DrawBoundingBox2D(sprite->bounds, tint);
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

    if (HasPhysics()) {
        // TODO: make optional
        // run physics engine
        float timeStep = delta;
        int velocityIterations = 6;
        int positionIterations = 2;

        assert(world);
        world->Step(timeStep, velocityIterations, positionIterations);
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

static void UpdateAndRenderInteractiveGroupLoader(EntityGroup& group);
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
    UpdateAndRenderInteractiveGroupLoader(*this);
}

bool EntityGroup::HasPhysics() {
    return this->world;
}

bool EntityGroup::IsHitboxAtPos(Vector2 centerPos, EntityID* id) {
    // convert pixel- to physics coordinates
    centerPos = Vector2Scale(centerPos, 1.f / PIXELS_PER_UNIT);

    std::multimap<EntityID, CompContainer> physBodies = GetComponents(COMP_PHYS_BODY);
    for (auto& phys : physBodies) {
        auto physBody = (PhysicsBody*)phys.second.data;
        b2Vec2 ePos = physBody->body->GetWorldCenter();
        if (FloatEquals(ePos.x, centerPos.x) && FloatEquals(ePos.y, centerPos.y)) {
            if (id)
                *id = phys.first;
            return true;
        }
    }
    if (id)
        *id = 0;
    return false;
}

EntityGroup::EntityGroup() {
    this->entityCount = 0;
    this->nextEntity = 0;

    // add stock updators, drawers
    RegisterUpdater(UpdateComponent);
    RegisterDrawer(DrawComponent);
    // RegisterDrawer(Update3DComponent);

    // add extended updaters, drawers
    this->world = NULL;
    this->gravity = 10.f;

    ClearGroup();

    // setup 2d camera for now
    this->camera2D.offset = Vector2Zero();              // Camera offset (displacement from target)
    this->camera2D.target = Vector2Zero();              // Camera target (rotation and zoom origin)
    this->camera2D.rotation = 0.f;                      // Camera rotation in degrees
    this->camera2D.zoom = 1.f;                          // Camera zoom (scaling), should be 1.0f by default
}

EntityGroup::~EntityGroup() {
    if (HasPhysics()) {
        delete this->world;
        DEBUG("Disposed Box2D world");
    }
}

// entity group functions
void SetEntityGroupCamera2D(Camera2D camera) {
    Group.camera2D = camera;
}

void SetEntityGroupCamera3D(Camera3D camera) {
    Group.camera3D = camera;
}

static std::vector<std::string> InteractiveGroups;
static PopMenu InteractivePopMenu = PopMenu(FOCUS_CRITICAL);
static void UpdateAndRenderInteractiveGroupLoader(EntityGroup& group) {
    if (InteractiveGroups.empty()) {
        return;
    }

    auto& menu = InteractivePopMenu;
    menu.RenderPanel();

    for (const auto& group : InteractiveGroups) {
        // TODO: check versions
        std::string smaller = group;
        while (smaller.size() > 32) {
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
            group.LoadGroup(path);
        }
        InteractiveGroups.clear();
    }
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

// =========================
// TODO: migrate to editor.cpp!!
// =========================
static const char* InteractiveFolder = NULL;
static uint InteractiveVersion = 0;

static void SaveGroupInteractivelyCallback(char* ctext) {
    std::string text = ctext;

    // replace spaces with underscores
    std::replace(text.begin(), text.end(), ' ', '_');

    // make name always lowercase
    text = TextToLower(text.c_str());

    std::string outputStr = ParsePath(text.c_str(), InteractiveFolder);
    if (outputStr.empty()) {
        WARN("EntityGroup interactive save cancelled");
    }
    else {
        Group.SaveGroup(outputStr.c_str(), InteractiveVersion);
    }
}

static void CollectSavedGroups(std::vector<std::string>* groups) {
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

void EntityGroup::SaveGroupInteractively(const char* folder, uint version) {
    InteractiveFolder = folder;
    InteractiveVersion = version;
    if (!ShowInputBoxEx("Save entity group", SaveGroupInteractivelyCallback, "mylevel", 4, 32)) {
        ERROR("Another input box is already opened!");
    }
}

void EntityGroup::LoadGroupInteractively(uint version) {
    // TODO: put in entity_extra maybe
    InteractiveVersion = version;
    InteractiveGroups.clear();
    CollectSavedGroups(&InteractiveGroups);
}

void EntityGroup::RegisterUpdater(UpdateComponentFunc updateFunc) {
    updaters.push_back(updateFunc);
}

void EntityGroup::RegisterDrawer(DrawComponentFunc drawFunc, bool isDebug) {
    drawers.insert({ drawFunc,isDebug });
}
