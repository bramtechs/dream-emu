#include "magma.h"

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
void Base::Translate(float x, float y, float z) {
    Translate({ x,y,z });
}
void Base::TranslateX(float x) {
    Translate({ x,0,0 });
}
void Base::TranslateY(float y) {
    Translate({ 0,y,0 });
}
void Base::TranslateZ(float z) {
    Translate({ 0,0,z });
}

void Base::SetCenter(Vector3 pos) {
    bounds.min = Vector3Subtract(pos, halfSize());
    bounds.max = Vector3Add(pos, halfSize());
}
void Base::SetCenter(float x, float y, float z){
    SetSize({x,y,z});
}

void Base::SetSize(Vector3 size){
    bounds.max = Vector3Add(bounds.min,size);
}
void Base::SetSize(float x, float y, float z){
    SetSize({x,y,z});
}

void Base::ResetTranslation() {
    SetCenter(Vector3Zero());
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
    this->isBeingMoved = false;
}

void Sprite::Translate(Vector2 offset) {
    bounds.min = Vector2Add(bounds.min, offset);
    bounds.max = Vector2Add(bounds.max, offset);
    // for PhysicsBody (if any)
    isBeingMoved = true;
}
void Sprite::Translate(float x, float y) {
    Translate({ x,y });
}
void Sprite::TranslateX(float x) {
    Translate({ x,0 });
}
void Sprite::TranslateY(float y) {
    Translate({ 0,y });
}
void Sprite::ResetTranslation() {
    SetCenter(Vector2Zero());
}

RayCollision Sprite::GetMouseRayCollision(Camera2D camera) {
    //Ray ray = GetWindowMouseRay(camera);
    //return GetRayCollisionBox(ray, bounds);
    assert(false); // TODO not implemented
    return {};
}

void Sprite::SetCenter(Vector2 pos, bool isRaw) {
    Vector2 hSize = halfSize();
    bounds.min = Vector2Subtract(pos, hSize);
    bounds.max = Vector2Add(pos, hSize);
    if (!isRaw) {
        isBeingMoved = true;
    }
}

void Sprite::SetCenter(float x, float y){
    SetCenter({x,y});
}
void Sprite::SetTopLeft(Vector2 pos) {
    Vector2 size = this->size();
    bounds.min = pos;
    bounds.max = Vector2Add(pos, size);
    isBeingMoved = true;
}
void Sprite::SetTopLeft(float x, float y){
    SetTopLeft({x,y});
}

void Sprite::SetSize(Vector2 size, bool isRaw){
    bounds.max = Vector2Add(bounds.min,size);
    if (!isRaw) {
        isBeingMoved = true;
    }
}
void Sprite::SetSize(float x, float y){
    SetSize({x,y});
}

void Sprite::SetTexture(Texture texture, Rectangle srcRect, bool isRaw){
    this->texture = texture;
    if (srcRect.x == 0 && srcRect.y == 0 &&
        srcRect.width == 0 && srcRect.height == 0){
        this->srcRect = { 0.f, 0.f,
                         (float) floorf(texture.width), (float) floorf(texture.height) };
    }
    else{
        this->srcRect = srcRect;
    }

    SetSize({this->srcRect.width,this->srcRect.height},isRaw);
}

Rectangle Sprite::region(){
    Vector2 size = this->size();
    return {bounds.min.x , bounds.min.y, size.x, size.y};
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

EntityGroup::EntityGroup(float gravity) {
    this->entityCount = 0;

    // make box2d world
    b2Vec2 gravVec2(0.f,gravity);
    this->world = new b2World(gravVec2);
    DEBUG("Allocated Box2D world");
}

EntityGroup::~EntityGroup() {
    delete this->world;
    DEBUG("Disposed Box2D world");
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

void EntityGroup::LoadGroup(const char* fileName) {
    comps.clear();
    INFO("Loaded entitygroup from %s", fileName);
}

void EntityGroup::SaveGroup(const char* fileName) {
    // TODO
    INFO("TODO");
    INFO("Exported entity component to %s", fileName);
}

EntityID EntityGroup::AddEntity() {
    EntityID id = entityCount;
    entityCount++;
    return id;
}

void* EntityGroup::GetEntityComponent(EntityID id, ItemType filter) {
    for (const auto& comp : comps) {
        if (comp.first == id && comp.second.type == filter) {
            return comp.second.data;
        }
    }
    return NULL;
}

std::vector<CompContainer> EntityGroup::GetEntityComponents(EntityID id, ItemType type) {
    std::vector<CompContainer> conts;
    for (const auto &comp : comps){
        if (comp.first == id){
            if (type == COMP_ALL || comp.second.type == type){
                CompContainer container = comp.second;
                conts.push_back(container);
            }
        }
    }
    return conts;
}

std::multimap<EntityID,void*> EntityGroup::GetComponents(ItemType type) {
    std::multimap<EntityID, void*> results;
    for (const auto &comp : comps) {
        if (type == COMP_ALL || comp.second.type == type){
            results.insert({comp.first, comp.second.data});
        }
    }
    return results;
}

size_t EntityGroup::UpdateGroup(float delta) {
    float scaledDelta = delta * GetTimeScale();

    //for (const auto& comp : comps) {
    //    switch (comp.second.type) {
    //    case COMP_BASE:
    //    {
    //        auto base = (Base*)comp.second.data;
    //    } break;
    //    default:
    //        break;
    //    }
    //}
    
    entityCount += UpdateGroupExtended(this,scaledDelta);
    return entityCount;
}

size_t EntityGroup::DrawGroup() {
    for (const auto& comp : comps) {
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
        case COMP_SPRITE:
        {
            auto sprite = (Sprite*) comp.second.data;

            Color tint = WHITE;
            if (sprite->texture.width > 0) {
                Rectangle dest = BoundingBoxToRect(sprite->bounds);
                DrawTexturePro(sprite->texture, sprite->srcRect,
                               dest, Vector2Zero(), 0.f, sprite->tint);
            }
        } break;
        default:
            break;
        }
    }
    entityCount += DrawGroupExtended(this);
    return entityCount;
}

size_t EntityGroup::DrawGroupDebug(Camera3D camera) {
    for (const auto& comp : comps) {
        switch (comp.first) {
        case COMP_BASE:
        {
            auto base = (Base*)comp.second.data;
            RayCollision col = base->GetMouseRayCollision(camera);
            Color tint = col.hit ? WHITE : GRAY;
            DrawBoundingBox(base->bounds, tint);
            DrawPoint3D(base->center(), col.hit ? WHITE : GRAY);
        } break;
        default:
            break;
        }
    }
    return entityCount;
}

size_t EntityGroup::DrawGroupDebug(Camera2D camera) {
    for (const auto& comp : comps) {
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
    return entityCount;
}
