// TODO standardize into reusable /shared c file
//

#include "entity.h"

typedef struct {
    Base base; 
    Texture texture;
} Floor;

typedef struct {
    Base base; 
    Texture texture;
} Block;

union Entity {
    Floor floor;
    Block block;
};

#include "entity.c"

void entity_block_draw(Block *block){
    Base *base = &block->base;

    Vector3 size = Vector3Multiply(base->scale, Vector3One());
    DrawCubeV(base->pos,size,base->tint);
}

