#define MAX_ENTITIES 1024

#include "scene.h"

Vector3 g0 = { -50.0f, 0.0f, -50.0f };
Vector3 g1 = { -50.0f, 0.0f,  50.0f };
Vector3 g2 = { 50.0f, 0.0f,  50.0f };
Vector3 g3 = { 50.0f, 0.0f, -50.0f };

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
    scene->root = entity_root(camera); // TODO try to get rid of root node
    scene->camera = camera;

    entity_block_create(scene->root,Vector3Zero(),camera);

    return scene;
}

void scene_update_and_render(Scene* scene, float delta)
{
    ClearBackground(scene->env.skyColor);

    DrawGrid(10, 1);
    Ray ray = GetMouseRay(GetMousePosition(), *scene->camera);
    RayCollision col = GetRayCollisionQuad(ray, g0, g1, g2, g3);
    if (col.hit && col.distance < 100) {
        DrawSphere(col.point, 0.1f, YELLOW);
    }

    entity_update_all(scene->root,delta);
    entity_draw_all(scene->root);
}

void scene_update_and_render_gui(Scene* scene, float delta)
{
    if (scene->editorVisible){
        scene->editorVisible = editor_update_and_draw(scene);
    }
    if (IsKeyPressed(KEY_F3)){
        scene->editorVisible = !scene->editorVisible;
    }
}

void scene_dispose(Scene *scene)
{
    MemFree(scene);
}
