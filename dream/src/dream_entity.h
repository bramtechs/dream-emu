#ifndef DREAM_ENTITY_H
#define DREAM_ENTITY_H

#include "raylib.h"
#include "entity.h"

typedef struct {
    Base base; 
    Texture texture;
} Floor;

typedef struct {
    Base base; 
    Texture texture;
} Block;


void entity_block_draw(void *ptr);

// TODO make scene
void entity_block_create(Entity *root, Vector3 pos, const char* texture);

#endif
