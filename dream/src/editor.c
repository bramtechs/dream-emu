#include "editor.h"

#define MAX_MODELS 128

static Scene* ActiveScene = NULL;

static int LayoutY = 0;

static bool FPSMode = false;
static bool PrevFPSMode = false;

static Model Models[MAX_MODELS];
static int ModelCount = 10;
static int SelectedModel = 0;

static float ElapsedTime;

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

    FilePathList models = IndexModels("");
    for (int i = 0; i < ModelCount; i++){
        const char* path = models.paths[i];
        Models[i] = LoadModel(path);
    }

    return editor;
}

void editor_dispose(Editor* editor){
   M_MemFree(editor);
}

void editor_update_and_draw(Editor* editor, float delta)
{
    assert(editor);

    if (PrevFPSMode != FPSMode){
        PrevFPSMode = FPSMode;
        SetCameraMode(ActiveScene->player.camera,FPSMode ? CAMERA_FIRST_PERSON : CAMERA_FREE);
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

    Settings.unlockFrameRate = GuiCheckBox(LAYOUT(20, 30, 30), "Unlock framerate (not recommended)", Settings.unlockFrameRate);
    Settings.drawOutlines = GuiCheckBox(LAYOUT(20, 30, 30), "Draw outlines", Settings.drawOutlines);
    Settings.drawGrid = GuiCheckBox(LAYOUT(20, 30, 30), "Draw grid", Settings.drawGrid);
    Settings.freeCam = GuiCheckBox(LAYOUT(20, 30, 30), "Unlock camera", Settings.freeCam);

    Camera cam = { 0 };
    cam.position = (Vector3){ 0, 0, -10 };
    cam.up = (Vector3){ 0, 1, 1 };
    cam.fovy = 45;
    cam.projection = CAMERA_PERSPECTIVE;

    // vomit out log
    DrawLog(700,50,24);

    int SIZE = 256;
	if (((int)ElapsedTime) % 2 == 1){
		DrawText(TextFormat("SPAWN MENU (%d)",SelectedModel), 650, 250, 36, YELLOW);
		DrawRectangleLines(GetScreenWidth()/2-SIZE/2,GetScreenHeight()/2-SIZE/2,SIZE,SIZE,RED);
	}

    BeginMode3D(cam);

    // draw catalog preview
    Vector3 pos = { -ModelCount/2, 0, 0 };
    pos.x = SelectedModel;
    for (int i = 0; i < ModelCount; i++){
        Model model = Models[i];
        BoundingBox box = GetModelBoundingBox(model);
        float diameter = Vector3Length(Vector3Subtract(box.max,box.min));
        float scale = 1 / diameter;
        DrawModelEx(model, pos, (Vector3) { 1,0.5f,0.3f } , (i*10.f)+ElapsedTime*40.f, (Vector3) { scale, scale, scale }, WHITE);

        pos.x--;
    }

    if (IsKeyPressed(KEY_LEFT)) {
        SelectedModel--;
    }

    if (IsKeyPressed(KEY_RIGHT)) {
        SelectedModel++;
    }

    SelectedModel = Clamp(SelectedModel,0, ModelCount-1);

    ElapsedTime += GetFrameTime();

    EndMode3D();

    return visible;
}
