#include "magma.h"

#define WIDTH 640
#define HEIGHT 480
#define SCALE 1.4

#define TEST_MODE false

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

int launch(std::vector<std::string> args){

    std::string filePath;
    bool testMode = false;

    for (auto& arg : args){
        if (arg == "--test") {
            testMode = true;
        }else if (filePath.empty() &&
                TextIsEqual(GetFileExtension(arg.c_str()),".mga")) {
            filePath = arg;
        }
    }
    if (testMode && filePath.empty()){
        std::cout << "File path is required to perform a test!" << std::endl;
        return -1;
    }

    SetTraceLogCallback(MagmaLogger);
    //SetTraceLogAssertLevel(LOG_ERROR);
    SetTraceLogOpenLevel(LOG_ERROR);
    SetTraceLogLevel(LOG_DEBUG);

    InitMagmaWindow(WIDTH, HEIGHT, "Magma Explorer");

    INFO("Launched at %s", GetWorkingDirectory());

    State curState = INTRO;
    bool showError = false;

    Explorer* explorer = NULL;
    int code = 0;

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
                    DrawCheckeredBackground(32,"Drag and drop an .mga asset pack\ninto this window to explore!",
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
                    explorer = new Explorer(filePath.c_str(),testMode);
                    if (!explorer->succeeded) { // abandon failed session
                        filePath.clear();
                        delete explorer;
                        explorer = NULL;
                        curState = INTRO;
                        showError = true;
                        if (testMode) { // test failed if this happened
                            CloseWindow();
                            code = -1;
                        }
                    }
                }
                else {
                    explorer->UpdateAndRender(delta);
                }
            }
                break;
            default:
                break;
        }

        UpdateAndDrawLog();
        EndDrawing();
    }

    CloseWindow();

    return code;
}

int main(int argc, char** argv)
{
    std::vector<std::string> args;

    if (TEST_MODE){
        args.push_back("X:\\temple\\raw_assets");
        args.push_back("X:\\temple\\assets.mga");
    }
    else {
        // parse args
        for (int i = 0; i < argc; i++){
            args.push_back(argv[i]);
        }
    }

    launch(args);
}
