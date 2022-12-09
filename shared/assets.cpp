struct Asset {
    void* memory;

    template <typename T>
    Asset(T asset){
        memory = v_malloc(sizeof(asset));
    }
}

struct GameAssets {

    std::unordered_map<std::string,Asset> assets;
    std::string prefix;

    GameAssets(const char* prefix){
        this->prefix = prefix;
    }

    template <typename T>
    T* get_loaded_or_null(const char *name){
        Asset *asset;

        try{
            asset = assets.at(name);
        }
        catch(const out_of_range &e){
            asset = nullptr;
        }

        return asset;
    }

    Shader *SHADER(const char *name, const char *vsPath, const char *fsPath)
    {
        Shader* shader = get_loaded_or_null(name);
        if (shader != nullptr){
            return shader;
        }

        const char *relVsPath = TextFormat("%s/shaders/glsl%i/%s.vs", Assets->assetPrefix.c_str(), GLSL_VERSION, vsPath);
        const char *relFsPath = TextFormat("%s/shaders/glsl%i/%s.fs", Assets->assetPrefix.c_str(), GLSL_VERSION, fsPath);

        Shader shader = LoadShader(relVsPath, relFsPath);
        Asset asset(shader);

        assets.insert({name, asset});
        return get_loaded_or_null(name);
    }

    Texture *TEXTURE(Image img)
    {
        Texture texture;
        if (img.data != nullptr)
        {
            texture = LoadTextureFromImage(img);
        }
        else
        {
            img = GenImageChecked(32, 32, 4, 4, RED, WHITE);
            texture = LoadTextureFromImage(img);
        }
        UnloadImage(img);

        Asset asset(img);
        assets.insert({name, asset});

        return get_loaded_or_null(name);
    }

    Texture *TEXTURE(const char *path)
    {
        std::string relpath = prefix + path;
        Image img = LoadImage(relpath.c_str());
        return TEXTURE(img);
    }

    //Image *IMAGE(const char *path)
    //{
    //    std::string relpath = Assets->assetPrefix + path;
    //    Image img = LoadImage(relpath.c_str());
    //    return Assets->images.push(img);
    //}

    //Model *MODEL(Mesh *mesh)
    //{
    //    Model model = LoadModelFromMesh(*mesh);

    //    // inject fog shader
    //    model.materials[0].shader = *Assets->fogShader;

    //    return Assets->models.push(model);
    //}

    //Mesh *MESH_LOAD(Mesh mesh)
    //{
    //    return Assets->meshes.push(mesh);
    //}
};

static GameAssets *Assets = nullptr;


//void assets_load_shader_fog()
//{
//    Assets->fogShader = SHADER_LOAD("lighting", "fog");
//    Shader shader = *Assets->fogShader;
//
//    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
//    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
//
//    int ambientLoc = GetShaderLocation(shader, "ambient");
//    float sunLight = 0.7f;
//    float ambientLightLevel[] = {sunLight, sunLight, sunLight, 1.0f};
//    SetShaderValue(shader, ambientLoc, &ambientLightLevel, SHADER_UNIFORM_VEC4);
//
//    int fogDensityLoc = GetShaderLocation(shader, "fogDensity");
//
//    Assets->fogShaderDensityLoc = GetShaderLocation(shader, "fogDensity");
//}
//
//void assets_load()
//{
//    Assets = new GameAssets();
//    Assets->assetPrefix = "assets/";
//
//    // Shaders
//    assets_load_shader_fog();
//
//    // Meshes
//    Assets->planeMesh = MESH_LOAD(GenMeshPlane(1, 1, 1, 1));
//    Assets->cubeMesh = MESH_LOAD(GenMeshCube(1, 1, 1));
//
//    // Images
//    Assets->paletteImage = IMAGE_LOAD("palette.png");
//
//    // Models
//    Assets->floorModel = MODEL_LOAD(Assets->planeMesh);
//    Assets->cubeModel = MODEL_LOAD(Assets->cubeMesh);
//
//    // Textures
//    Assets->placeHolderTexture = TEXTURE_LOAD();
//    Assets->noiseTexture = TEXTURE_LOAD("gfx/noise.png");
//    Assets->treeTexture = TEXTURE_LOAD("gfx/tree.png");
//
//    // Level Layouts
//    LAYOUT_LOAD("levels/level001.png");
//}
//
//void assets_dispose()
//{
//    if (Assets == nullptr)
//        return;
//
//    for (int i = 0; i < Assets->models.count; i++)
//        UnloadModel(*Assets->models.get(i));
//
//    for (int i = 0; i < Assets->meshes.count; i++)
//        UnloadMesh(*Assets->meshes.get(i));
//
//    for (int i = 0; i < Assets->textures.count; i++)
//        UnloadTexture(*Assets->textures.get(i));
//
//    TraceLog(LOG_INFO, "Disposed assets");
//}
