struct GameSession {
    Camera camera;
    bool isFlying;

    SmallMemoryArray<Light> lights;
};

static GameSession *CurrentSession = nullptr;
static LevelFeed *CurrentFeed = nullptr;

void session_reload(){
    level_load(Assets->levelLayouts.first(),CurrentFeed);
    TraceLog(LOG_INFO,"Reloaded session");

    // put level init in here
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