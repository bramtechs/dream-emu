#pragma once

#define RAYGUI_IMPLEMENTATION

#include "magma.h"
#include "settings.h"
#include "scene.h"

#define MAX_MODELS 128

#define EDITOR_NORMAL      0
#define EDITOR_SPAWN       1
#define EDITOR_MOVE        2
#define EDITOR_ROTATE      3
#define EDITOR_SCALE       4
#define EDITOR_MODE_COUNT  5

// bandaid fix to avoid circular dependencies
typedef void SCENE;

typedef struct {
    size_t id;
    const char name[40];
    KeyboardKey key;
    Color color;
} EditorMode;

typedef struct {
    size_t mode;
    EntityID subject;

    float elapsedTime;
    bool prevFreecamMode;

    // model selector
    char models[128][MAX_MODELS];
    size_t modelCount;
    size_t selectedModel;

} Editor;

Editor* editor_init(SCENE* scene);

void editor_dispose(Editor* editor);

void editor_update_and_draw(Editor* editor, float delta);

bool editor_update_and_draw_gui(Editor* editor);


