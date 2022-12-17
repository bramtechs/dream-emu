#define MAX_ENTITIES 1024
#include "scene.h"

bool DoDrawGrid = false; // TODO replace

inline Environment environment_default(){
    Environment env = { 0 };
    env.skyColor = SKYBLUE;
    env.fogColor = GRAY;
    env.fogDistance = 0.2f;
    return env;
}

Scene* scene_init(Camera* camera)
{
    Scene *scene = MemAlloc(sizeof(Scene));
    scene->env = environment_default();
    scene->group = CreateEntityGroup(camera);
    scene->camera = camera;

    scene->editor = editor_init(scene);
    scene->editorVisible = true;

    // GARDEN LEVEL: TODO refactor

    EntityID id = AddEntity(scene->group);

    Base base = CreateDefaultBase();
    ModelRenderer renderer = CreateModelRenderer("levels/garden/garden_start.obj",&base);

    AddEntityComponent(scene->group->bases, Base, &base, id);
    AddEntityComponent(scene->group->modelRenderers, ModelRenderer, &renderer, id);

    return scene;
}

void scene_update_and_render(Scene* scene, float delta)
{
    ClearBackground(scene->env.skyColor);

    UpdateGroup(scene->group,delta);
    DrawGroup(scene->group);

    if (scene->editorVisible){
        editor_update_and_draw(scene->editor,delta);
        if (DoDrawGrid) {
			DrawGrid(1000,1);
        }
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
}

void scene_dispose(Scene *scene)
{
    MemFree(scene);
}

