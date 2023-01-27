#include "magma.h"

Base::Base(EntityID id) {
    Base::Base(id, Vector3Zero(), WHITE);
}

Base::Base(EntityID id, Vector3 pos, Color tint) {
    BoundingBox box = {
        pos,
        Vector3Add(pos,Vector3One())
    };

    this->id = id;
    this->bounds = box;
    this->tint = tint;
}

void Base::Translate(Vector3 offset) {
    bounds.min = Vector3Add(bounds.min, offset);
    bounds.max = Vector3Add(bounds.max, offset);
}
inline void Base::TranslateX(float x) {
    Translate({ x,0,0 });
}
inline void Base::TranslateY(float y) {
    Translate({ 0,y,0 });
}
inline void Base::TranslateZ(float z) {
    Translate({ 0,0,z });
}
inline void Base::TranslateXYZ(float x, float y, float z) {
    Translate({ x,y,z });
}

void Base::SetCenter(Vector3 pos) {
    bounds.min = Vector3Subtract(pos, halfSize());
    bounds.max = Vector3Add(pos, halfSize());
}
inline void Base::ResetTranslation() {
    SetCenter(Vector3Zero());
}

inline Vector3 Base::center() {
    return Vector3Add(bounds.min, halfSize());
}

inline Vector3 Base::size() {
    return Vector3Subtract(bounds.max, bounds.min);
}

inline Vector3 Base::halfSize() {
    return Vector3Scale(size(), 0.5f);
}

RayCollision Base::GetMouseRayCollision(Camera camera) {
    Ray ray = GetWindowMouseRay(camera);
    return GetRayCollisionBox(ray, bounds);
}

ModelRenderer::ModelRenderer(EntityID id, const char* modelPath, Base* base) {
    Model model = RequestModel(modelPath);

    // make the base big enough to hold the model
    BoundingBox modelBox = GetModelBoundingBox(model);

    Vector3 size = Vector3Subtract(modelBox.max, modelBox.min);
    base->bounds.max = Vector3Add(base->bounds.min, size);

    Vector3 modelCenter = Vector3Add(modelBox.min, Vector3Scale(size, 0.5f));
    Vector3 offset = Vector3Subtract(base->center(), modelCenter);

    this->id = id;
    this->model = modelPath;
    this->accurate = false;
    this->offset = offset;
}

RayCollision EntityGroup::GetRayCollision(Ray ray) {
    float closestDistance = 10000000;
    RayCollision hit = { 0 };

    for (const auto& comp : comps) {
        if (comp.first == COMP_MODEL_RENDERER) {
            auto render = (ModelRenderer*)comp.second;
            auto base = (Base*)GetEntityComponent(render->id, COMP_BASE);
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
        auto base = (Base*)entry.second;
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
        // TODO dirty hack 
        EntityID otherId = *((EntityID*)comp.second);
        if (otherId == id) {
            return comp.second;
        }
    }
    return NULL;
}


size_t EntityGroup::UpdateGroup(float delta) {
    for (const auto& comp : comps) {
        switch (comp.first) {
        case COMP_BASE:
        {
            auto base = (Base*)comp.second;
        } break;
        default:
            break;
        }
    }
    return entityCount;
}

size_t EntityGroup::DrawGroup(Camera camera, bool drawOutlines) {
    for (const auto& comp : comps) {
        switch (comp.first) {
        case COMP_MODEL_RENDERER:
        {
            // draw modelrenderers
            auto renderer = (ModelRenderer*)comp.second;
            auto base = (Base*)GetEntityComponent(renderer->id, COMP_BASE);

            if (base == NULL) {
                assert(false); // model renderer has no base! TODO shouldn't crash
            }

            Model model = RequestModel(renderer->model);
            DrawModelEx(model, Vector3Add(base->center(), renderer->offset),
                Vector3Zero(), 0, Vector3One(), base->tint);

        } break;
        case COMP_BASE:
        {
            if (drawOutlines) {
                auto base = (Base*)comp.second;
                RayCollision col = base->GetMouseRayCollision(camera);
                Color tint = col.hit ? WHITE : GRAY;
                DrawBoundingBox(base->bounds, tint);
                DrawPoint3D(base->center(), col.hit ? WHITE : GRAY);
            }
        } break;
        default:
            break;
        }
    }
    return entityCount;
}

PlayerFPS::PlayerFPS(float eyeHeight) {
    this->eyeHeight = eyeHeight;
    this->isFocused = true;

    Camera* cam = &this->camera;
    cam->projection = CAMERA_PERSPECTIVE;
    cam->up = { 0.0f, 1.0f, 0.f };

    SetCameraMode(*cam, CAMERA_CUSTOM);

    SetAngle(0.f);
    SetFov(80.f);

    Focus();
}

void PlayerFPS::SetAngle(float lookAtDeg) {
    Vector3 pos = camera.position;
    Vector3 offset = {
        cosf(lookAtDeg * DEG2RAD),
        sinf(lookAtDeg * DEG2RAD),
        0
    };
    camera.target = Vector3Add(pos, offset);
}

void PlayerFPS::SetFov(float fovDeg) {
    camera.fovy = fovDeg;
}

// TODO shouldnt be a EntityGroup pointer but that doesnt compile for some reason
Vector3 PlayerFPS::Update(void* group, float delta) {

    // snap to the floor 
    Ray ray = { 0 };

    Vector3 offset = { 0,this->eyeHeight,0 };
    ray.position = Vector3Add(this->camera.position, offset);
    ray.direction = { 0,-1,0 };

    RayCollision col = ((EntityGroup*)group)->GetRayCollision(ray);

    // move player to hit point
    feet = Vector3Add(col.point, offset);
    camera.position = this->feet;

    if (this->isFocused) {

        // TODO naive use proper quaternions instead!

        Vector2 mouse = Vector2Scale(Vector2Scale(GetMouseDelta(), delta), 3.f);
        tilt -= mouse.y;
        angle -= mouse.x;
        SetMousePosition(GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f);

        // look around
        Vector3 angle = {
            sinf(this->angle * DEG2RAD),
            sinf(this->tilt * DEG2RAD) * 2,
            cosf(this->angle * DEG2RAD)
        };

        // clamp
        tilt = Clamp(tilt, -90, 90);

        // movement
        float axisX = 0;
        bool pressed = false;
        if (IsKeyDown(KEY_W)) {
            axisX = 90;
            pressed = true;
        }
        if (IsKeyDown(KEY_S)) {
            axisX = 270;
            pressed = true;
        }
        if (pressed) {
            Vector3 offset = { cosf((this->angle - axisX) * DEG2RAD), 0, sinf((this->angle + axisX) * DEG2RAD) };
            camera.position = Vector3Add(camera.position, Vector3Scale(Vector3Scale(offset, delta), 10));
        }

        // look
        camera.target = Vector3Add(camera.position, angle);
    }

    // DrawText(TextFormat("%f\n%f\n\n%f\n%f\n%f horizontal movement not implemented lmao",player->angle,player->tilt,
    //                                                                        player->camera.position.x,
    //                                                                        player->camera.position.y,

    return col.point;
}

void PlayerFPS::Focus() {
    isFocused = true;
    SetCameraMode(camera, CAMERA_CUSTOM);
}

void PlayerFPS::Unfocus() {
    isFocused = false;
}

void PlayerFPS::Teleport(Vector3 pos) {
    camera.position = pos;
}
