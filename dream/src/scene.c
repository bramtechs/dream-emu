#define MAX_ENTITIES 1024
#include "scene.h"

bool DoDrawGrid = false; // TODO replace

inline Environment environment_default(){
    Environment env = { 0 };
    env.skyColor = SKYBLUE;
    env.fogColor = GRAY;
    env.fogDistance = 0.2f;

    const char* def = "sky/sky.png";
    strcpy(env.skyboxName,def);

    return env;
}

Model scene_gen_skybox_model(const char* skybox){
    // TODO put skybox in struct, and dispose properly
    Mesh mesh = GenMeshSphere(-150, 10, 10);
    Model model = LoadModelFromMesh(mesh);
    Texture texture = RequestTexture(skybox);

    // Happy little accident, looks really cool, might use later
//    Image img = LoadImageFromTexture(texture);
//    ImageFlipVertical(&img);
//    ImageRotateCW(&img);

    Image img = LoadImageFromTexture(texture);
    ImageRotateCCW(&img);

    Texture textureFlipped = LoadTextureFromImage(img);
    model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = textureFlipped;

    return model;
}

Scene* scene_init(Camera* camera)
{
    Scene *scene = MemAlloc(sizeof(Scene));
    scene->env = environment_default();
    scene->group = CreateEntityGroup(camera);
    scene->camera = camera;

    scene->editor = editor_init(scene);
    scene->editorVisible = true;

    // for aLL levels
    //{
    //    Model sky = scene_gen_skybox_model(scene->env.skyboxName);
    //    EntityID id = AddEntity(scene->group);

    //    Base base = CreateDefaultBase();
    //    ModelRenderer renderer = CreateModelRenderer(sky);

    //    AddEntityComponent(scene->group->bases, Base, &base, id);
    //    AddEntityComponent(scene->group->modelRenderers, ModelRenderer, &renderer, id);
    //}

    // GARDEN LEVEL: TODO refactor

    EntityID id = AddEntity(scene->group);

    Base base = CreateDefaultBase();
    ModelRenderer renderer = CreateModelRendererFromFile("levels/garden/garden_start.obj",&base);

    AddEntityComponent(scene->group->bases, Base, &base, id);
    AddEntityComponent(scene->group->modelRenderers, ModelRenderer, &renderer, id);

    return scene;
}

void scene_update_and_render(Scene* scene, float delta)
{
    ClearBackground(scene->env.skyColor);

    // move skybox around
    Base *skyBase = GetArrayItem(scene->group->bases,0,Base);
    //skyBase->pos = scene->camera->position;

    UpdateGroup(scene->group,delta);
    DrawGroup(scene->group);

    if (scene->editorVisible){
        editor_update_and_draw(scene->editor,delta);
        if (DoDrawGrid) {
			DrawGrid(1000,1);
        }
    }
}

void scene_update_and_render_gui(Scene* scene, float delta)
{
    if (scene->editorVisible){
        scene->editorVisible = editor_update_and_draw_gui(scene->editor);
    }
    if (IsKeyPressed(KEY_F3)){
        scene->editorVisible = !scene->editorVisible;
    }
}

void scene_dispose(Scene *scene)
{
    MemFree(scene);
}

