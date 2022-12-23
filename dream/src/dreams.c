#include "dreams.h"

void dream_update_hub(Scene* scene, float delta){
    // move skybox around
    Base *skyBase = GetArrayItem(scene->group->bases,0,Base);
    skyBase->pos = scene->player.camera.position;
    skyBase->rotation.z += 2.0f * delta;
}

Scene* dream_init_hub(){
    Scene* scene = scene_init(dream_update_hub);
    scene->env.skyColor = WHITE;

    {
        // generate skybox 
        Model sky = scene_gen_skybox_model("sky/sky.png");
        EntityID id = AddEntity(scene->group);

        Base base = CreateDefaultBase();
        ModelRenderer renderer = CreateModelRenderer(sky, &base);

        AddEntityComponent(scene->group->bases, Base, &base, id);
        AddEntityComponent(scene->group->modelRenderers, ModelRenderer, &renderer, id);
    }

    {
        EntityID id = AddEntity(scene->group);
        Base base = CreateBase(Vector3Zero(), RAYWHITE);

        ModelRenderer renderer = CreateModelRendererFromFile("levels/hub/hub.obj",&base);

        AddEntityComponent(scene->group->bases, Base, &base, id);
        AddEntityComponent(scene->group->modelRenderers, ModelRenderer, &renderer, id);
    }

    return scene;
}

// garden? more like an empty wasteland for now
void dream_update_garden(Scene* scene, float delta){
}

Scene* dream_init_garden(){
    Scene* scene = scene_init(dream_update_garden);

    EntityID id = AddEntity(scene->group);

    Base base = CreateDefaultBase();
    ModelRenderer renderer = CreateModelRendererFromFile("levels/garden/garden_start.obj",&base);

    AddEntityComponent(scene->group->bases, Base, &base, id);
    AddEntityComponent(scene->group->modelRenderers, ModelRenderer, &renderer, id);

    return scene;
}
