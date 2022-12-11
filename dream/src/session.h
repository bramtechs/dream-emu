#ifndef SESSION_H
#define SESSION_H

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

Scene* session_init(void);

void session_update_and_render(Scene* scene, float delta);

void session_update_and_render_gui(Scene* scene, float delta);

void session_dispose();

#endif
