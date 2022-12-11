#ifndef MG_ENTITY
#define MG_ENTITY

union Entity;
typedef union Entity Entity;

typedef void(*UPDATE_FUNC)(Entity*,float);
typedef void(*DRAW_FUNC)(Entity*);

typedef struct {
    Vector3 pos;
    Vector3 scale;
    Vector3 rotation;

    Color tint;
} Base;

inline Base base_create(Vector3 pos, Color tint);

inline Base base_default();

inline Base base_random();

struct EntityContainer;
struct EntityGroup;
typedef struct EntityContainer EntityContainer;
typedef struct EntityGroup EntityGroup;

void entity_add(EntityGroup* group, EntityContainer entity);

void entity_update_all(EntityGroup* group, float delta);

void entity_draw_all(EntityGroup* group);

#endif
