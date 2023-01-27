#include "magma.h"

#define PATH_MAX_LEN 128

#define CUSTOM    -1
#define TEXTURE 0
#define MODEL    1
#define SOUND    2

struct RawAsset {
    char path[PATH_MAX_LEN];
    int64_t size;
    char* data;
};

struct GameAssets {
    std::vector<RawAsset> assets;

    // caches
    std::map<std::string, Texture> textures;
    std::map<std::string, Model> models;
};

static GameAssets Assets = {};
static Texture PlaceholderTexture = {};

// TODO work with filters instead?
const RawAsset* QueryAsset(const char* name) {
    std::string wot = std::string(name); // do not remove this line or you will get weird bugs, because const char* likes to change value for some reason
    for (const auto &item : Assets.assets) {
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

bool LoadAssets() {
    INFO("Loading assets...");

    if (!LoadAssetPackage("assets.mga")){
        // visual studio
        return LoadAssetPackage("../../assets.mga");
    }
    return true;
}

bool LoadAssetPackage(const char* filePath){
    std::ifstream stream;
    try {
        stream = std::ifstream(filePath,std::ifstream::binary);
    }
    catch (const std::ifstream::failure& e) {
        TraceLog(LOG_ERROR,"Exception opening asset package at %s",filePath);
        return false;
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

        Assets.assets.push_back(asset);
        TraceLog(LOG_DEBUG,"Loaded asset %s", asset.path);
    }

    TraceLog(LOG_INFO,("Loaded asset pack %s", filePath));
    stream.close();

    return true;
}

void DisposeAssets() {
    for (const auto& item : Assets.assets) {
        MemFree(item.data);
    }
    for (const auto& item : Assets.textures) {
        UnloadTexture(item.second);
    }
    for (const auto& item : Assets.models) {
        UnloadModel(item.second);
    }

    TraceLog(LOG_DEBUG,("Disposed asset pack"));
    UnloadTexture(PlaceholderTexture);
}

Texture RequestTexture(const char* name) {
    // ATTEMPT 1: get cached texture
    for (const auto& item : Assets.textures) {
        if (item.first == name) {
            return item.second;
        }
    }

    // ATTEMPT 2: load texture from package
    Texture texture = {};
    if (IsAssetLoaded(name)) {
        Image img = RequestImage(name);
        if (img.width == 0){ // loading image failed show placeholder
            if (PlaceholderTexture.width == 0){
                Image img = GenImageColor(16,16,RED);
                PlaceholderTexture = LoadTextureFromImage(img);
                UnloadImage(img);
            }
            return PlaceholderTexture;
        }
        texture = LoadTextureFromImage(img);
    }
    else {
        // ATTEMPT 3: load texture from disk
        const char* path = TextFormat("raw_assets/%s", name);
        texture = LoadTexture(name);
        if (texture.width == 0) {
            // FAILED: generate placeholder instead
            Image temp = GenImageChecked(32, 32, 4, 4, RED, WHITE);
            texture = LoadTextureFromImage(temp);
            UnloadImage(temp);
        }
    }

    // push into texture array
    Assets.textures.insert({ name,texture });
    return texture;
}

Image RequestImage(const char* name) {
    // ATTEMPT 1: load image from package
    Image image = {};
    if (IsAssetLoaded(name)) {
        // check if exists
        const RawAsset* asset = QueryAsset(name);
        if (asset == NULL) {
            TraceLog(LOG_ERROR,"Packaged image with name %s not found!", name);
            return GenImageColor(16,16,PURPLE);
        }
        if (GetAssetType(asset->path) != TEXTURE) {
            TraceLog(LOG_ERROR,"Packaged asset with name %s is not an image/texture!", name);
            return GenImageColor(16,16,PINK);
        }
        const char* ext = GetFileExtension(asset->path);
        image = LoadImageFromMemory(ext, (const unsigned char*) asset->data, asset->size);
    }
    else {
        // ATTEMPT 2: load image from disk
        const char* path = TextFormat("raw_assets/%s", name);
        image = LoadImage(name);
        if (image.width == 0) {
            // FAILED: generate placeholder instead
            image = GenImageChecked(32, 32, 4, 4, RED, WHITE);
        }
    }

    return image;
}

Model RequestModel(const char* name) {

    // ATTEMPT 1: get cached model
    for (const auto& item : Assets.models) {
        if (item.first == name) {
            return item.second;
        }
    }

    Model model = {};
    // ATTEMPT 2: Load model from package
    if (IsAssetLoaded(name)){
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
        model = LoadModelFromMemory(ext, (const unsigned char*) asset->data, asset->size);
    }
    else{
        // ATTEMPT 3: Load model from disk
        const char* path = TextFormat("raw_assets/%s", name);
        model = LoadModel(name);
    }

    // raylib automatically handles if model isn't found

    // push into model cache array
    Assets.models.insert({ name,model });
    return model;
}

Shader RequestShader(const char* name){
    Shader shader = LoadShader(0, name);
    return shader;
}

Palette ParsePalette(char* text) {
    std::string bloat(text);
    std::stringstream stream(bloat);

    int lineIndex = 0;
    std::string line;
    while (std::getline(stream, line, '\n')) {
        if (lineIndex >= 2) {
            std::cout << line << std::endl;
        }
        lineIndex++;
    }
    // TODO
    assert(false);
    return {};
}

Palette RequestPalette(const char* name) {
    // ATTEMPT 1: load palette from package
    if (IsAssetLoaded(name)) {
        size_t size = 0; 
        char* data = RequestCustom(name, &size, ".pal");

        // todo load palette in
        Palette pal = ParsePalette(data);
        return pal;
    }

    // ATTEMPT 2: load image from disk
    const char* path = TextFormat("raw_assets/%s", name);
    if (FileExists(path)) {
        char* paletteText = LoadFileText(path);
        Palette pal = ParsePalette(paletteText);
        UnloadFileText(paletteText);
        return pal;
    }
    return {};
}

void ShowFailScreen(const char* text) {
    while (!WindowShouldClose()) {
        BeginMagmaDrawing();
        DrawCheckeredBackground(32, text, PURPLE, DARKPURPLE, PINK);
        EndMagmaDrawing();
        EndDrawing();
    }
}

char* RequestCustom(const char* name, size_t* size, const char* ext) {
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

    // return memory
    *size = asset->size;
    return asset->data;
}

std::vector<std::string> GetAssetPaths() {
    std::vector<std::string> names;
    for (auto& item : Assets.assets) {
        names.push_back(item.path);
    }
    return names;
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

void PrintAssetList() {
    for (const auto& item : Assets.assets) {
        TraceLog(LOG_INFO,"asset --> %s (size %d bytes)",item.path,item.size);
    }
}

size_t GetAssetCount() {
    return Assets.assets.size();
}
