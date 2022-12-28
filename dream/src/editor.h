#pragma once

#define RAYGUI_IMPLEMENTATION

#include "magma.h"
#include "settings.h"
#include "scene.h"

#define MAX_MODELS 128

// bandaid fix to avoid circular dependencies
typedef void SCENE;

typedef enum {
    EDITOR_NORMAL,
    EDITOR_SPAWN,
    EDITOR_MOVE,
    EDITOR_ROTATE,
    EDITOR_SCALE
} EditorMode;

typedef struct {
    int id;
    EditorMode mode;
    EntityID subject;

    float elapsedTime;
    bool prevFreecamMode;

    // model selector
    Model models[MAX_MODELS];
    size_t modelCount;
    size_t selectedModel;

} Editor;

Editor* editor_init(SCENE* scene);

void editor_dispose(Editor* editor);

void editor_update_and_draw(Editor* editor, float delta);

bool editor_update_and_draw_gui(Editor* editor);


