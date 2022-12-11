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
