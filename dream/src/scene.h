#pragma once

#include "magma.h"
#include "dream_entity.h"
#include "client.h"

typedef struct {
    AssetList assetList;
} Editor;

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

    Editor* editor;
    bool editorVisible;
} Scene;

Scene* scene_init(Assets* assets, Camera *camera);

void scene_update_and_render(Scene* scene, float delta);

void scene_update_and_render_gui(Scene* scene, float delta);

void scene_dispose(Scene* scene);

Editor* editor_init(Assets* assets);

void editor_dispose(Editor* editor);

void editor_update_and_draw(Editor* editor, float delta);

bool editor_update_and_draw_gui(Editor* editor);

