#include "dream_entity.h"

void entity_block_draw(void *ptr){
    Block *block = (Block*) ptr;
    Base *base = &block->base;

    DrawCubeV(base->pos,base->scale,base->tint);
}

void entity_block_create(Entity* root, Vector3 pos, const char* texture){
    Block block = { 0 };
    block.base = base_create(pos,WHITE);

    entity_add(root,&block,sizeof(Block),NULL,&entity_block_draw);
}

void entity_block_create_rainbow(Entity* root)
{
    Block block = { 0 };
    Color col = {
        GetRandomValue(200,255),
        GetRandomValue(200,255),
        GetRandomValue(200,255),
        255
    };
    int BOUNDS = 100;
    Vector3 pos = {
        GetRandomValue(-BOUNDS,BOUNDS),
        GetRandomValue(-BOUNDS,BOUNDS),
        GetRandomValue(-BOUNDS,BOUNDS),
    };

    block.base = base_create(pos,col);

    entity_add(root,&block,sizeof(Block),NULL,&entity_block_draw);
}
