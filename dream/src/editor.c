#define RAYGUI_IMPLEMENTATION
#include "editor.h"

typedef struct {
    AssetList assetList;
} EditorInstance;

static EditorInstance* Editor = NULL;

static int LayoutY = 0;

static Rectangle LAYOUT(int x, int w, int h){

    Rectangle rect = {
        x,LayoutY,w,h
    };
    LayoutY += h + 20;

    return rect;
}

void editor_init(Assets* assets){
    assert(!Editor);

    Editor = new(EditorInstance);
    Editor->assetList = GetLoadedAssetList(assets);
}

void editor_dispose(){
   UnloadAssetList(Editor->assetList);
   M_MemFree(Editor);
}

bool editor_update_and_draw(Scene* scene)
{
    assert(Editor);

    int WIN_X = 10;
    int WIN_Y = 10;
    int WIN_W = 300;
    int WIN_H = 700;

    LayoutY = WIN_Y + 40;

    Rectangle rect = {WIN_X, WIN_Y, WIN_W, WIN_H};
    bool visible = !GuiWindowBox(rect, "Editor");

    scene->env.skyColor = GuiColorPicker(LAYOUT(20,250,200), "Sky color", scene->env.skyColor);

    char* fogStr = TextFormat("%f",scene->env.fogDistance);
    scene->env.fogDistance = GuiSlider(LAYOUT(40,100,20), "Fog", fogStr, scene->env.fogDistance, 0.f, 1.f);       // Slider control, returns selected value

    for (int i = 0; i < Editor->assetList.count; i++){
        GuiLabel(LAYOUT(20,100,10), Editor->assetList.names[i]);                                            // Label control, shows text
    }

    return visible;
}
