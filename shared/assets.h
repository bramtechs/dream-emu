#ifndef ASSETS_H
#define ASSETS_H

#define MAX_ASSETS (1000)

#define TEXTURE_TYPE 1
#define SHADER_TYPE 2

#include "raylib.h"
#include <assert.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    char name[128];
    char type;
    void *memory;
} Asset;

typedef struct {
    char prefix[128];
    Asset assets[MAX_ASSETS];
    int count;
} GameAssets;

extern GameAssets *Assets = NULL;

void assets_load(const char *prefix);
void assets_dispose();

Shader assets_shader(const char *name);
Texture assets_texture(const char *name);

#endif
