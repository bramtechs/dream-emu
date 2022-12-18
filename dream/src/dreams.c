#include "dreams.h"

Scene* dream_init_hub(){
    Scene* scene = scene_init();
    scene->env.skyColor = WHITE;

    EntityID id = AddEntity(scene->group);
    Base base = CreateBase(Vector3Zero(), RAYWHITE);

    ModelRenderer renderer = CreateModelRendererFromFile("levels/hub/hub.obj",&base);

    AddEntityComponent(scene->group->bases, Base, &base, id);
    AddEntityComponent(scene->group->modelRenderers, ModelRenderer, &renderer, id);

    return scene;
}

// garden? more like an empty wasteland for now
Scene* dream_init_garden(){
    Scene* scene = scene_init();

    EntityID id = AddEntity(scene->group);

    Base base = CreateDefaultBase();
    ModelRenderer renderer = CreateModelRendererFromFile("levels/garden/garden_start.obj",&base);

    AddEntityComponent(scene->group->bases, Base, &base, id);
    AddEntityComponent(scene->group->modelRenderers, ModelRenderer, &renderer, id);

    return scene;
}
