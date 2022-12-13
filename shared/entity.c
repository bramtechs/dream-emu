#include "entity.h"

Base base_create(Vector3 pos, Color tint){
    return (Base) {
        pos, Vector3One(), Vector3One(), tint
    };
}

Base base_default(){
    return (Base) {
        Vector3Zero(), Vector3One(), Vector3Zero(), WHITE
    };
}

Base base_random(){
    int MAX_RANGE = 100;

    Vector3 pos = {
        GetRandomValue(-MAX_RANGE,MAX_RANGE),
        GetRandomValue(-MAX_RANGE,MAX_RANGE),
        GetRandomValue(-MAX_RANGE,MAX_RANGE)
    };

    Color col = {
        GetRandomValue(100,255),
        GetRandomValue(100,255),
        GetRandomValue(100,255),
        255
    };

    return base_create(pos,col);
}

BoundingBox base_bounds(Base *base){
    Vector3 halfSize = Vector3Scale(base->size, 0.5f);
    Vector3 startCorner = Vector3Subtract(base->pos, halfSize);
    return (BoundingBox) {
        startCorner,
        Vector3Add(startCorner,base->size)
    };
}

RayCollision base_hits_ray(Base *base, Ray ray){
    BoundingBox box = base_bounds(base);
    return GetRayCollisionBox(ray, box);
}

Group* entity_root(Camera* camera){
    Group *g = new(Group);
    g->camera = camera;
    g->root = new(Entity);
    return g;
}

void entity_add_child(Entity* root, void* data, size_t size, UPDATE_FUNC updateFunc, DRAW_FUNC drawFunc){
    assert(root != NULL);

    if (root->next != NULL){
        entity_add(root->next,data,size,updateFunc,drawFunc);
        return;
    }

    root->next = MemAlloc(sizeof(Entity));
    root->next->updateFunc = updateFunc;
    root->next->drawFunc = drawFunc;

    root->next->content = MemAlloc(size);
    memcpy(root->next->content,data,size);
}

void entity_add(Group* group, void* data, size_t size, UPDATE_FUNC updateFunc, DRAW_FUNC drawFunc){
    assert(group != NULL);

    entity_add_child(group->root,data,size,updateFunc,drawFunc);
}

size_t entity_update_all(Group* group, float delta){
    assert(group != NULL);

    Entity *next = group->root->next;
    size_t counter = 0;
    while (next != NULL){
        UPDATE_FUNC func = next->updateFunc;
        if (func != NULL){
            (*func)(next->content,delta);
        }
        next = next->next;
        counter ++;
    }
    return counter;
}

size_t entity_draw_all(Group* group){
    assert(group != NULL);

    Entity *next = group->root->next;
    size_t counter = 0;
    while (next != NULL){
        DRAW_FUNC func = next->drawFunc;
        if (func != NULL){
            (*func)(next->content);
        }
        next = next->next;
        counter++;
    }
    return counter;
}
