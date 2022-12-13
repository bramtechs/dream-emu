#include "entity.h"

Base CreateBase(Vector3 pos, Color tint){
    return (Base) {
        pos, Vector3One(), Vector3One(), tint
    };
}

Base CreateDefaultBase(){
    return (Base) {
        Vector3Zero(), Vector3One(), Vector3Zero(), WHITE
    };
}

Base CreateRandomBase(){
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

    return CreateBase(pos,col);
}

BoundingBox GetBaseBounds(Base *base){
    Vector3 halfSize = Vector3Scale(base->size, 0.5f);
    Vector3 startCorner = Vector3Subtract(base->pos, halfSize);
    return (BoundingBox) {
        startCorner,
        Vector3Add(startCorner,base->size)
    };
}

RayCollision GetRayCollisionBase(Base *base, Ray ray){
    BoundingBox box = GetBaseBounds(base);
    return GetRayCollisionBox(ray, box);
}

EntityGroup* CreateEntityGroup(Camera* camera){
    EntityGroup *g = new(EntityGroup);
    g->camera = camera;
    g->root = new(Entity);
    return g;
}

// TODO horrible name
void add_group_entity_child(Entity* root, void* data, size_t size, UPDATE_FUNC updateFunc, DRAW_FUNC drawFunc){
    assert(root != NULL);

    if (root->next != NULL){
        add_group_entity_child(root->next,data,size,updateFunc,drawFunc);
        return;
    }

    root->next = MemAlloc(sizeof(Entity));
    root->next->updateFunc = updateFunc;
    root->next->drawFunc = drawFunc;

    root->next->content = MemAlloc(size);
    memcpy(root->next->content,data,size);
}

void AddGroupEntity(EntityGroup* group, void* data, size_t size, UPDATE_FUNC updateFunc, DRAW_FUNC drawFunc){
    assert(group != NULL);

    add_group_entity_child(group->root,data,size,updateFunc,drawFunc);
}

size_t UpdateGroup(EntityGroup* group, float delta){
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

size_t DrawGroup(EntityGroup* group){
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
