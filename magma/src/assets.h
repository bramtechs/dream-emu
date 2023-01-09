#pragma once

#include <stdio.h>

#include "raylib.h"
#include "raymath.h"

#include "logger.h"
#include "memory.h"
#include "array.h"

#define MAX_ASSETS 512 

typedef struct {
    char name[128];
    Texture texture;
} TextureContainer;

typedef struct {
    char name[128];
    Model model;
} ModelContainer;

typedef struct {
    TextureContainer textures[MAX_ASSETS];
    size_t textureCount;

    ModelContainer models[MAX_ASSETS];
    size_t modelCount;
} GameAssets;

void InitAssets(const char* folder);
void DisposeAssets();

Texture RequestTexture(const char* name);
Model RequestModel(const char* name);
Shader RequestShader(const char* name);

FilePathList IndexModels();
