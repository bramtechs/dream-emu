#ifndef MG_ENTITY_H
#define MG_ENTITY_H

#include "magma.h"

typedef void(*UPDATE_FUNC)(void*,float);
typedef void(*DRAW_FUNC)(void*);

struct Entity;
typedef struct Entity Entity;

struct Entity {
    UPDATE_FUNC updateFunc;
    DRAW_FUNC drawFunc;
    Entity* next;
    void* content;
};

typedef struct {
    Camera* camera;
    Entity* root;
} EntityGroup;

typedef struct {
    Vector3 pos;
    Vector3 size;
    Vector3 rotation;

    Color tint;
} Base;

Base CreateBase(Vector3 pos, Color tint);

Base CreateDefaultBase();

Base CreateRandomBase();

BoundingBox GetBaseBounds(Base *base);

RayCollision GetRayCollisionBase(Base *base, Ray ray);

EntityGroup* CreateEntityGroup(Camera* camera);

void AddGroupEntity(EntityGroup* group, void* data, size_t size, UPDATE_FUNC updateFunc, DRAW_FUNC drawFunc);

size_t UpdateGroup(EntityGroup* group, float delta);

size_t DrawGroup(EntityGroup* group);

#endif
