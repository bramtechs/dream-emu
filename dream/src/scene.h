#pragma once

#include "magma.h"
#include "client.h"
#include "editor.h"

extern bool DoDrawGrid;

typedef struct {
    Color skyColor;
    Color fogColor;
    float fogDistance;
    char skyboxName[128];
} Environment;

inline Environment environment_default();

typedef struct {
    Environment env;
    EntityGroup *group;
    Camera camera;

    void* editor;
    bool editorVisible;
} Scene;

Scene* scene_init();

void scene_update_and_render(Scene* scene, float delta);

void scene_update_and_render_gui(Scene* scene, float delta);

void scene_dispose(Scene* scene);

