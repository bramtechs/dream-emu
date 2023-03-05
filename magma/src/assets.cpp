// LONG TERM TODO: rewrite in modern c++ with templates -- DRY
// TODO: Deprecate palettes
// TODO: Deprecate disk loading
// TODO: combine music and sound into one asset type

#include "magma.h"

#define PL_MPEG_IMPLEMENTATION

#include "external/pl_mpeg.h"

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <string.h>
#include <filesystem>

template<typename T>
class AssetMap : public std::map<std::string, T> {
};

struct RawAsset {
    char path[PATH_MAX_LEN];
    int64_t size;
    char* data;
};

struct GameAssets {
    std::vector<RawAsset> assets;

    // caches
    AssetMap<Texture> textures;
    AssetMap<Model> models;
    AssetMap<Shader> shaders;
    AssetMap<Sound> sounds;
    AssetMap<Music> music;
    AssetMap<Font> fonts;
    AssetMap<Video> videos;
};

Model LoadOBJFromMemory(const char* fileName);

static GameAssets Assets = {};
static Texture PlaceholderTexture = {};

static RawAsset QueryAsset(const std::string& name, std::string filterExt = "") {
    std::string baseName = GetFileNameWithoutExt(name.c_str());

    for (int i = 0; i < Assets.assets.size(); i++) {
        RawAsset item = Assets.assets.at(i);
        std::string base = GetFileNameWithoutExt(item.path);
        std::string ext = GetFileExtension(item.path);
        if (filterExt.empty() || ext == filterExt) {
            if (base == baseName) {
                RawAsset a;
                memcpy(&a, &item, sizeof(RawAsset));
                return a;
            }
        }
    }
    return {};
}

// traverse each directory until assets.mga is found
static std::vector<std::string> CrawlAssetDirs = {
    "assets.mga",
    "X:/temple/assets.mga"
};

bool LoadAssets() {
    INFO("Loading assets...");

    bool didFind = false;
    for (const auto& dir : CrawlAssetDirs) {
        if (ImportAssetPackage(dir.c_str())) {
            // move process into the same folder where assets.mga is
            const char* p = GetDirectoryPath(dir.c_str());
            ChangeDirectory(p);
            INFO("Moving execution directory to %s...", p);
            didFind = true;
            break;
        }
    }
    return didFind;
}

bool ImportAssetPackage(const char* filePath) {

    // load asset file
    unsigned int dataSize = 0;
    unsigned char* data = LoadFileData(filePath, &dataSize); // TODO: find way to avoid copying data
    if (dataSize == 0) {
        TraceLog(LOG_WARNING, "Failed opening asset package at %s", filePath);
        return false;
    }

    // check if compressed
    std::string dataStr;
    if (data[0] == 0x4d && data[1] == 0x47 && data[2] == 0x41) {
        int decompSize = 0;
        int dataSizeWOHeader = (int)dataSize - 3;
        unsigned char* uncompData = DecompressData(&data[3], dataSizeWOHeader, &decompSize);
        if (decompSize == 0) {
            TraceLog(LOG_ERROR, "Failed decompressing asset package at %s", filePath);
            return false;
        }
        dataStr = std::string((char*)uncompData, decompSize);
        INFO("Package is compressed.");
        MemFree(uncompData);
    }
    else {
        dataStr = std::string((char*)data, dataSize);
        INFO("Package is uncompressed.");
    }
    UnloadFileData(data);

    auto stream = std::stringstream(dataStr);

    // >> size
    int64_t count = -1;
    stream.read((char*)&count, sizeof(int64_t));
    if (count <= 0)
    {
        ERROR("Malformed package, could not determine number of items! (wrong version?)");
        return false;
    }

    // >> items
    for (auto i = 0; i < count; i++) {
        // >> item
        RawAsset asset = {};
        stream.read(asset.path, PATH_MAX_LEN);

        stream.read((char*)&asset.size, sizeof(int64_t));
        if (asset.size <= 0)
        {
            WARN("Malformed package item %s", asset.path);
        }

        unsigned char* loaded = (unsigned char*)MemAlloc(asset.size);
        stream.read((char*)loaded, asset.size);
        asset.data = (char*)loaded;

        Assets.assets.push_back(asset);
        TraceLog(LOG_DEBUG, "Loaded asset %s", asset.path);
    }

    TraceLog(LOG_INFO, ("Loaded asset pack %s", filePath));
    return true;
}

void DisposeAssets() {
    for (const auto& item : Assets.assets) {
        if (item.size > 0) {
            MemFree(item.data);
        }
    }
    for (const auto& item : Assets.textures) {
        UnloadTexture(item.second);
    }
    for (const auto& item : Assets.models) {
        UnloadModel(item.second);
    }
    for (const auto& item : Assets.shaders) {
        UnloadShader(item.second);
    }
    for (const auto& item : Assets.sounds) {
        UnloadSound(item.second);
    }
    for (const auto& item : Assets.fonts) {
        UnloadFont(item.second);
    }
    for (const auto& item : Assets.videos) {
        UnloadVideo(item.second,false);
    }
    TraceLog(LOG_DEBUG, "Disposed asset pack");
    UnloadTexture(PlaceholderTexture);
}

Texture RequestPlaceholderTexture() {
    return PlaceholderTexture;
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
        if (img.width == 0) { // loading image failed show placeholder
            if (PlaceholderTexture.width == 0) {
                Image img = GenImageColor(16, 16, RED);
                PlaceholderTexture = LoadTextureFromImage(img);
                UnloadImage(img);
            }
            return PlaceholderTexture;
        }
        texture = LoadTextureFromImage(img);
    }
    else {
        // generate placeholder on fail
        Image temp = GenImageChecked(32, 32, 4, 4, RED, WHITE);
        texture = LoadTextureFromImage(temp);
        UnloadImage(temp);
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
        RawAsset asset = QueryAsset(name, ".png");
        if (asset.data == NULL) {
            TraceLog(LOG_ERROR, "Packaged image with name %s not found!", name);
            return {};
        }
        if (GetAssetType(asset.path) != ASSET_TEXTURE) {
            TraceLog(LOG_ERROR, "Packaged asset with name %s is not an image/texture!", name);
            return {};
        }
        const char* ext = GetFileExtension(asset.path);
        image = LoadImageFromMemory(ext, (const unsigned char*)asset.data, asset.size);
    }
    else {
        image = GenImageChecked(32, 32, 4, 4, RED, WHITE);
    }

    return image;
}

// This is a very dumb implementation, but might work
unsigned char* load_filedata_from_pack(const char* fileName, unsigned int* bytesRead) {
    const char* name = GetFileNameWithoutExt(fileName);
    const char* ext = GetFileExtension(fileName);
    char* data = RequestCustom(name, (size_t*)bytesRead, ext);
    DEBUG("Rerouting file data %s from memory...", fileName);
    return (unsigned char*)data;
}

char* load_filetext_from_pack(const char* fileName) {
    const char* name = GetFileNameWithoutExt(fileName);
    const char* ext = GetFileExtension(fileName);
    char* data = RequestCustom(name, NULL, ext);
    DEBUG("Rerouting file text %s from memory...", fileName);
    return (char*)TextFormat("%s\0", data); // add a null-terminator, just to be sure
}


Model RequestModel(const char* name) {
    ERROR("Implement model loading again!");

    // ATTEMPT 1: get cached model
    for (const auto& item : Assets.models) {
        if (item.first == name) {
            return item.second;
        }
    }

    Model model = {};
#if 0
    // ATTEMPT 2: Load model from package
    if (IsAssetLoaded(name)) {
        // check if exists
        RawAsset asset = QueryAsset(name, ".obj");
        if (asset.data == NULL) {
            TraceLog(LOG_ERROR, "Packaged model with name %s not found!", name.c_str());
            return LoadModel(""); // force raylib to return default cube
        }
        if (GetAssetType(asset.path) != ASSET_MODEL) {
            TraceLog(LOG_ERROR, "Packaged asset with name %s is not a model!", name.c_str());
            return LoadModel(""); // force raylib to return default cube
        }

        // write material file to temp location
        std::string tempDir = "";

        const char* origDir = GetWorkingDirectory();
        assert(ChangeDirectory(tempDir.c_str()));

        size_t size = 0;
        char* mtlData = RequestCustom(name, &size, ".mtl");
        if (size != 0) {
            const char* fileName = TextFormat("%s.mtl", name.c_str());
            assert(SaveFileData(fileName, mtlData, size));
        }

        const char* ext = GetFileExtension(asset.path);

        // load textures from memory instead of disk
        SetLoadFileDataCallback(load_filedata_from_pack);
        SetLoadFileTextCallback(load_filetext_from_pack);

        model = LoadOBJFromMemory(asset.path);

        SetLoadFileTextCallback(NULL);
        SetLoadFileDataCallback(NULL);

        assert(ChangeDirectory(origDir));
    }
    else {
        // ATTEMPT 3: Load model from disk
        const char* path = TextFormat("raw_assets/%s", name);
        model = LoadModel(name.c_str());
    }

    // raylib automatically handles if model isn't found
#endif
    // push into model cache array
    Assets.models.insert({ name,model });
    return model;
}

Shader RequestShader(const char* name) {
    // ATTEMPT 1: Load shader from cache
    for (const auto& item : Assets.shaders) {
        if (item.first == name) {
            return item.second;
        }
    }

    // ATTEMPT 2: Load from asset package 

    // fragment shader
    std::string frag = "";

    if (IsAssetLoaded(name)) {
        RawAsset asset = QueryAsset(name, ".fs");
        if (asset.data != NULL) {
            if (GetAssetType(asset.path) == ASSET_FRAG_SHADER) {
                // read from memory
                frag = std::string(asset.data, asset.size);
            }
            else {
                TraceLog(LOG_ERROR, "Packaged asset with name %s is not a fragment shader!", name);
            }
        }
    }

    // vertex shader
    std::string vert = "";

    if (IsAssetLoaded(name)) {
        RawAsset asset = QueryAsset(name, ".vs");
        if (asset.data != NULL) {
            if (GetAssetType(asset.path) == ASSET_VERT_SHADER) {
                // read from memory
                frag = std::string(asset.data, asset.size);
            }
            else {
                TraceLog(LOG_ERROR, "Packaged asset with name %s is not a vertex shader!", name);
            }
        }
    }

    // finally load the shader
    Shader shader = LoadShaderFromMemory(vert.empty() ? NULL : vert.c_str(), frag.empty() ? NULL : frag.c_str());
    Assets.shaders.insert({ name,shader });
    return shader;
}

Sound RequestSound(const char* name) {
    // ATTEMPT 1: Load sound from cache
    for (const auto& item : Assets.sounds) {
        if (item.first == name) {
            return item.second;
        }
    }

    // ATTEMPT 2: Load from asset package 
    Wave wave = {};
    if (IsAssetLoaded(name)) {
        RawAsset asset = QueryAsset(name, ".wav");
        if (asset.data != NULL) {
            if (GetAssetType(asset.path) == ASSET_SOUND) {
                // read from memory
                wave = LoadWaveFromMemory(".wav", (const unsigned char*)asset.data, asset.size);
            }
            else {
                TraceLog(LOG_ERROR, "Packaged asset with name %s is not a sound effect!", name);
                return {};
            }
        }
    }

    // finally load the sound 
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);

    Assets.sounds.insert({ name,sound });
    return sound;
}

// NOTE: Maybe music shouldn't be cached at all
Music RequestMusic(const char* name) {
    // ATTEMPT 1: Load music from cache
    for (const auto& item : Assets.music) {
        if (item.first == name) {
            return item.second;
        }
    }

    // ATTEMPT 2: Load from asset package 
    Music music = {};
    if (IsAssetLoaded(name)) {
        RawAsset asset = QueryAsset(name, ".ogg");
        if (asset.data != NULL) {
            if (GetAssetType(asset.path) == ASSET_MUSIC) {
                // read from memory
                music = LoadMusicStreamFromMemory(".ogg", (const unsigned char*)asset.data, asset.size);
            }
            else {
                TraceLog(LOG_ERROR, "Packaged asset with name %s is not music!", name);
                return {};
            }
        }
    }

    Assets.music.insert({ name,music });
    return music;
}

Font RequestFont(const char* name) {
    // ATTEMPT 1: Load font from cache
    for (const auto& item : Assets.fonts) {
        if (item.first == name) {
            return item.second;
        }
    }

    Font font;
    // ATTEMPT 2: Load from asset package
    if (IsAssetLoaded(name)) {
        RawAsset asset = QueryAsset(name, ".ttf");
        if (asset.data != NULL) {
            if (GetAssetType(asset.path) == ASSET_FONT) {
                // read from memory
                font = LoadFontFromMemory(".ttf", (const unsigned char*)asset.data, asset.size, 72, NULL, 0); // -- chars and glyphcount
                if (font.baseSize < 0) {
                    TraceLog(LOG_ERROR, "Malformed font %s!", name);
                    font = GetFontDefault();
                }
            }
            else {
                TraceLog(LOG_ERROR, "Packaged asset with name %s is not a font!", name);
                font = GetFontDefault();
            }
        }
    }
    else {
        // ATTEMPT 3: Load font from disk
        const char* fontPath = TextFormat("%s.ttf", name);
        if (FileExists(fontPath)) {
            font = LoadFont(fontPath);
            if (font.baseSize < 0) {
                TraceLog(LOG_ERROR, "Malformed disk font %s!", name);
                font = GetFontDefault();
            }
        }
        else {
            TraceLog(LOG_WARNING, "Didn't find font with name %s!", name);
            font = GetFontDefault();
        }
    }

    // Cache the font
    Assets.fonts.insert({ name,font });
    return font;
}

Font GetRetroFont() {
    return RequestFont("font_core_retro2");
}

Video RequestVideo(const char* name) {
    // Request music of the video

    // TODO: ugly
    std::string videoPrefix = "video_";
    std::string nameStr = name;
    std::string musicName = "music_" + nameStr.substr(videoPrefix.length());
    Music audio = RequestMusic(musicName.c_str());

    // ATTEMPT 1: Load video from cache
    for (const auto& item : Assets.videos) {
        if (item.first == name) {
            return item.second;
        }
    }

    Video video;
    // ATTEMPT 2: Load from asset package
    if (IsAssetLoaded(name)) {
        RawAsset asset = QueryAsset(name, ".mpeg");
        if (asset.data != NULL) {
            if (GetAssetType(asset.path) == ASSET_VIDEO) {
                // read from memory
                video = LoadVideoWithAudioFromMemory((uint8_t*)asset.data, asset.size, audio);
                if (video.id == -1) {
                    TraceLog(LOG_ERROR, "Malformed video %s!", name);
                }
            }
            else {
                TraceLog(LOG_ERROR, "Packaged asset with name %s is not a video!", name);
            }
        }
    }

    // Cache the video
    Assets.videos.insert({ name,video });
    return video;
}

void ShowFailScreen(const char* text) {
    while (!WindowShouldClose()) {
        BeginMagmaDrawing();
        DrawCheckeredBackground(32, text, PURPLE, DARKPURPLE, PINK, WHITE);
        EndMagmaDrawing();
        EndDrawing();
    }
}

char* RequestCustom(const char* name, size_t* size, const char* ext) {
    // check if exists
    RawAsset asset = QueryAsset(name, ext);
    if (asset.data == NULL) {
        TraceLog(LOG_ERROR, "Packaged custom with name %s not found!", name);
        return {};
    }

    // check extension
    if (ext != NULL) {
        const char* fext = ext;
        if (ext[0] != '.') {
            fext = TextFormat(".%s", ext);
        }
        if (!TextIsEqual(GetFileExtension(asset.path), fext)) {
            TraceLog(LOG_ERROR, "Custom packaged asset is not of type %s", fext);
            return {};
        }
    }

    // return memory
    if (size != NULL) {
        *size = asset.size;
    }

    return asset.data;
}

StringArray GetAssetPaths(AssetType type) {
    StringArray result = {};

    for (auto& item : Assets.assets) {
        if (result.count < STRING_COUNT && (type == ASSET_ANY || GetAssetType(item.path) == type)) {
            strcpy_s(result.entries[result.count++],STRING_MAX_LEN, item.path);
        }
    }

    return result;
}

StringArray GetAssetNames(AssetType type) {
    StringArray result = {};

    std::vector<std::string> names;
    for (auto& item : Assets.assets) {
        if (result.count < STRING_COUNT && (type == ASSET_ANY || GetAssetType(item.path) == type)) {
            const char* name = GetFileNameWithoutExt(item.path);
            strcpy_s(result.entries[result.count++], STRING_MAX_LEN, name);
        }
    }
    return result;
}

StringArray GetTileNames() {
    StringArray result = {};

    std::string prefix = "tile_";
    std::vector<std::string> names;
    auto textures = GetAssetNames(ASSET_TEXTURE);
    for (int i = 0; i < MIN(textures.count,STRING_COUNT); i++) {
        std::string name = textures.entries[i];
        if (name.substr(0, prefix.size()) == prefix) {
            strcpy_s(result.entries[result.count++], STRING_MAX_LEN, name.c_str());
        }
    }
    return result;
}

// TODO: rewrite as struct
AssetType GetAssetType(const char* name) {
    std::string ext = GetFileExtension(name);
    if (ext == ".png" || ext == ".gif" || ext == ".jpg" || ext == ".jpeg") {
        return ASSET_TEXTURE;
    }
    if (ext == ".wav") {
        return ASSET_SOUND;
    }
    if (ext == ".ogg") {
        return ASSET_MUSIC;
    }
    if (ext == ".obj" || ext == ".fbx") {
        return ASSET_MODEL;
    }
    if (ext == ".ttf") {
        return ASSET_FONT;
    }
    if (ext == ".fs") {
        return ASSET_FRAG_SHADER;
    }
    if (ext == ".vs") {
        return ASSET_VERT_SHADER;
    }
    if (ext == ".mpeg") {
        return ASSET_VIDEO;
    }
    return ASSET_CUSTOM;
}

void PrintAssetStats() {
    INFO("Fonts %d", Assets.fonts.size());
    INFO("Sounds %d", Assets.sounds.size());
    INFO("Shaders %d", Assets.shaders.size());
    INFO("Models %d", Assets.models.size());
    INFO("Textures %d", Assets.textures.size());
}

void PrintAssetList() {
    for (const auto& item : Assets.assets) {
        const char* ext = GetFileExtension(item.path);
        const char* name = GetFileName(item.path);
        if (item.size >= 1000) {
            int kbSize = item.size / 1000;
            TraceLog(LOG_INFO, "%s --> %s (size %d KB)", ext, name, kbSize);
        }
        else {
            TraceLog(LOG_INFO, "%s --> %s (size %d bytes)", ext, name, item.size);
        }
    }
}

size_t GetAssetCount() {
    return Assets.assets.size();
}

bool IsAssetLoaded(const std::string& name) {
    RawAsset asset = QueryAsset(name);
    return asset.data != NULL;
}