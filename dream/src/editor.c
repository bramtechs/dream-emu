#define UI_PADDING 40

#define RAYGUI_IMPLEMENTATION
#include "editor.h"

typedef struct {
    AssetList assetList;
} EditorInstance;

static EditorInstance* Editor = NULL;

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

    int win_x = 10; 
    int win_y = 10; 
    int width = 300;

    Rectangle rect = {win_x, win_y, width, 700};

    bool visible = !GuiWindowBox(rect, "Editor");

    rect.x += UI_PADDING;
    rect.y += UI_PADDING+20;
    rect.width = 150;
    rect.height = 150;

    scene->env.skyColor = GuiColorPicker(rect, "Sky color", scene->env.skyColor);

    rect.y += 150+UI_PADDING;
    rect.height = 20;

    char* fogStr = TextFormat("%f",scene->env.fogDistance);
    scene->env.fogDistance = GuiSlider(rect, "Fog", fogStr, scene->env.fogDistance, 0.f, 1.f);       // Slider control, returns selected value

   
    rect.y += 50;
    rect.height = 14;
    for (int i = 0; i < Editor->assetList.count; i++){
        GuiLabel(rect, Editor->assetList.names[i]);                                            // Label control, shows text
        rect.y += 20;
    }

    return visible;
}
