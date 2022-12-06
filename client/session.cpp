struct GameSession {
    Camera camera;
    bool isFlying;

    SmallMemoryArray<Light> lights;
};

static GameSession *CurrentSession = nullptr;
static LevelFeed *CurrentFeed = nullptr;

void session_reload()
{
    // put level init in here

    level_load(Assets->levelLayouts.first(), CurrentFeed);
    TraceLog(LOG_INFO, "Reloaded session");

    CurrentSession->lights.clear();

    SetShaderValue(*Assets->fogShader, Assets->fogShaderDensityLoc, &CurrentFeed->environment.fogDensity,
                   SHADER_UNIFORM_FLOAT);

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

    gizmos_draw(CurrentFeed);
}
