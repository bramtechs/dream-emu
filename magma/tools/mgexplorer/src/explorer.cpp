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

	InitMagmaWindow(WIDTH, HEIGHT, WIDTH * SCALE, HEIGHT * SCALE, "Magma Explorer");

	INFO("Launched at %s", GetWorkingDirectory());

	State curState = INTRO;
	bool showError = false;

	std::string filePath;

	while (!WindowShouldClose()) // Detect window close button or ESC key
	{
		float delta = GetFrameTime();
		float time = GetTime();

		BeginMagmaDrawing();

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


			}
				break;
			default:
				break;
		}

		EndMagmaDrawing();
		UpdateAndDrawLog();
		EndDrawing();

	}

	CloseMagmaWindow();

	return 0;
}
