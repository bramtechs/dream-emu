#include "magma.h"

template<typename T>
class AssetMap : public std::map<std::string,T>{
};

struct IndexedTexture {
    std::string palName; // palette used to extract colors
    Texture texture;
};

struct GameAssets {
    std::vector<RawAsset> assets;

    // caches
    AssetMap<IndexedTexture> indexedTextures;
    AssetMap<Texture> textures;
    AssetMap<Model> models;
    AssetMap<Palette> palettes;
    AssetMap<Shader> shaders;
    AssetMap<Sound> sounds;
    AssetMap<Font> fonts;
};

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
    "temple/assets.mga",
    "../../assets.mga",
    "../../temple/assets.mga"
    "../../temple/assets.mga",
};

bool LoadAssets() {
    INFO("Loading assets...");

    for (const auto& dir : CrawlAssetDirs){
        if (ImportAssetPackage(dir.c_str())){
            // move process into the same folder where assets.mga is
            const char* p = GetDirectoryPath(dir.c_str());
            ChangeDirectory(p);
            break;
        }
    }
    return true;
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
    if (data[0] == 0x4d && data[1] == 0x47 && data[2] == 0x41){
        int decompSize = 0;
        int dataSizeWOHeader = (int) dataSize - 3;
        unsigned char* uncompData = DecompressData(&data[3], dataSizeWOHeader, &decompSize);
        if (decompSize == 0) {
            TraceLog(LOG_ERROR, "Failed decompressing asset package at %s", filePath);
            return false;
        }
        dataStr = std::string((char*)uncompData,decompSize);
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
            WARN("Malformed package item %s",asset.path);
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
    for (const auto& item : Assets.indexedTextures) {
        UnloadTexture(item.second.texture);
    }
#if defined(MAGMA_3D)
    for (const auto& item : Assets.models) {
        UnloadModel(item.second);
    }
#endif
    for (const auto& item : Assets.shaders) {
        UnloadShader(item.second);
    }
    for (const auto& item : Assets.sounds) {
        UnloadSound(item.second);
    }
    for (const auto& item : Assets.fonts) {
        UnloadFont(item.second);
    }

    TraceLog(LOG_DEBUG, "Disposed asset pack");
    UnloadTexture(PlaceholderTexture);
}

// TODO: maybe cache combination name, palette
Texture RequestIndexedTexture(const std::string& name, Palette palette) {
    // check if previously cached
    for (const auto& item : Assets.indexedTextures) {
        if (item.first == name) {
            return item.second.texture;
        }
    }

    Texture standardTexture = RequestTexture(name);
    Image img = LoadImageFromTexture(standardTexture);
    palette.MapImage(img);
    Texture texture = LoadTextureFromImage(img);
    UnloadImage(img);

    // cache result
    IndexedTexture cont;
    cont.palName = std::string(palette.name,64);
    cont.texture = texture;
    Assets.indexedTextures.insert({name, cont});

    return texture;
}

bool HasDefaultPalette(){
    return Window.hasDefaultPalette;
}

static bool HasWarned = false;
Texture RequestIndexedTexture(const std::string& name) {
    Texture texture;
    if (HasDefaultPalette()){
        texture = RequestIndexedTexture(name,Window.defaultPalette);
    }
    else {
        if (!HasWarned){
            WARN("No default palette set to load indexed texture!");
            HasWarned = true;
        }
        texture = RequestTexture(name);
    }
    return texture;
}

Texture RequestPlaceholderTexture() {
    return PlaceholderTexture;
}

Texture RequestTexture(const std::string& name) {
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
        // ATTEMPT 3: load texture from disk
        const char* path = TextFormat("raw_assets/%s", name.c_str());
        texture = LoadTexture(name.c_str());
        if (texture.width == 0) {
            // generate placeholder on fail
            Image temp = GenImageChecked(32, 32, 4, 4, RED, WHITE);
            texture = LoadTextureFromImage(temp);
            UnloadImage(temp);
        }
    }

    // push into texture array
    Assets.textures.insert({ name,texture });
    return texture;
}

Image RequestImage(const std::string& name) {
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
        // ATTEMPT 2: load image from disk
        const char* path = TextFormat("raw_assets/%s", name.c_str());
        image = LoadImage(name.c_str());
        if (image.width == 0) {
            image = GenImageChecked(32, 32, 4, 4, RED, WHITE);
        }
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

#ifdef MAGMA_3D
Model RequestModel(const std::string& name) {

    // ATTEMPT 1: get cached model
    for (const auto& item : Assets.models) {
        if (item.first == name) {
            return item.second;
        }
    }

    Model model = {};
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
        std::string tempDir = GetTempDirectory();

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

    // push into model cache array
    Assets.models.insert({ name,model });
    return model;
}
#endif

Shader RequestShader(const std::string& name) {
    // ATTEMPT 1: Load shader from cache
    for (const auto &item : Assets.shaders){
        if (item.first == name){
            return item.second;
        }
    }

    // ATTEMPT 2: Load from asset package 
    
    // fragment shader
    std::string frag = "";

    if (IsAssetLoaded(name)){
        RawAsset asset = QueryAsset(name, ".fs");
        if (asset.data != NULL) {
            if (GetAssetType(asset.path) == ASSET_FRAG_SHADER) {
                // read from memory
                frag = std::string(asset.data,asset.size);
            }
            else {
                TraceLog(LOG_ERROR, "Packaged asset with name %s is not a fragment shader!", name);
            }
        }
    }
    else{
        // ATTEMPT 3: Load fragment shader from disk
        const char* fragPath = TextFormat("%s.fs",name.c_str());
        if (FileExists(fragPath)){
            char* fragC = LoadFileText(fragPath);
            frag = fragC;
            UnloadFileText(fragC);
        }
    }

    // vertex shader
    std::string vert = "";

    if (IsAssetLoaded(name)){
        RawAsset asset = QueryAsset(name, ".vs");
        if (asset.data != NULL) {
            if (GetAssetType(asset.path) == ASSET_VERT_SHADER) {
                // read from memory
                frag = std::string(asset.data,asset.size);
            }
            else {
                TraceLog(LOG_ERROR, "Packaged asset with name %s is not a vertex shader!", name);
            }
        }
    }
    else{
        // ATTEMPT 3: Load fragment shader from disk
        const char* vertPath = TextFormat("%s.vs",name.c_str());
        if (FileExists(vertPath)){
            char* vertC = LoadFileText(vertPath);
            vert = vertC;
            UnloadFileText(vertC);
        }
    }

    // finally load the shader
    Shader shader = LoadShaderFromMemory(vert.empty() ? NULL:vert.c_str(), frag.empty() ? NULL:frag.c_str());
    Assets.shaders.insert({name,shader});
    return shader;
}

Sound RequestSound(const std::string& name){
    // ATTEMPT 1: Load sound from cache
    for (const auto &item : Assets.sounds){
        if (item.first == name){
            return item.second;
        }
    }

    // ATTEMPT 2: Load from asset package 
    Wave wave = {};
    if (IsAssetLoaded(name)){
        RawAsset asset = QueryAsset(name,".wav");
        if (asset.data != NULL){
            if (GetAssetType(asset.path) == ASSET_SOUND) {
                // read from memory
                wave = LoadWaveFromMemory(".wav", (const unsigned char*) asset.data, asset.size);
            }
            else {
                TraceLog(LOG_ERROR, "Packaged asset with name %s is not a sound effect!", name);
                return {};
            }
        }
    }
    else{
        // ATTEMPT 3: Load sound from disk
        const char* wavePath = TextFormat("%s.wav",name.c_str());
        if (FileExists(wavePath)){
            wave = LoadWave(wavePath);
        }
        else{
            return {};
        }
    }

    // finally load the sound 
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);

    Assets.sounds.insert({name,sound});
    return sound;
}

Font RequestFont(const std::string& name){
    // ATTEMPT 1: Load font from cache
    for (const auto &item : Assets.fonts){
        if (item.first == name){
            return item.second;
        }
    }

    Font font;
    // ATTEMPT 2: Load from asset package
    if (IsAssetLoaded(name)){
        RawAsset asset = QueryAsset(name,".ttf");
        if (asset.data != NULL){
            if (GetAssetType(asset.path) == ASSET_FONT) {
                // read from memory
                font = LoadFontFromMemory(".ttf", (const unsigned char*) asset.data, asset.size, 72, NULL, 0); // -- chars and glyphcount
                if (font.baseSize < 0) {
                    TraceLog(LOG_ERROR, "Malformed font %s!", name.c_str());
                    font = GetFontDefault();
                }
            }
            else {
                TraceLog(LOG_ERROR, "Packaged asset with name %s is not a font!", name.c_str());
                font = GetFontDefault();
            }
        }
    }
    else {
         // ATTEMPT 3: Load font from disk
        const char* fontPath = TextFormat("%s.ttf", name.c_str());
        if (FileExists(fontPath)) {
            font = LoadFont(fontPath);
            if (font.baseSize < 0) {
                TraceLog(LOG_ERROR, "Malformed disk font %s!", name.c_str());
                font = GetFontDefault();
            }
        }
        else {
            TraceLog(LOG_WARNING, "Didn't find font with name %s!", name.c_str());
            font = GetFontDefault();
        }
    }

    // Cache the font
    Assets.fonts.insert({name,font});
    return font;
}

Font GetRetroFont(){
    return RequestFont("font_core_retro2");
}

// TODO put in engine
std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

bool parse_triple_ints(const std::string& line, std::vector<int>* results) {
    auto members = split(line, ' ');
    if (members.size() != 3) {
        WARN("Parse triple ints has no three members");
        return false;
    }
    for (const auto& item : members) {
        int conv = 0;
        try {
            conv = std::stoi(item);
        }
        catch (std::exception e) {
            WARN("Failed to parse triple ints");
            return false;
        }
        results->push_back(conv);
    }
    return true;
}

Palette ParsePalette(char* text, const char* name) {
    std::string bloat(text);
    std::stringstream stream(bloat);

    int colors[COLORS_PER_PALETTE * 3] = {};

    // TODO check if third line (color count) does not exceed 256

    int colIndex = 0;
    int lineIndex = 0;
    std::string line;
    while (std::getline(stream, line, '\n')) {
        if (lineIndex >= 3) {

            // parse the line
            std::vector<int> members;
            if (parse_triple_ints(line, &members)) {
                colors[colIndex++] = members[0];
                colors[colIndex++] = members[1];
                colors[colIndex++] = members[2];
            }
        }
        lineIndex++;
    }

    Palette pal = {};
    memcpy(pal.colors, colors, sizeof(colors));
    return pal;
}

Palette RequestPalette(const std::string& name) {
    
    // ATTEMPT 1: load palette from cache
    for (const auto& item : Assets.palettes) {
        if (item.first == name) {
            return item.second;
        }
    }

    Palette pal = {};

    // ATTEMPT 2: load palette from package
    if (IsAssetLoaded(name)) {
        size_t size = 0;
        char* data = RequestCustom(name, &size, ".pal");
        pal = ParsePalette(data,name.c_str());
    }
    else {
        // ATTEMPT 3: load image from disk
        const char* path = TextFormat("raw_assets/%s", name.c_str());
        if (FileExists(path)) {
            char* paletteText = LoadFileText(path);
            pal = ParsePalette(paletteText, name.c_str());
            UnloadFileText(paletteText);
        }
    }
    Assets.palettes.insert({ name, pal });
    return pal;
}

void ShowFailScreen(const std::string& text) {
    while (!WindowShouldClose()) {
        BeginMagmaDrawing();
        DrawCheckeredBackground(32, text.c_str(), PURPLE, DARKPURPLE, PINK);
        EndMagmaDrawing();
        EndDrawing();
    }
}

char* RequestCustom(const std::string& name, size_t* size, const char* ext) {
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

std::vector<std::string> GetAssetPaths(AssetType type) {
    std::vector<std::string> paths;
    for (auto& item : Assets.assets) {
        if (type == ASSET_ANY || GetAssetType(item.path) == type){
            paths.push_back(item.path);
        }
    }
    return paths;
}

std::vector<std::string> GetAssetNames(AssetType type) {
    std::vector<std::string> names;
    for (auto& item : Assets.assets) {
        if (type == ASSET_ANY || GetAssetType(item.path) == type){
            const char* name = GetFileNameWithoutExt(item.path);
            names.push_back(name);
        }
    }
    return names;
}

std::vector<std::string> GetTileNames() {
    std::string prefix = "tile_";
    std::vector<std::string> names;
    auto textures = GetAssetNames(ASSET_TEXTURE);
    for (const auto& name : textures){
        if (name.substr(0,prefix.size()) == prefix){
            names.push_back(name);
        }
    }
    return names;
}

AssetType GetAssetType(const char* name) {
    std::string ext = GetFileExtension(name);
    if (ext == ".png" || ext == ".gif" || ext == ".jpg" || ext == ".jpeg") {
        return ASSET_TEXTURE;
    }
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") {
        return ASSET_SOUND;
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
    return ASSET_CUSTOM;
}

void PrintAssetStats() {
    INFO("Fonts %d", Assets.fonts.size());
    INFO("Sounds %d", Assets.sounds.size());
    INFO("Shaders %d", Assets.shaders.size());
    INFO("Palettes %d", Assets.palettes.size());
    INFO("Models %d", Assets.models.size());
    INFO("Textures %d", Assets.textures.size());
    INFO("Paletted/indexed textures %d", Assets.indexedTextures.size());
}

void PrintAssetList() {
    for (const auto& item : Assets.assets) {
        const char* ext = GetFileExtension(item.path);
        const char* name = GetFileName(item.path);
        if (item.size >= 1000){
            int kbSize = item.size / 1000;
            TraceLog(LOG_INFO, "%s --> %s (size %d KB)", ext, name, kbSize);
        }else{
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
