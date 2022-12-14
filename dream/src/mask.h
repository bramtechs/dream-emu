#pragma once

#define MAX_VERTICES 256;

#define COMP_MASK   (1 << 13)

#include "magma.h"
#include "dream_entity.h"

typedef struct {
    Vector3 vertices;
    size_t vertexCount;

    // TODO way to dispose these
    Model model;
    Mesh mesh;

    Base base;
} Mask;


void entity_mask_create(EntityGroup* group, Vector3 pos);

void entity_mask_dispose(void* ptr);

void entity_mask_regenerate(Mask* mask);

void entity_mask_draw(void* ptr);
