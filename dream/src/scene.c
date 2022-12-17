#define MAX_ENTITIES 1024
#include "scene.h"

static Model TestModel = {0};

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
    scene->editorVisible = false;

    // GARDEN LEVEL: TODO refactor

    EntityID id = AddEntity(scene->group);

    Base base = CreateDefaultBase();
    AddEntityComponent(scene->group, Base, &base, id);

    ModelRenderer renderer = CreateModelRenderer("levels/garden/garden_start.obj",&base);
    AddEntityComponent(scene->group, ModelRenderer, &renderer, id);

    return scene;
}

void scene_update_and_render(Scene* scene, float delta)
{
    ClearBackground(scene->env.skyColor);

    UpdateGroup(scene->group,delta);
    DrawGroup(scene->group);

    if (scene->editorVisible){
        editor_update_and_draw(scene->editor,delta);
        DrawGrid(100,1);
    }

    if (TestModel.materials != NULL) {
        DrawModel(TestModel, Vector3Zero(), 1.f, WHITE);
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

