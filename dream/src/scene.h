#pragma once

#include "magma.h"
#include "dream_entity.h"
#include "client.h"
#include "editor.h"

typedef struct {
    Color skyColor;
    Color fogColor;
    float fogDistance;
} Environment;

inline Environment environment_default();

typedef struct {
    Environment env;
    EntityGroup *group;
    Camera *camera;

    void* editor;
    bool editorVisible;
} Scene;

Scene* scene_init(Camera *camera);

void scene_update_and_render(Scene* scene, float delta);

void scene_update_and_render_gui(Scene* scene, float delta);

void scene_dispose(Scene* scene);

