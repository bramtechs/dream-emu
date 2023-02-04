#include "dreams.h"

Model scene_gen_skybox_model(const char* skybox) {
    // TODO put skybox in struct, and dispose properly
    Mesh mesh = GenMeshSphere(-150, 10, 10);
    Model model = LoadModelFromMesh(mesh);
    Texture texture = RequestTexture(skybox);

    Image img = LoadImageFromTexture(texture);
    ImageRotateCCW(&img);

    Texture textureFlipped = LoadTextureFromImage(img);
    model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = textureFlipped;

    return model;
}

GardenDream::GardenDream() {
    EntityID id = group.AddEntity();

    Base base = Base();
    ModelRenderer renderer = ModelRenderer("mesh_garden_start.obj",&base); // TODO: WRONG!!!!!
    renderer.accurate = true;

    group.AddEntityComponent(COMP_BASE, id, base);
    group.AddEntityComponent(COMP_MODEL_RENDERER, id, renderer);
}

void GardenDream::update(float delta) {

}

HubDream::HubDream() {
    env.skyColor = WHITE;

    {
        // generate skybox 
        Model sky = scene_gen_skybox_model("spr_sky.png");
        EntityID id = group.AddEntity();

        auto base = Base();
        // ModelRenderer renderer = ModelRenderer(id, sky, &base); TODO create in blender

        // TODO
        //AddEntityComponent(scene->group, COMP_BASE, &base, sizeof(Base), id);
        // AddEntityComponent(scene->group, COMP_MODEL_RENDERER, &renderer, sizeof(ModelRenderer), id);
    }

    {
        EntityID id = group.AddEntity();
        Base base = Base(Vector3Zero(), RAYWHITE);

        ModelRenderer renderer = ModelRenderer("mesh_hub.obj",&base);

        group.AddEntityComponent(COMP_BASE, id, base);
        group.AddEntityComponent(COMP_MODEL_RENDERER, id, renderer);
    }
}

void HubDream::update(float delta)
{
    // move skybox around
    Base *skyBase = (Base*) group.GetEntityComponent(0,COMP_BASE);
    skyBase->SetCenter(player.camera.position);
}
