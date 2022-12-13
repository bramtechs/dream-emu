#ifndef DREAM_ENTITY_H
#define DREAM_ENTITY_H

#include "magma.h"
#include "entity.h"

typedef struct {
    Base base; 
    Texture texture;
} Floor;

typedef struct {
    Base base; 
    Texture texture;
    Camera* camera;
} Block;


void entity_block_draw(void *ptr);

// TODO make scene
void entity_block_create(EntityGroup *group, Vector3 pos, Camera *camera);
void entity_block_create_rainbow(Entity *root);

#endif
