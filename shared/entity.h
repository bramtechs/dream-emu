#ifndef MG_ENTITY_H
#define MG_ENTITY_H

#include "magma.h"

typedef void(*UPDATE_FUNC)(void*,float);
typedef void(*DRAW_FUNC)(void*);

struct Entity;
typedef struct Entity Entity;

// TODO maybe all components should have update and draw function?

struct Entity {
    UPDATE_FUNC updateFunc;
    DRAW_FUNC drawFunc;
    Entity* next;
    void* content;
};

typedef struct {
    Camera* camera;
    Entity* root;
} Group;

typedef struct {
    Vector3 pos;
    Vector3 size;
    Vector3 rotation;

    Color tint;
} Base;

Base base_create(Vector3 pos, Color tint);

Base base_default();

Base base_random();

BoundingBox base_bounds(Base *base);

RayCollision base_hits_ray(Base *base, Ray ray);

Group* entity_root(Camera* camera);

void entity_add(Group* group, void* data, size_t size, UPDATE_FUNC updateFunc, DRAW_FUNC drawFunc);

size_t entity_update_all(Group* group, float delta);

size_t entity_draw_all(Group* group);

#endif
