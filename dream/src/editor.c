#include "editor.h"

static Scene* ActiveScene = NULL;

static EditorMode Modes[EDITOR_MODE_COUNT] = {
    {
        EDITOR_NORMAL,
        "Normal Mode",
        KEY_ESCAPE,
        { 255, 255, 255, 255 },
    },
    {
        EDITOR_SPAWN,
        "Spawn Mode",
        KEY_I,
        { 0, 228, 48, 255 },
    },
    {
        EDITOR_MOVE,
        "Move Mode",
        KEY_G,
        { 230, 41, 55, 255 },
    },
    {
        EDITOR_ROTATE,
        "Rotation Mode",
        KEY_R,
        { 255, 109, 194, 255 },
    },
    {
        EDITOR_SCALE,
        "Scale Mode",
        KEY_S,
        { 200, 122, 255, 255 },
    },
};

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

    FilePathList models = IndexModels("");
    editor->modelCount = models.count;
    for (int i = 0; i < editor->modelCount; i++){
        const char* path = models.paths[i];
        editor->models[i] = LoadModel(path);
    }

    return editor;
}

void editor_print_transform(Editor* e, Base* base){
    Color color = Modes[e->mode].color;
    Vector3 center = base->center;
    DrawText(TextFormat("center\nx %f\ny %f\nz %f",center.x,center.y,center.z), 150, 550, 16, color);
}

void editor_dispose(Editor* editor){
   M_MemFree(editor);
}

void editor_move_gui(Editor* e, Base* base){
    editor_print_transform(e, base);
}

void editor_move(Editor* e, Base* base){

    Vector2 delta = Vector2Scale(GetMouseDelta(),0.5f);
    if (IsKeyDown(KEY_F)){
        SetBaseCenter(base, ActiveScene->player.feet);
    }

    if (IsKeyDown(KEY_ENTER)){
        Ray ray = { 0 };
        ray.position = Vector3Add(base->center, (Vector3) { 0.f, -0.01f, 0.f });
        ray.direction = (Vector3) {0,-1,0};

        RayCollision col = GetRayCollisionGroup(ActiveScene->group, ray);
        if (col.hit) {
            SetBaseCenter(base,col.point);
        }
    }

    if (IsKeyDown(KEY_X)){
        TranslateBaseX(base,delta.x);
    }
    if (IsKeyDown(KEY_Y)){
        TranslateBaseY(base,-delta.y);
    }
    if (IsKeyDown(KEY_Z)){
        TranslateBaseZ(base,delta.x);
    }
    if (IsKeyDown(KEY_ZERO)){
        ResetBaseTranslation(base);
    }
    if (IsKeyDown(KEY_TAB)){
        Vector3 pos = {
            floorf(base->center.x),
            floorf(base->center.y),
            floorf(base->center.z),
        };
        SetBaseCenter(base,pos);
    }

}

void editor_spawner_gui(Editor* e){
    Camera cam = { 0 };
    cam.position = (Vector3){ 0, 0, -10 };
    cam.up = (Vector3){ 0, 1, 1 };
    cam.fovy = 45;
    cam.projection = CAMERA_PERSPECTIVE;

    int SIZE = 256;
    if (((int)e->elapsedTime) % 2 == 1){
        DrawText(TextFormat("SPAWN MENU (%d)",e->selectedModel), 650, 250, 36, YELLOW);
        DrawRectangleLines(GetScreenWidth()/2-SIZE/2,GetScreenHeight()/2-SIZE/2,SIZE,SIZE,RED);
    }

    BeginMode3D(cam);

    // draw catalog preview
    Vector3 pos = { -e->modelCount/2, 0, 0 };
    pos.x = e->selectedModel;
    for (int i = 0; i < e->modelCount; i++){
        Model model = e->models[i];
        BoundingBox box = GetModelBoundingBox(model);
        float diameter = Vector3Length(Vector3Subtract(box.max,box.min));
        float scale = 1 / diameter;
        DrawModelEx(model, pos, (Vector3) { 1,0.5f,0.3f } , (i*10.f)+e->elapsedTime*40.f, (Vector3) { scale, scale, scale }, WHITE);

        pos.x--;
    }

    if (IsKeyPressed(KEY_LEFT) && e->selectedModel > 0) {
        e->selectedModel--;
    }

    if (IsKeyPressed(KEY_RIGHT) && e->selectedModel < e->modelCount-1) {
        e->selectedModel++;
    }

    if (IsKeyPressed(KEY_ENTER)){
        Model model = e->models[e->selectedModel];

        // spawn new entity
        EntityID id = AddEntity(ActiveScene->group);
        Base base = CreateBase(id, Vector3Zero(), RAYWHITE);

        ModelRenderer renderer = CreateModelRenderer(id,model,&base);

        AddEntityComponent(ActiveScene->group, COMP_BASE, &base, sizeof(Base), id);
        AddEntityComponent(ActiveScene->group, COMP_MODEL_RENDERER, &renderer, sizeof(ModelRenderer), id);

        e->mode = EDITOR_MOVE;
        e->subject = id;
    }

    e->elapsedTime += GetFrameTime();

    EndMode3D();
}

void editor_update_and_draw(Editor* e, float delta)
{
    assert(e);

    if (e->prevFreecamMode != Settings.freeCam){
        e->prevFreecamMode = Settings.freeCam;
        SetCameraMode(ActiveScene->player.camera,Settings.freeCam ? CAMERA_FIRST_PERSON : CAMERA_FREE);
    }

    // highlight selected
    Base* subjectBase = GetEntityComponent(ActiveScene->group, e->subject, COMP_BASE);
    DrawBoundingBox(subjectBase->bounds, Modes[e->mode].color);

    // change selected subject on clicking it
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
        Base* base = NULL;
        if (GetMousePickedBase(ActiveScene->group,ActiveScene->player.camera,&base)){
            EntityID id = base->id;

            e->subject = id;
            DEBUG("Changed editor subject to %d",id);
        }
    }

    // draw spawnpoint
    DrawSphere(ActiveScene->spawnPoint, 0.4f, ORANGE);
}

bool editor_update_and_draw_gui(Editor* e)
{
    assert(e);
    assert(ActiveScene);

    int WIN_X = 10;
    int WIN_Y = 10;
    int WIN_W = 300;
    int WIN_H = 700;

    LayoutY = WIN_Y + 40;

    EditorMode curMode = Modes[e->mode];

    Rectangle rect = {WIN_X, WIN_Y, WIN_W, WIN_H};
    bool visible = !GuiWindowBox(rect, curMode.name);

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

    // get current selected base
    Base* subjectBase = GetEntityComponent(ActiveScene->group, e->subject, COMP_BASE);

    // vomit out log
    DrawLog(350,80,24);

    DrawText(curMode.name,350,HEIGHT-50,36,curMode.color);
    
    // check keys
    for (int i = 0; i < EDITOR_MODE_COUNT; i++){
        if (IsKeyPressed(Modes[i].key)){
            e->mode = i;
            break;
        }
    }

    // set spawnpoint
    if (IsKeyDown(KEY_LEFT_CONTROL))
    {
        if (IsKeyPressed(KEY_HOME)){
            Vector3 spawn = ActiveScene->spawnPoint = ActiveScene->player.feet;
            INFO("Changed spawnpoint to %f, %f, %f!", spawn.x, spawn.y, spawn.z);
        }
        if (IsKeyPressed(KEY_S)){
            SaveEntityGroup(ActiveScene->group,"levels/savedmap001");
            INFO("Saved scene!");
        }
    }

    switch (e->mode){
        case EDITOR_NORMAL:
            break;
        case EDITOR_MOVE:
            editor_move(e,subjectBase);
            editor_move_gui(e,subjectBase);
            break;
        case EDITOR_ROTATE:
            break;
        case EDITOR_SCALE:
            break;
        case EDITOR_SPAWN:
            editor_spawner_gui(e);
            break;

        default:
            assert(false);
    }

    return visible;
}
