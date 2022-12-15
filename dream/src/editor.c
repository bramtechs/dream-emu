#include "editor.h"


static Scene* ActiveScene = NULL;

static int LayoutY = 0;

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
void editor_entity_polled(Entity entity, EntityGroup* group){
    if (entity.components & COMP_BLOCK){
        Base *base = &((Block*) entity.content)->base; // woah! how ugly

        RayCollision col = GetMouseRayCollisionBase(*base,*ActiveScene->camera);
        if (col.hit && col.distance < 50) {
            DrawCubeWiresV(base->pos, base->size, GREEN);
        }
    }
    if (entity.components & COMP_MASK){
        Mask* mask = (Mask*) entity.content;
        Base *base = &mask->base;

        Mesh* mesh = &mask->mesh;
        // assert(mesh->vertexCount == 0);

        // draw vertices
        for (int i = 0; i < mesh->vertexCount; i += 3){
            Vector3 pos = {
                    mesh->vertices[i+0],
                    mesh->vertices[i+1],
                    mesh->vertices[i+2],
            };
            Vector3 vertex = Vector3Add(pos,base->pos);
            DrawSphere(vertex,0.1f,RED);
        }

        RayCollision col = GetMouseRayCollisionBase(*base,*ActiveScene->camera);
        if (col.hit && col.distance < 50) {
            BoundingBox box = GetMeshBoundingBox(*mesh);
            DrawBoundingBox(box, GREEN);
        }
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

    return visible;
}
