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

Entity* entity_root(){
    Entity *root = MemAlloc(sizeof(Entity));
    return root;
}

void entity_add(Entity* root, void* data, size_t size, UPDATE_FUNC updateFunc, DRAW_FUNC drawFunc){
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

size_t entity_update_all(Entity* root, float delta){
    assert(root != NULL);

    Entity *next = root->next;
    size_t counter = 0;
    while (next != NULL){
        UPDATE_FUNC func = next->drawFunc;
        if (func != NULL){
            (*func)(next->content,delta);
        }
        next = next->next;
        counter ++;
    }
    return counter;
}

size_t entity_draw_all(Entity* root){
    assert(root != NULL);

    Entity *next = root->next;
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
