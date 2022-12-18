#include "editor.h"

static Scene* ActiveScene = NULL;

static int LayoutY = 0;

static bool FPSMode = false;
static bool PrevFPSMode = false;

static Rectangle LAYOUT(int x, int w, int h){

    Rectangle rect = {
        x,LayoutY,w,h
    };
    LayoutY += h + 10;

    return rect;
}

Editor* editor_init(SCENE* scene){
    Editor *editor = new(Editor);

    ActiveScene = (Scene*) scene;

    return editor;
}

void editor_dispose(Editor* editor){
   M_MemFree(editor);
}

// TODO split this into relevant files
// TODO this won't scale, make some kind of ECS system instead idk
// void editor_entity_polled(Entity entity, EntityGroup* group){
//     if (entity.components & COMP_BLOCK){
//         Base *base = &((Block*) entity.content)->base; // woah! how ugly
// 
//         RayCollision col = GetMouseRayCollisionBase(*base,*ActiveScene->camera);
//         if (col.hit && col.distance < 50) {
//             DrawCubeWiresV(base->pos, base->size, GREEN);
//         }
//     }
// }

void editor_update_and_draw(Editor* editor, float delta)
{
    assert(editor);

    if (PrevFPSMode != FPSMode){
        PrevFPSMode = FPSMode;
        SetCameraMode(*ActiveScene->camera,FPSMode ? CAMERA_FIRST_PERSON : CAMERA_FREE);
    }
}

bool editor_update_and_draw_gui(Editor* editor)
{
    assert(editor);
    assert(ActiveScene);

    int WIN_X = 10;
    int WIN_Y = 10;
    int WIN_W = 300;
    int WIN_H = 700;

    LayoutY = WIN_Y + 40;

    Rectangle rect = {WIN_X, WIN_Y, WIN_W, WIN_H};
    bool visible = !GuiWindowBox(rect, "Editor");

    ActiveScene->env.skyColor.r = GuiSlider(LAYOUT(40,100,20),"Red","255",ActiveScene->env.skyColor.r,0.f,255.f);
    ActiveScene->env.skyColor.g = GuiSlider(LAYOUT(40,100,20),"Green","255",ActiveScene->env.skyColor.g,0.f,255.f);
    ActiveScene->env.skyColor.b = GuiSlider(LAYOUT(40,100,20),"Blue","255",ActiveScene->env.skyColor.b,0.f,255.f);

    char* fogStr = TextFormat("%f",ActiveScene->env.fogDistance);
    ActiveScene->env.fogDistance = GuiSlider(LAYOUT(40,100,20), "Fog", fogStr, ActiveScene->env.fogDistance, 0.f, 1.f);       // Slider control, returns selected value

    GuiLabel(LAYOUT(20, WIN_W - 50, 50),"Hold middle mouse to move around,\nhold alt to look around.\nUse scrollwheel");

    LockFramerate = GuiCheckBox(LAYOUT(20, 30, 30), "Lock framerate (recommended)", LockFramerate);
    DrawOutlines = GuiCheckBox(LAYOUT(20, 30, 30), "Draw outlines", DrawOutlines);
    DoDrawGrid = GuiCheckBox(LAYOUT(20, 30, 30), "Draw grid", DoDrawGrid);
    FPSMode = GuiCheckBox(LAYOUT(20, 30, 30), "FPS mode", FPSMode);

    return visible;
}
