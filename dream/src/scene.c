#define MAX_ENTITIES 1024
#include "scene.h"

static Vector3 Feet; // lol

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

Scene* scene_init(UPDATE_FUNC updateFunc)
{
    Scene* scene = MemAlloc(sizeof(Scene));
    scene->env = environment_default();
    scene->group = CreateEntityGroup();

    scene->updateFunc = updateFunc;
    scene->player = SpawnPlayerFPS(1.8f);
    TeleportPlayerFPS(&scene->player,(Vector3) {10,2,3} );

    scene->editor = editor_init(scene);

    return scene;
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
    DrawGroup(scene->group);

    DrawSphere(scene->player.camera.position, 0.2f, GREEN);
    DrawSphere(Feet, 0.3f, PURPLE);

    if (Settings.editorVisible) {

        editor_update_and_draw(scene->editor, delta);
        if (Settings.drawGrid) {
            DrawGrid(1000, 1);
        }
    }
    EndMode3D();

    // ui drawing
            //DrawCircleV(GetScaledMousePosition(), 4.f, RED);                              // Draw a color-filled circle

    DrawFPS(10, 10);

    Feet = UpdatePlayerFPS(&scene->player,scene->group,delta);

    if (((int)GetTime()) % 2 == 0) {
        DrawText("DEMO DISC", WIDTH - MeasureText("DEMO DISC ", 20), HEIGHT - 20, 20, WHITE);
    }

    EndMagmaDrawing();

    scene_update_and_render_gui(scene, delta);

    EndDrawing();
}

void scene_update_and_render_gui(Scene* scene, float delta)
{
    if (Settings.editorVisible) {
        Settings.editorVisible = editor_update_and_draw_gui(scene->editor);
    }
    if (IsKeyPressed(KEY_F3)) {
        Settings.editorVisible = !Settings.editorVisible;
        if (Settings.editorVisible){
            UnfocusPlayerFPS(&scene->player);
        }else{
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
