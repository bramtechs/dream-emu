#pragma once

#include <iostream>

#include "raylib.h"
#include "raymath.h"

#include "logger.h"
#include "memory_custom.h"

#define MAX_ASSETS 512 

#define ASSET_CUSTOM   0
#define ASSET_TEXTURE  1
#define ASSET_MESH     2
#define ASSET_AUDIO    3

struct TextureContainer{
    char name[128];
    Texture texture;
};

struct ModelContainer{
    char name[128];
    Model model;
};

struct Assets{
    TextureContainer textures[MAX_ASSETS];
    size_t textureCount;

    ModelContainer models[MAX_ASSETS];
    size_t modelCount;

    static Assets* Init(const char* folder);
    static void Dispose();
    ~Assets();

	static Texture RequestTexture(const char* name);
	static Model RequestModel(const char* name);
	static Shader RequestShader(const char* name);

	static FilePathList IndexModels();
};

