#include "dreams.h"

void dream_update_hub(Scene* scene, float delta){
    // move skybox around
    Base *skyBase = GetEntityComponent(scene->group,0,COMP_BASE);
    SetBaseCenter(skyBase,scene->player.camera.position);
    // skyBase->rotation.z += 2.0f * delta;
}

Scene* dream_init_hub(){
    Scene* scene = scene_init(dream_update_hub);
    scene->env.skyColor = WHITE;

    {
        // generate skybox 
        Model sky = scene_gen_skybox_model("spr_sky.png");
        EntityID id = AddEntity(scene->group);

        Base base = CreateDefaultBase(id);
        // ModelRenderer renderer = CreateModelRenderer(id, sky, &base);

        // TODO
        //AddEntityComponent(scene->group, COMP_BASE, &base, sizeof(Base), id);
        // AddEntityComponent(scene->group, COMP_MODEL_RENDERER, &renderer, sizeof(ModelRenderer), id);
    }

    {
        EntityID id = AddEntity(scene->group);
        Base base = CreateBase(id, Vector3Zero(), RAYWHITE);

        ModelRenderer renderer = CreateModelRenderer(id,"mesh_hub.obj",&base);

        AddEntityComponent(scene->group, COMP_BASE, &base, sizeof(Base), id);
        AddEntityComponent(scene->group, COMP_MODEL_RENDERER, &renderer, sizeof(ModelRenderer), id);
    }

    return scene;
}

// garden? more like an empty wasteland for now
void dream_update_garden(Scene* scene, float delta){
}

Scene* dream_init_garden(){
    Scene* scene = scene_init(dream_update_garden);

    EntityID id = AddEntity(scene->group);

    Base base = CreateDefaultBase(id);
    ModelRenderer renderer = CreateModelRenderer(id,"mesh_garden_start.obj",&base);
    renderer.accurate = true;

	AddEntityComponent(scene->group, COMP_BASE, &base, sizeof(Base));
	AddEntityComponent(scene->group, COMP_MODEL_RENDERER, &renderer, sizeof(ModelRenderer));

    return scene;
}
