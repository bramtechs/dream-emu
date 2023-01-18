#define MAX_ENTITIES 1024
#include "scene.h"

static Vector3 Feet;

inline Environment environment_default() {
    Environment env = { 0 };
    env.skyColor = SKYBLUE;
    env.fogColor = GRAY;
    env.fogDistance = 0.2f;
    return env;
}

Model scene_gen_skybox_model(const char* skybox) {
    // TODO put skybox in struct, and dispose properly
    Mesh mesh = GenMeshSphere(-150, 10, 10);
    Model model = LoadModelFromMesh(mesh);
    Texture texture = RequestTexture(skybox);

    Image img = LoadImageFromTexture(texture);
    ImageRotateCCW(&img);

    Texture textureFlipped = LoadTextureFromImage(img);
    model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = textureFlipped;

    return model;
}

void scene_prepare(Scene* scene){
    scene->env = environment_default();
    scene->spawnPoint = Vector3Zero();

    scene->player = SpawnPlayerFPS(1.8f);
    TeleportPlayerFPS(&scene->player, scene->spawnPoint);

    scene->editor = editor_init(scene);
}

Scene* scene_init(UPDATE_FUNC updateFunc){
    Scene* scene = MemAlloc(sizeof(Scene));
    scene->group = CreateEntityGroup();
    scene->updateFunc = updateFunc;

    scene_prepare(scene);

    return scene;
}

Scene* scene_load(const char* fileName, UPDATE_FUNC updateFunc){
    Scene* scene = scene_init(updateFunc);
    LoadEntityGroup(scene->group, fileName);
    scene_prepare(scene);
}

void scene_update_and_render(Scene* scene, float delta)
{
    assert(scene);

    BeginMagmaDrawing();

    Camera* cam = &scene->player.camera;
    UpdateCamera(cam);

    BeginMode3D(scene->player.camera);

    ClearBackground(scene->env.skyColor);

    if (scene->updateFunc != NULL){
        (*scene->updateFunc)((void*)scene,delta);
    }

    UpdateGroup(scene->group, delta);
    DrawGroup(scene->group,&scene->player.camera,Prefs.drawOutlines);

    if (Prefs.editorVisible) {
        editor_update_and_draw(scene->editor, delta);
    }

    EndMode3D();

    DrawFPS(10, 10);

    Feet = UpdatePlayerFPS(&scene->player,scene->group,delta);

    if (((int)GetTime()) % 2 == 0) {
        DrawText("DEV-DISC", WIDTH - MeasureText("DEV-DISC ", 20), HEIGHT - 20, 20, WHITE);
    }

    EndMagmaDrawing();

    scene_update_and_render_gui(scene, delta);

    EndDrawing();
}

void scene_update_and_render_gui(Scene* scene, float delta)
{
    if (Prefs.editorVisible) {
        Prefs.editorVisible = editor_update_and_draw_gui(scene->editor);
    }
    if (IsKeyPressed(KEY_F3)) {
        Prefs.editorVisible = !Prefs.editorVisible;
        if (Prefs.editorVisible){
            UnfocusPlayerFPS(&scene->player);
            SetCameraMode(scene->player.camera,CAMERA_FREE);
        } else {
            FocusPlayerFPS(&scene->player);
        }
    }
    if (IsKeyPressed(KEY_HOME)) {
        TeleportPlayerFPS(&scene->player, Vector3Zero());
    }
}

void scene_dispose(Scene* scene)
{
    DisposeEntityGroup(scene->group);
    MemFree(scene);
}
