#pragma once

// TODO rename to scene.h

#include "raylib.h"
#include "raymath.h"
#include "dream_entity.h"

typedef struct {
    Color skyColor;
    Color fogColor;
    float fogDistance;
} Environment;

inline Environment environment_default();

typedef struct {
    Environment env;
    Entity *root;
} Scene;

#include "editor.h"

Scene* scene_init(void);

void scene_update_and_render(Scene* scene, float delta);

void scene_update_and_render_gui(Scene* scene, float delta);

void scene_dispose();
