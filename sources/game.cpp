struct LevelData {
    int width;
    int height;
    int *tiles;
};

struct Environment {
    Color skyColor;
};

struct GameLevel {
    Camera camera;
    bool isFlying;

    Environment environment;
    LevelData data;

    ~GameLevel();
};

static GameLevel *CurrentLevel = nullptr;

GameLevel* game_level_load(){
    if (CurrentLevel == nullptr){
        CurrentLevel = new GameLevel();
    }

    // setup camera
    CurrentLevel->camera = {{0.2f, 0.4f, 0.2f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 45.0f, 0};
    SetCameraMode(CurrentLevel->camera, CAMERA_FIRST_PERSON);     // Set camera mode

    return CurrentLevel;
}

void game_update_and_render(){

    UpdateCamera(&CurrentLevel->camera);

    if (IsKeyPressed(KEY_F3)){
        bool *flying = &CurrentLevel->isFlying;
        *flying = !*flying;
        if (*flying){
            SetCameraMode(CurrentLevel->camera, CAMERA_FREE);     // Set camera mode
        }else{
            SetCameraMode(CurrentLevel->camera, CAMERA_FIRST_PERSON);     // Set camera mode
        }
    }

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    int width = CurrentLevel->data.width;
    int height = CurrentLevel->data.height;

    ClearBackground(CurrentLevel->environment.skyColor);

    BeginMode3D(CurrentLevel->camera);

    DrawCube({0,0,0},1,1,1,YELLOW);

    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int i = y * width + x;
            int tileIndex = CurrentLevel->data.tiles[i];
            tile_draw(x,y,tileIndex);
        }
    }

    EndMode3D();

    DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}

GameLevel::~GameLevel()
{
    free(data.tiles);
}

void game_dispose(){
    delete CurrentLevel;
}