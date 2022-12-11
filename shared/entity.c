inline Base base_create(Vector3 pos, Color tint){
    return (Base) {
        pos, Vector3One(), Vector3One(), tint
    };
}

inline Base base_default(){
    return (Base) {
        Vector3Zero(), Vector3One(), Vector3Zero(), WHITE
    };
}

inline Base base_random(){
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

void entity_clear(Entity* entity){
    entity->updateFunc = NULL; 
    entity->drawFunc = NULL; 
    entity->next = NULL; 
    entity->content = NULL; 
}

void entity_add(Entity* root, void* data, size_t size, UPDATE_FUNC updateFunc, DRAW_FUNC drawFunc){
    Entity *next = root->next;
    while (next != NULL){
        next = next->next;
    }

    assert(next == NULL);
    next = MemAlloc(sizeof(Entity));
    next.updateFunc = updateFunc;
    next.drawFunc = drawFunc;

    next.content = MemAlloc(size);
    memcpy(next.content,data,size);

    next.next = NULL;
}

size_t entity_update_all(Entity* root, float delta){
    Entity *next = root->next;
    while (next != NULL){
        UPDATE_FUNC func = next.drawFunc;
        (*func)(next.content,delta);
        next = next->next;
    }
}

void entity_draw_all(Entity* root){
    Entity *next = root->next;
    while (next != NULL){
        DRAW_FUNC func = next.drawFunc;
        (*func)(next.content);
        next = next->next;
    }
}
