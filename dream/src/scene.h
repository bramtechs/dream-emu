#pragma once

#include "magma.h"
#include "client.h"
#include "editor.h"

extern bool DoDrawGrid;

struct Scene;
typedef struct Scene Scene;

typedef void(*UPDATE_FUNC)(Scene*,float);

typedef struct {
    Color skyColor;
    Color fogColor;
    float fogDistance;
} Environment;

inline Environment environment_default();

struct Scene {
    Environment env;
    EntityGroup *group;
    Camera camera;

    UPDATE_FUNC updateFunc;

    void* editor;
    bool editorVisible;
};

Model scene_gen_skybox_model(const char* skybox);

Scene* scene_init(UPDATE_FUNC updateFunc);

void scene_update_and_render(Scene* scene, float delta);

void scene_update_and_render_gui(Scene* scene, float delta);

void scene_dispose(Scene* scene);

