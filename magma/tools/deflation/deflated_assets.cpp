#include "deflated_assets.h"

static Texture PlaceholderTexture;

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
            asset.size = size;
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

    UnloadTexture(PlaceholderTexture);
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
    if (img.width == 0){ // loading image failed show placeholder
        if (PlaceholderTexture.width == 0){
            Image img = GenImageColor(16,16,RED);
            PlaceholderTexture = LoadTextureFromImage(img);
            UnloadImage(img);
        }
        return PlaceholderTexture;
    }
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
    Image img = LoadImageFromMemory(ext, (const unsigned char*) asset->data, asset->size);
    return img;
}

Model DeflationPack::RequestModel(const char* name) {
    // check if exists
    const RawAsset* asset = QueryAsset(name);
    if (asset == NULL) {
        TraceLog(LOG_ERROR,"Packaged model with name %s not found!", name);
        return LoadModel(""); // force raylib to return default cube
    }
    if (GetAssetType(asset->path) != MODEL) {
        TraceLog(LOG_ERROR,"Packaged asset with name %s is not a model!", name);
        return LoadModel(""); // force raylib to return default cube
    }
    const char* ext = GetFileExtension(asset->path);
    Model model = LoadModelFromMemory(ext, (const unsigned char*) asset->data, asset->size);
    return model;
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

std::vector<std::string> DeflationPack::GetAssetPaths() {
    std::vector<std::string> names;
    for (auto& item : assets) {
        names.push_back(item.path);
    }
    return names;
}

size_t DeflationPack::GetAssetCount() {
    return assets.size();
}

// TODO work with filters instead?
const RawAsset* DeflationPack::QueryAsset(const char* name) {
    std::string wot = std::string(name); // do not remove this line or you will get weird bugs, because const char* likes to change value for some reason
    for (const auto &item : assets) {
        auto base = GetFileNameWithoutExt(item.path);
        auto ext = GetFileExtension(item.path);

        // skip .mtl files
        if (TextIsEqual(ext,".mtl")){
            continue;
        }

        if (TextIsEqual(base,wot.c_str())) {
            return &item;
        }
    }
    return NULL;
}

static std::string GetTempDirectory() {
    std::filesystem::path path = std::filesystem::temp_directory_path();
    std::string tempFolStr = path.string();
    return tempFolStr;
}

// This is a very dumb implementation, but might work
Model LoadModelFromMemory(const char* fileType, const unsigned char *fileData, int dataSize){
    // write file to temp location
    std::string tempDir = GetTempDirectory();
    const char* fileName = TextFormat("%s/model_%d%s", tempDir.c_str(), GetRandomValue(0, 10000), fileType);
    if (SaveFileData(fileName, (void*)fileData, dataSize)) {
        
    }
    else {
        return LoadModel("");
    }
    
    // hijjack raylib to load textures from asset manager instead from disk
    

    // undo hack
    return LoadModel("");
}
