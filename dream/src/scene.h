#pragma once

// TODO rename to scene.h

#include "magma.h"
#include "dream_entity.h"
#include "client.h"

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

    bool editorVisible;
} Scene;

#include "editor.h"

Scene* scene_init(Camera *camera);

void scene_update_and_render(Scene* scene, float delta);

void scene_update_and_render_gui(Scene* scene, float delta);

void scene_dispose();