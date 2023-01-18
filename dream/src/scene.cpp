#define MAX_ENTITIES 1024
#include "scene.h"

static Vector3 Feet;

Environment::Environment() {
    skyColor = SKYBLUE;
    fogColor = GRAY;
    fogDistance = 0.2f;
}

Environment::Environment(Color sky, Color fog, float distance) {
	skyColor = sky;
	fogColor = fog;
	fogDistance = distance;
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

void Scene::reset() {
    env = Environment();
    spawnPoint = Vector3Zero();

    player = PlayerFPS(1.8f);
    player.Teleport(spawnPoint);

    editor = editor_init(this);
}

void scene_prepare(Scene* scene){
}

Scene::Scene() {
    group = EntityGroup();
    scene_prepare(this);
}

Scene::Scene(const char* fileName) {
    group.LoadGroup(fileName);
    scene_prepare(scene);
}

void Scene::update_and_render(float delta)
{
    BeginMagmaDrawing();

    Camera* cam = &player.camera;
    UpdateCamera(cam);

    BeginMode3D(player.camera);

    ClearBackground(env.skyColor);

    update(delta);

    group.UpdateGroup(delta);
    group.DrawGroup(player.camera,Prefs.drawOutlines);

    if (Prefs.editorVisible) {
        editor_update_and_draw(scene->editor, delta);
    }

    EndMode3D();

    DrawFPS(10, 10);

    Feet = player.Update(&group,delta);

    if (((int)GetTime()) % 2 == 0) {
        DrawText("DEV-DISC", WIDTH - MeasureText("DEV-DISC ", 20), HEIGHT - 20, 20, WHITE);
    }

    EndMagmaDrawing();

    scene_update_and_render_gui(scene, delta);

    EndDrawing();
}

void Scene::update_and_render_gui(float delta)
{
    if (Prefs.editorVisible) {
        Prefs.editorVisible = editor_update_and_draw_gui(scene->editor);
    }
    if (IsKeyPressed(KEY_F3)) {
        Prefs.editorVisible = !Prefs.editorVisible;
        if (Prefs.editorVisible){
            player.Unfocus();
            SetCameraMode(player.camera,CAMERA_FREE);
        } else {
            player.Focus();
        }
    }
    if (IsKeyPressed(KEY_HOME)) {
        player.Teleport(Vector3Zero());
    }
}
