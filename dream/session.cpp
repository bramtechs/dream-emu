struct Lamp {
    Vector3 pos;
    Color color;
    float brightness;
    bool disabled;
};

struct Billboard {
    int id;
    Vector3 pos;
};

struct Block {
    int id;
    Vector3 pos;
    bool isBlock;
};

struct Environment {
    Color skyColor;
    Color sunColor;
    float fogDensity;
    Vector3 sunDirection;
};

struct GameSession {
    Camera camera;
    bool isFlying;

    Environment environment;
    SmallMemoryArray<Light> lights;

    // should be one
    MemoryArray<Block> blocks;
    MemoryArray<Billboard> billboards;
    SmallMemoryArray<Lamp> lamps;
};

static GameSession *CurrentSession = nullptr;

void session_reload()
{
    // put level init in here

    level_load(Assets->levelLayouts.first(), CurrentFeed);
    TraceLog(LOG_INFO, "Reloaded session");

    CurrentSession->lights.clear();

    SetShaderValue(*Assets->fogShader, Assets->fogShaderDensityLoc, &CurrentFeed->environment.fogDensity,
                   SHADER_UNIFORM_FLOAT);

    // setup environment
    CurrentSession->environment.sunDirection = Vector3Normalize({1, -1, 1});
    CurrentSession->environment.fogDensity = 0.0f;
    CurrentSession->environment.skyColor = SKYBLUE;
    CurrentSession->environment.sunColor = WHITE;

    // add default lamps
    Light sun = CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), CurrentFeed->environment.sunDirection, WHITE,
                            *Assets->fogShader);
    CurrentSession->lights.push(sun);

    // import lamps
    for (int i = 0; i < CurrentFeed->lamps.count; i++)
    {
        Lamp *lamp = CurrentFeed->lamps.get(i);
        Light light = CreateLight(LIGHT_POINT, lamp->pos, lamp->pos, lamp->color, *Assets->fogShader);
        CurrentSession->lights.push(light);
    }
}

void session_reset()
{
    if (CurrentSession == nullptr)
    {
        CurrentSession = new GameSession();
    }

    // setup camera
    CurrentSession->camera = {{0.2f, 0.4f, 0.2f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 45.0f, 0};
    CurrentSession->isFlying = false;
    SetCameraMode(CurrentSession->camera, CAMERA_FIRST_PERSON);     // Set camera mode

    session_reload();
}

void session_update_lighting()
{
    Shader fogShader = *Assets->fogShader;
    // TODO fog shader cleanup
    SetShaderValue(fogShader, Assets->fogShaderDensityLoc, &CurrentFeed->environment.fogDensity,
                   SHADER_UNIFORM_FLOAT);

    // Update lighting
    for (int i = 0; i < NN(CurrentSession)->lights.count; i++)
    {
        Light light = *CurrentSession->lights.get(i);
        UpdateLightValues(*Assets->fogShader, light);
    }

    if (IsKeyPressed(KEY_PAGE_UP))
    {
        CurrentFeed->environment.fogDensity += GetFrameTime() * 0.2f;
    }
    if (IsKeyPressed(KEY_PAGE_DOWN))
    {
        CurrentFeed->environment.fogDensity -= GetFrameTime() * 0.2f;
    }

    // Update the light shader with the camera view position
    SetShaderValue(fogShader, fogShader.locs[SHADER_LOC_VECTOR_VIEW], &CurrentSession->camera.position.x,
                   SHADER_UNIFORM_VEC3);

}

void session_update_and_render(float delta)
{
    ClearBackground(NN(CurrentFeed)->environment.skyColor);

    session_update_lighting();

    auto blocks = &CurrentFeed->blocks;
    for (int i = 0; i < blocks->count; i++)
    {
        Block *block = blocks->get(i);
        drawing_draw(block);
    }

    auto billboards = &CurrentFeed->billboards;
    for (int i = 0; i < billboards->count; i++)
    {
        drawing_draw_billboard(billboards->get(i), Assets->treeTexture, &CurrentSession->camera);
    }

    gizmos_draw(CurrentFeed);
}
