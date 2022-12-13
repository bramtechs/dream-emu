#pragma once

#include "magma.h"
#include "entity.h"

#define COMP_PICKABLE   (1 << 10)
#define COMP_FLOOR      (1 << 11)
#define COMP_BLOCK      (1 << 12)

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
void entity_block_create(EntityGroup *group, Vector3 pos);
void entity_block_create_rainbow(EntityGroup *group);
