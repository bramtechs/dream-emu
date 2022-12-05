struct GameAssets {
    Mesh *planeMesh;
    Mesh *cubeMesh;

    Model *floorModel;
    Model *cubeModel;

    Texture* placeHolderTexture;
    Texture* noiseTexture;

    // easier for cleaning everything up
    // or rendering everything at once
    MemoryArray<Texture> textures;
    MemoryArray<Model> models;
    MemoryArray<Mesh> meshes;

    std::string assetPrefix;
};

static GameAssets *Assets = nullptr;

Texture *TEXTURE_LOAD(Image img)
{
    if (img.data != nullptr)
    {
        Texture texture = LoadTextureFromImage(img);
        UnloadImage(img);
        return Assets->textures.push(texture);
    }
    return Assets->placeHolderTexture;
}

Texture *TEXTURE_LOAD(const char *path)
{
    std::string relpath = Assets->assetPrefix + path;
    Image img = LoadImage(relpath.c_str());
    return TEXTURE_LOAD(img);
}

Model *MODEL_LOAD(Mesh *mesh)
{
    Model model = LoadModelFromMesh(*mesh);
    return Assets->models.push(model);
}

Mesh *MESH_LOAD(Mesh mesh)
{
    return Assets->meshes.push(mesh);
}

void assets_load()
{
    Assets = new GameAssets();
    Assets->assetPrefix = "assets/";

    // Meshes
    Mesh *plane = Assets->planeMesh = MESH_LOAD(GenMeshPlane(1, 1, 1, 1));

    // Models
    Assets->floorModel = MODEL_LOAD(plane);

    // Textures
    Assets->placeHolderTexture = TEXTURE_LOAD(GenImageChecked(32, 32, 4, 4, RED, WHITE));
    Assets->noiseTexture = TEXTURE_LOAD("gfx/noise.png");
}

void assets_dispose()
{
    if (Assets == nullptr)
        return;

    for (int i = 0; i < Assets->meshes.count; i++)
        UnloadMesh(*Assets->meshes.get(i));

    for (int i = 0; i < Assets->models.count; i++)
        UnloadModel(*Assets->models.get(i));

    for (int i = 0; i < Assets->textures.count; i++)
        UnloadTexture(*Assets->textures.get(i));

    TraceLog(LOG_INFO, "Disposed assets");
}