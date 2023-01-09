#pragma once

#include "magma.h"
#include "client.h"
#include "editor.h"
#include "settings.h"

extern bool DoDrawGrid;

struct Scene;
typedef struct Scene Scene;

typedef void(*UPDATE_FUNC_SCENE)(Scene*,float);

typedef struct {
    Color skyColor;
    Color fogColor;
    float fogDistance;
} Environment;

inline Environment environment_default();

struct Scene {
    Environment env;
    PlayerFPS player;
    Vector3 spawnPoint;

    UPDATE_FUNC updateFunc;

    EntityGroup *group;
    void* editor;
};

Model scene_gen_skybox_model(const char* skybox);

Scene* scene_init(UPDATE_FUNC_SCENE updateFunc);
Scene* scene_load(const char* fileName, UPDATE_FUNC_SCENE updateFunc);

void scene_update_and_render(Scene* scene, float delta);

void scene_update_and_render_gui(Scene* scene, float delta);

void scene_dispose(Scene* scene);

