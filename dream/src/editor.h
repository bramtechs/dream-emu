#pragma once

#include "stdbool.h"
#include "scene.h"

#include "magma.h"

void editor_init(Assets* assets);
void editor_dispose();
bool editor_update_and_draw(Scene* scene);
