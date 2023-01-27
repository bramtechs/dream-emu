#pragma once

#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <filesystem>

#include "raylib.h"

#define PATH_MAX_LEN 128

// API to access "Deflation" asset packs
#define CUSTOM    -1
#define TEXTURE 0
#define MODEL    1
#define SOUND    2

struct RawAsset {
    char path[PATH_MAX_LEN];
    int64_t size;
    char* data;
};

class DeflationPack {

public:
    bool succeeded;
    std::vector<RawAsset> assets;

    DeflationPack(const char* filePath);
    ~DeflationPack();

    void PrintAssetList();

    bool AssetExists(const char* name);
    Image RequestImage(const char* name);
    Model RequestModel(const char* name);
    Texture RequestTexture(const char* name); // NOTE: GL context required!
    RawAsset RequestCustom(const char* name, const char* ext=NULL);

    std::vector<std::string> GetAssetPaths();
    size_t GetAssetCount();

private:
    const RawAsset* QueryAsset(const char* name);
};

Model LoadModelFromMemory(const char *fileType, const unsigned char *fileData, int dataSize); // very scuffed and hacky, not recommended

int GetAssetType(const char* name);
