#pragma once

#include "raylib.h"
#include "raymath.h"

#include "window.h"
#include "memory_custom.h"
#include "assets.h"

#include <map>
#include <memory>

#define COMP_ALL                0
#define COMP_BASE               1
#define COMP_MODEL_RENDERER     2

typedef unsigned int uint;
typedef uint ItemType;
typedef uint EntityID;

struct Base {
    EntityID id;
    BoundingBox bounds;
    Color tint;

    // read only
    Vector3 center;
    Vector3 size;
    Vector3 halfSize;
};

struct ModelRenderer{
    EntityID id;
    const char* model;
    bool accurate;
    Vector3 offset; //from base center
};

struct EntityGroup {
    uint entityCount;
    std::multimap<ItemType, std::shared_ptr<void*>> comps;
};

Base CreateBase(EntityID id, Vector3 pos, Color tint);
#define CreateDefaultBase(ID) CreateBase(ID,Vector3Zero(),WHITE)

void TranslateBase(Base* base, Vector3 offset);
#define TranslateBaseX(BASE_PTR,X) TranslateBase(BASE_PTR, (Vector3) {X,0.f,0.f})
#define TranslateBaseY(BASE_PTR,Y) TranslateBase(BASE_PTR, (Vector3) {0.f,Y,0.f})
#define TranslateBaseZ(BASE_PTR,Z) TranslateBase(BASE_PTR, (Vector3) {0.f,0.f,Z})
#define TranslateBaseXYZ(BASE_PTR,X,Y,Z) TranslateBase(BASE_PTR, (Vector3) {X,Y,Z})

void SetBaseCenter(Base* base, Vector3 pos);
#define ResetBaseTranslation(BASE_PTR) SetBaseCenter(BASE_PTR, Vector3Zero())

ModelRenderer CreateModelRenderer(EntityID id, const char* modelPath, Base* base);

RayCollision GetRayCollisionGroup(EntityGroup* groups, Ray ray);
RayCollision GetMouseRayCollisionBase(Base base, Camera camera);

bool GetMousePickedBase(EntityGroup* group, Camera camera, Base** result);
bool GetMousePickedBaseEx(EntityGroup* group, Camera camera, Base** result, RayCollision* col);

void LoadEntityGroup(EntityGroup* group, const char* fileName);
void SaveEntityGroup(EntityGroup* group, const char* fileName);

EntityID AddEntity(EntityGroup* group);

template <typename T>
void AddEntityComponent(EntityGroup* group, ItemType type, EntityID id, T data);

void* GetEntityComponent(EntityGroup* group, EntityID id, ItemType filter);

size_t UpdateGroup(EntityGroup* group, float delta);

size_t DrawGroup(EntityGroup* group, Camera* camera, bool drawOutlines);
