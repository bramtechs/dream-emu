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

struct EntityContainer{
    Entity entity; 
    UPDATE_FUNC updateFunc;
    DRAW_FUNC drawFunc;
};

struct EntityGroup {
    EntityContainer entities[MAX_ENTITIES];
    int count;
};

void entity_add(EntityGroup* group, EntityContainer entity){
    group->entities[group->count] = entity;
    group->count++; 
}

void entity_update_all(EntityGroup* group, float delta){
    for (int i = 0; i < group->count; i++){
        UPDATE_FUNC func = group->entities[i].updateFunc;
        if (func != NULL){
            (*func)(&group->entities[i].entity,delta);
        }
    }
}

void entity_draw_all(EntityGroup* group){
    for (int i = 0; i < group->count; i++){
        DRAW_FUNC func = group->entities[i].drawFunc;
        if (func != NULL){
            (*func)(&group->entities[i].entity);
        }
    }
}
