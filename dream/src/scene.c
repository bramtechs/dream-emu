#define MAX_ENTITIES 1024
#include "scene.h"


inline Environment environment_default(){
    Environment env = { 0 };
    env.skyColor = SKYBLUE;
    env.fogColor = GRAY;
    env.fogDistance = 0.2f;
    return env;
}

Scene* scene_init(Assets* assets, Camera* camera)
{
    Scene *scene = MemAlloc(sizeof(Scene));
    scene->env = environment_default();
    scene->group = CreateEntityGroup(camera);
    scene->camera = camera;

    scene->editor = editor_init(assets,scene);
    scene->editorVisible = true;

    for (int i = 0; i < 10000; i++) {
        entity_block_create_rainbow(scene->group,Vector3Zero(),camera);
    }

    return scene;
}

void scene_update_and_render(Scene* scene, float delta)
{
    ClearBackground(scene->env.skyColor);

    UpdateGroup(scene->group,delta);
    DrawGroup(scene->group);

    if (scene->editorVisible){
        editor_update_and_draw(scene->editor,delta);
    }
}

void scene_update_and_render_gui(Scene* scene, float delta)
{
    if (scene->editorVisible){
        scene->editorVisible = editor_update_and_draw_gui(scene->editor);
    }
    if (IsKeyPressed(KEY_F3)){
        scene->editorVisible = !scene->editorVisible;
    }
    DrawText("Press F3 for editor",10,HEIGHT*SCALE-20,16,PURPLE);
}

void scene_dispose(Scene *scene)
{
    MemFree(scene);
}

