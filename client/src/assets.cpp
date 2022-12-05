#define RLIGHTS_IMPLEMENTATION

#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

const char *AssetPrefix = "assets/";

struct GameAssets {
    Mesh *planeMesh;
    Mesh *cubeMesh;

    Model *floorModel;
    Model *cubeModel;

    Image *paletteImage;

    Texture *placeHolderTexture;
    Texture *noiseTexture;

    Shader *lightShader;
    Shader *fogShader;
    int fogShaderDensityLoc;

    // TODO NOT AN ASSET
    Light playerLight;

    // easier for cleaning everything up
    // or rendering everything at once
    std::vector<LevelLayout> levelLayouts;
    std::vector<Image> images;
    std::vector<Texture> textures;
    std::vector<Model> models;
    std::vector<Mesh> meshes;
    std::vector<Shader> shaders;
};

static GameAssets *Assets = nullptr;

Shader *SHADER_LOAD(const char *vsPath, const char *fsPath)
{
    const char *relVsPath = TextFormat("%s/shaders/glsl%i/%s.vs", AssetPrefix, GLSL_VERSION, vsPath);
    const char *relFsPath = TextFormat("%s/shaders/glsl%i/%s.fs", AssetPrefix, GLSL_VERSION, fsPath);
    Shader shader = LoadShader(relVsPath, relFsPath);

    Assets->shaders.push_back(shader);
    return &Assets->shaders.back();

}

Texture *TEXTURE_LOAD(Image img)
{
    if (img.data != nullptr)
    {
        Texture texture = LoadTextureFromImage(img);
        UnloadImage(img);
        Assets->textures.push_back(texture);
        return &Assets->textures.back();
    }
    return Assets->placeHolderTexture;
}

Texture *TEXTURE_LOAD(const char *path)
{
    const char *relpath = TextFormat("%s%s", AssetPrefix, path);
    Image img = LoadImage(relpath);
    return TEXTURE_LOAD(img);
}

Image *IMAGE_LOAD(const char *path)
{
    const char *relpath = TextFormat("%s%s", AssetPrefix, path);
    Image img = LoadImage(relpath);
    Assets->images.push_back(img);
    return &Assets->images.back();
}

Model *MODEL_LOAD(Mesh *mesh)
{
    Model model = LoadModelFromMesh(*mesh);

    // inject fog shader
    model.materials[0].shader = *Assets->fogShader;

    Assets->models.push_back(model);
    return &Assets->models.back();
}

Mesh *MESH_LOAD(Mesh mesh)
{
    Assets->meshes.push_back(mesh);
    return &Assets->meshes.back();
}

LevelLayout *LEVEL_LAYOUT_LOAD(const char *path)
{
    LevelLayout layout = {};

    int paletteCount;
    Color *palColors = LoadImagePalette(*Assets->paletteImage, 512, &paletteCount);
    for (int i = 0; i < paletteCount; i++)
        layout.colors.push_back(palColors[i]);

    Image *levelImg = IMAGE_LOAD(path);
    Color *colors = LoadImageColors(*levelImg);
    for (int i = 0; i < levelImg->width * levelImg->height; i++)
        layout.colors.push_back(colors[i]);

    TraceLog(LOG_INFO, "Loaded %d palette indices...", paletteCount);

    Assets->levelLayouts.push_back(layout);
    return &Assets->levelLayouts.back();
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

    // Shaders
    assets_load_shader_fog();

    // Meshes
    Assets->planeMesh = MESH_LOAD(GenMeshPlane(1, 1, 1, 1));
    Assets->cubeMesh = MESH_LOAD(GenMeshCube(1, 1, 1));

    // Models
    Assets->floorModel = MODEL_LOAD(Assets->planeMesh);
    Assets->cubeModel = MODEL_LOAD(Assets->cubeMesh);

    // Images
    Assets->paletteImage = IMAGE_LOAD("palette.png");

    // Textures
    Assets->placeHolderTexture = TEXTURE_LOAD(GenImageChecked(32, 32, 4, 4, RED, WHITE));
    Assets->noiseTexture = TEXTURE_LOAD("gfx/noise.png");

    // Level layouts
    LEVEL_LAYOUT_LOAD("levels/level001.png");
}

void assets_dispose()
{
    if (Assets == nullptr)
        return;

    for (const auto &item: Assets->models)
    {
        UnloadModel(item);
    }
    for (const auto &item: Assets->meshes)
    {
        UnloadMesh(item);
    }
    for (const auto &item: Assets->textures)
    {
        UnloadTexture(item);
    }

    delete Assets;

    TraceLog(LOG_INFO, "Disposed assets");
}