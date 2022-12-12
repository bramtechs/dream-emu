#pragma once
#include "magma.h"

#define MAX_ASSETS 1000

typedef enum {
    AssetTexture,
    AssetImage,
    AssetMesh,
    AssetModel,
    AssetSound,
} AssetType;

typedef struct {
    AssetType type;
    char name[256];
    void* memory;
} Asset;

typedef struct {
    char folder[256];
    Asset assets[MAX_ASSETS];
    size_t count;
} Assets;

typedef struct {
    const char** names;
    size_t count; 
} AssetList;

constructor(Assets);
destructor(Assets);

Assets* LoadAssets(const char* folder);
void UnloadAssets();

AssetList GetLoadedAssetList(Assets *assets);
void UnloadAssetList(AssetList list);
