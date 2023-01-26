#include "deflated_assets.h"

int GetAssetType(const char* name) {
    std::string ext = GetFileExtension(name);
    if (ext == ".png" || ext == ".gif" || ext == ".jpg" ||ext == ".jpeg") {
        return TEXTURE;
    }
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") {
        return SOUND;
    }
    if (ext == ".obj" || ext == ".fbx") {
        return MODEL;
    }
    return CUSTOM;
}

DeflationPack::DeflationPack(const char* filePath) {
    succeeded = false;
    std::ifstream stream;
    try {
        stream = std::ifstream(filePath,std::ifstream::binary);
        succeeded = true;
    }
    catch (const std::ifstream::failure& e) {
        TraceLog(LOG_ERROR,"Exception opening asset package at %s",filePath);
        return;
    }

    // >> size
    int64_t count = -1;
    stream.read((char*) &count,sizeof(int64_t));

    bool isCompressed = false;
    // >> compressed flag
    stream.read((char*) &isCompressed,sizeof(bool));

    // >> items
    for (auto i = 0; i < count; i++) {
        // >> item
        RawAsset asset = {};
        stream.read(asset.path, PATH_MAX_LEN);

        stream.read((char*)&asset.size, sizeof(int64_t));
        assert(asset.size > 0); // TODO handle errors properly

        unsigned char* loaded = (unsigned char*) MemAlloc(asset.size);
        stream.read((char*)loaded, asset.size);

        if (isCompressed) {
            int size = 0;
            asset.data = (char*) DecompressData(loaded, asset.size, &size);
            MemFree(loaded);
        }
        else {
            asset.data = (char*) loaded;
        }

        assets.push_back(asset);
        TraceLog(LOG_DEBUG,"Loaded asset %s", asset.path);
    }

    TraceLog(LOG_INFO,("Loaded asset pack %s", filePath));
    stream.close();
}

DeflationPack::~DeflationPack() {
    for (const auto& item : assets) {
        MemFree(item.data);
    }
    TraceLog(LOG_DEBUG,("Disposed asset pack"));
}

void DeflationPack::PrintAssetList() {
    for (const auto& item : assets) {
        TraceLog(LOG_INFO,"asset --> %s (size %d bytes)",item.path,item.size);
    }
}

bool DeflationPack::AssetExists(const char* name) {
    return QueryAsset(name) != NULL;
}

Texture DeflationPack::RequestTexture(const char* name) {
    Image img = RequestImage(name);
    return LoadTextureFromImage(img);
}

Image DeflationPack::RequestImage(const char* name) {
    // check if exists
    const RawAsset* asset = QueryAsset(name);
    if (asset == NULL) {
        TraceLog(LOG_ERROR,"Packaged image with name %s not found!", name);
        return GenImageColor(16,16,PURPLE);
    }
    if (GetAssetType(asset->path) != TEXTURE) {
        TraceLog(LOG_ERROR,"Packaged asset with name %s is not an image/texture!", name);
        return GenImageColor(16,16,PURPLE);
    }
    const char* ext = GetFileExtension(asset->path);
    return LoadImageFromMemory(ext, (const unsigned char*) asset->data, asset->size);
}

RawAsset DeflationPack::RequestCustom(const char* name, const char* ext) {
    // check if exists
    const RawAsset* asset = QueryAsset(name);
    if (asset == NULL) {
        TraceLog(LOG_ERROR,"Packaged palette with name %s not found!", name);
        return {};
    }
    if (GetAssetType(asset->path) != CUSTOM) {
        TraceLog(LOG_ERROR,"Packaged asset with name %s is not custom!", name);
        return {};
    }

    // check extension
    if (ext != NULL) {
        const char* fext = ext;
        if (ext[0] != '.') {
            fext = TextFormat(".%s", ext);
        }
        if (!TextIsEqual(GetFileExtension(asset->path), fext)) {
            TraceLog(LOG_ERROR,"Custom packaged asset is not of type %s", fext);
            return {};
        }
    }
    return *asset;
}

std::vector<std::string> DeflationPack::GetAssetNames() {
    std::vector<std::string> names;
    for (auto& item : assets) {
        names.push_back(item.path);
    }
    return names;
}

const RawAsset* DeflationPack::QueryAsset(const char* name) {
    for (const auto &item : assets) {
        auto base = GetFileNameWithoutExt(item.path);
        if (TextIsEqual(base,name)) {
            return &item;
        }
    }
    return NULL;
}
