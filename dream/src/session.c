#define MAX_ENTITIES 1024

#include "session.h"

inline Environment environment_default(){
    Environment env = { 0 };
    env.skyColor = SKYBLUE;
    env.fogColor = GRAY;
    env.fogDistance = 0.2f;
    return env;
}

Scene* session_init(void)
{
    Scene *scene = MemAlloc(sizeof(Scene));
    scene->env = environment_default();
    scene->root = entity_root(); // TODO try to get rid of root node

    //entity_block_create(scene->root,Vector3Zero(),"gfx/noise");

    return scene;
}

void session_update_and_render(Scene* scene, float delta)
{
    ClearBackground(scene->env.skyColor);
    DrawGrid(10, 1);

    entity_update_all(scene->root,delta);
    entity_draw_all(scene->root);

    entity_block_create_rainbow(scene->root);
}

void session_update_and_render_gui(Scene* scene, float delta)
{
    editor_update_and_draw(scene);
}

void session_dispose(Scene *scene)
{
    MemFree(scene);
}
