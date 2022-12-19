#pragma once

#define RAYGUI_IMPLEMENTATION
#include "magma.h"
#include "settings.h"
#include "scene.h"

// bandaid fix to avoid circular dependencies
typedef void SCENE;

typedef struct {
	int id;
} Editor;

Editor* editor_init(SCENE* scene);

void editor_dispose(Editor* editor);

void editor_update_and_draw(Editor* editor, float delta);

bool editor_update_and_draw_gui(Editor* editor);


