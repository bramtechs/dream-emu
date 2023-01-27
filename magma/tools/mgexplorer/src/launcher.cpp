#include "magma.h"

#define WIDTH 640
#define HEIGHT 480
#define SCALE 1.2

#include "explorer.h"

enum State {
    INTRO,
    EXPLORER
};

bool validate_dropped_file(std::string *filePath) {
    FilePathList list = LoadDroppedFiles();
    bool valid = false;
    if (list.count > 0) {
        const char* path = list.paths[0];
        const char* ext = GetFileExtension(path);
        if (!TextIsEqual(ext, ".mga")) {
            goto end;
        }
        if (!FileExists(path)) {
            goto end;
        }
        *filePath = path;
        valid = true;
    }

    end: 
        UnloadDroppedFiles(list);
        return valid;
}

int main()
{
    SetTraceLogCallback(MagmaLogger);
    SetTraceLogLevel(LOG_DEBUG);
    assert(ChangeDirectory("X:\\temple"));

    InitMagmaWindow(WIDTH, HEIGHT, "Magma Explorer");

    INFO("Launched at %s", GetWorkingDirectory());

    State curState = INTRO;
    bool showError = false;

    Explorer* explorer = NULL;
    std::string filePath;

    // TODO bypass for testing
    filePath = "X:\\assets.mga";

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        float delta = GetFrameTime();
        float time = GetTime();

        BeginDrawing();

        ClearBackground(SKYBLUE);

        switch (curState) {
            case INTRO:
            {
                if (IsFileDropped()) {
                    if (validate_dropped_file(&filePath)) {
                        const char* text = TextFormat("Loading %s...",filePath);
                        DrawCheckeredBackground(32,text,
                                                GREEN,DARKGREEN,LIME);
                    }
                    else {
                        showError = true;
                    }
                }
                else {
                    DrawCheckeredBackground(32,"Drag and drop a .mga asset pack\ninto this window to explore!",
                                            GREEN,DARKGREEN,LIME);
                }

                if (showError) {
                        DrawCheckeredBackground(32,"Invalid file!",
                                                RED,ORANGE,RED);
                        if (IsMouseButtonPressed(0)) {
                            showError = false;
                        }
                }

                if (!filePath.empty()) {
                    curState = EXPLORER;
                }
            }
                break;
            case EXPLORER:
            {
                if (explorer == NULL){
                    explorer = new Explorer(filePath.c_str());
                }

                explorer->UpdateAndRender(delta);
            }
                break;
            default:
                break;
        }

        UpdateAndDrawLog();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
