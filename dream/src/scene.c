#define MAX_ENTITIES 1024
#define RAYGUI_IMPLEMENTATION
#include "scene.h"

static int LayoutY = 0;

static Scene* ActiveScene = NULL;

inline Environment environment_default(){
    Environment env = { 0 };
    env.skyColor = SKYBLUE;
    env.fogColor = GRAY;
    env.fogDistance = 0.2f;
    return env;
}

Scene* scene_init(Assets* assets, Camera* camera)
{
    Scene *scene = MemAlloc(sizeof(Scene));
    scene->env = environment_default();
    scene->group = CreateEntityGroup(camera);
    scene->camera = camera;

    scene->editor = editor_init(assets);
    scene->editorVisible = true;
    ActiveScene = scene;

    for (int i = 0; i < 10000; i++) {
        entity_block_create_rainbow(scene->group,Vector3Zero(),camera);
    }

    return scene;
}

void scene_update_and_render(Scene* scene, float delta)
{
    ClearBackground(scene->env.skyColor);

    UpdateGroup(scene->group,delta);
    DrawGroup(scene->group);

    if (scene->editorVisible){
        editor_update_and_draw(scene->editor,delta);
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
    DrawText("Press F3 for editor",10,HEIGHT*SCALE-20,16,PURPLE);
}

void scene_dispose(Scene *scene)
{
    MemFree(scene);
}

static Rectangle LAYOUT(int x, int w, int h){

    Rectangle rect = {
        x,LayoutY,w,h
    };
    LayoutY += h + 20;

    return rect;
}

Editor* editor_init(Assets* assets){
    Editor *editor = new(Editor);
    editor->assetList = GetLoadedAssetList(assets);
    return editor;
}

void editor_dispose(Editor* editor){
   UnloadAssetList(editor->assetList);
   M_MemFree(editor);
}

void editor_entity_polled(void *ptr, EntityGroup* group){
    Base *base = (Base*)ptr;
    RayCollision col = GetMouseRayCollisionBase(*base,*ActiveScene->camera);

    if (col.hit && col.distance < 50) {
        DrawCubeWiresV(base->pos, base->size, GREEN);
    }
}

void editor_update_and_draw(Editor* editor, float delta)
{
    assert(editor);

    // poll all the entities
    PollEntities(ActiveScene->group, COMP_BASE, editor_entity_polled);
}

bool editor_update_and_draw_gui(Editor* editor)
{
    assert(editor);

    int WIN_X = 10;
    int WIN_Y = 10;
    int WIN_W = 300;
    int WIN_H = 700;

    LayoutY = WIN_Y + 40;

    Rectangle rect = {WIN_X, WIN_Y, WIN_W, WIN_H};
    bool visible = !GuiWindowBox(rect, "Editor");

    ActiveScene->env.skyColor = GuiColorPicker(LAYOUT(20,250,200), "Sky color", ActiveScene->env.skyColor);

    char* fogStr = TextFormat("%f",ActiveScene->env.fogDistance);
    ActiveScene->env.fogDistance = GuiSlider(LAYOUT(40,100,20), "Fog", fogStr, ActiveScene->env.fogDistance, 0.f, 1.f);       // Slider control, returns selected value

    for (int i = 0; i < editor->assetList.count; i++){
        GuiLabel(LAYOUT(20,100,10), editor->assetList.names[i]);                                            // Label control, shows text
    }

    GuiLabel(LAYOUT(20, WIN_W - 50, 50),"Hold middle mouse to move around,\nhold alt to look around.\nUse scrollwheel\nClose this window for free FPS.");

    return visible;
}
