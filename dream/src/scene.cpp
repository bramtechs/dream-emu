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

Scene::Scene() : player(1.8f) {
    spawnPoint = Vector3Zero();
    env = Environment();
    player.Teleport(spawnPoint);
}

Scene::Scene(const char* fileName) {
    spawnPoint = Vector3Zero();
    group.LoadGroup(fileName);
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
    group.DrawGroup();
    if (Prefs.drawOutlines){
        group.DrawGroupDebug(player.camera);
    }

    EndMode3D();

    DrawFPS(10, 10);

    Feet = player.Update(&group,delta);

    if (((int)GetTime()) % 2 == 0) {
        DrawRetroText("DEV-DISC", WIDTH - MeasureText("DEV-DISC ", 20), HEIGHT - 20, 20, WHITE);
    }

    EndMagmaDrawing();

    update_and_render_gui(delta);

    EndDrawing();
}

void Scene::update_and_render_gui(float delta)
{
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
