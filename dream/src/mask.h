#pragma once

#define COMP_MASK   (1 << 13)

#include "magma.h"
#include "dream_entity.h"

typedef struct {
    Base base;

    // TODO add a way to dispose these
    Model model;
    Mesh mesh;
} Mask;


void entity_mask_create(EntityGroup* group, Vector3 pos);

void entity_mask_dispose(void* ptr);

void entity_mask_regenerate(Mask* mask);

void entity_mask_draw(void* ptr);
