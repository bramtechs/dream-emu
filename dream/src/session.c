#define MAX_ENTITIES 1024

typedef struct {
    Color skyColor;
    Color fogColor;
    float fogDistance;
} Environment;

inline Environment environment_default(){
    Environment env = { 0 };
    env.skyColor = SKYBLUE;
    env.fogColor = GRAY;
    env.fogDistance = 0.2f;
    return env;
}

#include "entity.c"

typedef struct {
    Environment env;
    EntityGroup entities;
} Scene;

static Scene *CurrentScene;

#include "editor.c"

void session_init(void)
{
    CurrentScene = MemAlloc(sizeof(Scene));
    CurrentScene->env = environment_default();
    CurrentScene->entities.count = 0;

    for (int i = 0; i < 1000; i++){
        EntityContainer cont = {0};
        cont.type = ENTITY_BLOCK;
        cont.entity.block.base = base_random();
        // cont.entity.block.
        entity_add(&CurrentScene->entities,cont);
    }
}

void session_update_and_render(float delta)
{
    ClearBackground(CurrentScene->env.skyColor);
    DrawGrid(10, 1);

    entity_update_all(&CurrentScene->entities,delta);
    entity_draw_all(&CurrentScene->entities);
}

void session_update_and_render_gui(float delta)
{
    editor_update_and_draw(CurrentScene);
}

void session_dispose()
{
    MemFree(CurrentScene);
    CurrentScene = NULL;
}
