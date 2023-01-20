#include "magma.h"

static Assets* _Assets = NULL;

bool try_init_assets(const char* folder) {
    if (DirectoryExists(folder)) {
       INFO("Found assets at %s ...",folder);
       assert(ChangeDirectory(folder));
       return true;
    }
    WARN("Did not find assets at %s, keep searching...",folder);
    return false;
}

Assets* Assets::Init(const char* folder) {
    _Assets = new(Assets);

    INFO("Loading assets...");

    if (try_init_assets(folder)) {
        return _Assets;
    }

    // visual studio
    if (try_init_assets("../../assets/")) {
        return _Assets;
    }

    assert(false);
    return NULL;
}

void Assets::Dispose() {
    delete _Assets;
}

Assets::~Assets() {
    for (int i = 0; i < textureCount; i++){
        UnloadTexture(textures[i].texture);
    }
    for (int i = 0; i < modelCount; i++){
        UnloadModel(models[i].model);
    }
}

Texture Assets::RequestTexture(const char* name) {

    // get cached texture
    for (int i = 0; i < _Assets->textureCount; i++){
        TextureContainer cont = _Assets->textures[i];
        if (TextIsEqual(cont.name,name)){
            return cont.texture;
        }
    }

    // load texture from disk
    Texture texture = LoadTexture(name);
    if (texture.width == 0) {
        // failed, generate placeholder instead
        Image temp = GenImageChecked(32, 32, 4, 4, RED, WHITE);
        texture = LoadTextureFromImage(temp);
        UnloadImage(temp);
    }

    // push into texture array
    TextureContainer* cont = &_Assets->textures[_Assets->textureCount];
    strcpy(cont->name, name);
    cont->texture = texture;

    _Assets->textureCount++;
    return texture;
}

Model Assets::RequestModel(const char* name) {
    // get cached model
    for (int i = 0; i < _Assets->modelCount; i++){
        ModelContainer cont = _Assets->models[i];
        if (TextIsEqual(cont.name,name)){
            return cont.model;
        }
    }

    Model model = LoadModel(name);

    // raylib automatically handles if model isn't found
    // NOTE memcopying models doesn't seem to work so you'll have to dispose these things manually for now
    
    // push into model array
    ModelContainer* cont = &_Assets->models[_Assets->modelCount];
    strcpy(cont->name, name);
    cont->model = model;

    _Assets->modelCount++;
    return model;
}

Shader Assets::RequestShader(const char* name){
    Shader shader = LoadShader(0, name);
    return shader;
}

// TODO inline
FilePathList Assets::IndexModels(){
    FilePathList list =  LoadDirectoryFilesEx(".", ".obj", true);
    for (int i = 0; i < list.count; i++){
        DEBUG(">>> %s",list.paths[i]);
    }
    // TODO dispose
    return list;
}
