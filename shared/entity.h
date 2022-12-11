#ifndef MG_ENTITY_H
#define MG_ENTITY_H

#include "raylib.h"
#include <stdio.h>

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
    Vector3 pos;
    Vector3 scale;
    Vector3 rotation;

    Color tint;
} Base;

inline Base base_create(Vector3 pos, Color tint);

inline Base base_default();

inline Base base_random();

void entity_clear(Entity* entity);

void entity_add(Entity* root, void* data, size_t size, UPDATE_FUNC updateFunc, DRAW_FUNC drawFunc);

size_t entity_update_all(Entity* root, float delta);

size_t entity_draw_all(Entity* root);

#endif
