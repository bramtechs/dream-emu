#include "dream_entity.h"

void entity_block_draw(void *ptr){
    Block *block = (Block*) ptr;
    Base *base = &block->base;

    DrawCubeV(base->pos,base->size,base->tint);
}

// TODO put into a Pickable3D component?!
void entity_block_update(void* ptr, float delta){
    Block *block = (Block*) ptr;
    Base *base = &block->base;

    base->pos = Vector3Lerp(base->pos,Vector3Zero(), 0.0001);
}

void entity_block_create(EntityGroup* group, Vector3 pos){
    Block block = { 0 };
    block.base = CreateBase(pos,WHITE);

    Components comps = COMP_BASE | COMP_BLOCK | COMP_PICKABLE;
    AddGroupEntity(group,&block,sizeof(Block),comps,entity_block_update,entity_block_draw);
}

void entity_block_create_rainbow(EntityGroup* group)
{
    Block block = { 0 };
    Color col = {
        GetRandomValue(100,255),
        GetRandomValue(100,255),
        GetRandomValue(100,255),
        255
    };
    int BOUNDS = 100;
    Vector3 pos = {
        GetRandomValue(-BOUNDS,BOUNDS),
        GetRandomValue(-BOUNDS,BOUNDS),
        GetRandomValue(-BOUNDS,BOUNDS),
    };

    block.base = CreateBase(pos,col);

    Components comps = COMP_BASE | COMP_BLOCK | COMP_PICKABLE;
    AddGroupEntity(group,&block,sizeof(Block),comps,entity_block_update,entity_block_draw);
}
