#pragma once

#include "magma.h"
#include "entity.h"
#include "scene.h"

Scene* dream_init_hub();
void dream_update_hub(Scene* scene, float delta);

// garden? more like an empty wasteland for now
Scene* dream_init_garden();
