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
    
    Vector2 mouse = GetMousePosition();
    Camera cam = *block->camera;
    Ray ray = GetMouseRay(mouse,cam);
    RayCollision hit = base_hits_ray(base,ray);
    if (hit.hit){
        DrawSphere(hit.point, 0.1f, GREEN);
    }

}

void entity_block_create(Entity* root, Vector3 pos, Camera* camera){
    Block block = { 0 };
    block.base = base_create(pos,WHITE);
    block.camera = camera;

    entity_add(root,&block,sizeof(Block),&entity_block_update,&entity_block_draw);
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
