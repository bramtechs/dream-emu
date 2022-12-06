#define RLIGHTS_IMPLEMENTATION

#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

struct GameAssets {
    Mesh *planeMesh;
    Mesh *cubeMesh;

    Model *floorModel;
    Model *cubeModel;

    Texture *placeHolderTexture;
    Texture *noiseTexture;

    Shader *lightShader;
    Shader *fogShader;
    int fogShaderDensityLoc;

    // TODO NOT AN ASSET
    Light playerLight;

    // easier for cleaning everything up
    // or rendering everything at once
    MemoryArray<Texture> textures;
    MemoryArray<Model> models;
    MemoryArray<Mesh> meshes;
    MemoryArray<Shader> shaders;
    MemoryArray<LevelLayout> levelLayouts;

    std::string assetPrefix;
};

static GameAssets *Assets = nullptr;

// TODO make image another asset type
LevelLayout *LAYOUT_LOAD(const char *levelPath, Image palette)
{
    std::string relpath = Assets->assetPrefix + levelPath;
    Image img = LoadImage(relpath.c_str());

    LevelLayout layout = {};
    layout.width = img.width;
    layout.height = img.height;
    layout.colors = LoadImageColors(img);
    layout.paletteColors = LoadImageColors(palette);

    return Assets->levelLayouts.push(layout);

}

Shader *SHADER_LOAD(const char *vsPath, const char *fsPath)
{
    const char *relVsPath = TextFormat("%s/shaders/glsl%i/%s.vs", Assets->assetPrefix.c_str(), GLSL_VERSION, vsPath);
    const char *relFsPath = TextFormat("%s/shaders/glsl%i/%s.fs", Assets->assetPrefix.c_str(), GLSL_VERSION, fsPath);
    Shader shader = LoadShader(relVsPath, relFsPath);

    return Assets->shaders.push(shader);
}

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

    // inject fog shader
    model.materials[0].shader = *Assets->fogShader;

    return Assets->models.push(model);
}

Mesh *MESH_LOAD(Mesh mesh)
{
    return Assets->meshes.push(mesh);
}

void assets_load_shader_fog()
{
    Assets->fogShader = SHADER_LOAD("lighting", "fog");
    Shader shader = *Assets->fogShader;

    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    int ambientLoc = GetShaderLocation(shader, "ambient");
    float sunLight = 0.7f;
    float ambientLightLevel[] = {sunLight, sunLight, sunLight, 1.0f};
    SetShaderValue(shader, ambientLoc, &ambientLightLevel, SHADER_UNIFORM_VEC4);

    float fogDensity = 0.15f;
    int fogDensityLoc = GetShaderLocation(shader, "fogDensity");
    SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);

    Assets->fogShaderDensityLoc = GetShaderLocation(shader, "fogDensity");

    Assets->playerLight = CreateLight(LIGHT_POINT, (Vector3) {0, 0, 0}, Vector3Zero(), WHITE, shader);
}

void assets_load()
{
    Assets = new GameAssets();
    Assets->assetPrefix = "assets/";

    // Shaders
    assets_load_shader_fog();

    // Meshes
    Assets->planeMesh = MESH_LOAD(GenMeshPlane(1, 1, 1, 1));
    Assets->cubeMesh = MESH_LOAD(GenMeshCube(1, 1, 1));

    // Models
    Assets->floorModel = MODEL_LOAD(Assets->planeMesh);
    Assets->cubeModel = MODEL_LOAD(Assets->cubeMesh);

    // Textures
    Assets->placeHolderTexture = TEXTURE_LOAD(GenImageChecked(32, 32, 4, 4, RED, WHITE));
    Assets->noiseTexture = TEXTURE_LOAD("gfx/noise.png");

    // Level Layouts
}

void assets_dispose()
{
    if (Assets == nullptr)
        return;

    for (int i = 0; i < Assets->models.count; i++)
        UnloadModel(*Assets->models.get(i));

    for (int i = 0; i < Assets->meshes.count; i++)
        UnloadMesh(*Assets->meshes.get(i));

    for (int i = 0; i < Assets->textures.count; i++)
        UnloadTexture(*Assets->textures.get(i));

    TraceLog(LOG_INFO, "Disposed assets");
}