#include "magma.h"
#include "client.h"
#include "dreams.h"

#define RAYGUI_IMPLEMENTATION

bool LastLockFramerate = true;

int main()
{
	// Initialization
	//--------------------------------------------------------------------------------------
	InitMagmaWindow(WIDTH, HEIGHT, WIDTH * SCALE, HEIGHT * SCALE, "DREAM_EMU");
	SetWindowState(FLAG_WINDOW_MAXIMIZED);

	INFO("Launched at %s", GetWorkingDirectory());

	RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

	SetTargetFPS(60);

	SetTraceLogLevel(LOG_DEBUG);

	if (Assets::Init("assets") != NULL) {
		UserPrefs::Load();
		LoadMagmaSettings();

		//Scene* scene = dream_init_hub();
		auto scene = GardenDream();

		// SetWindowState(FLAG_WINDOW_MAXIMIZED);

		MainMenu menu = MainMenu({
			WIDTH,
			HEIGHT,
			{
				{
					"gui_splash.png",
					4.f,
				},
				{
					"gui_save_warning.png",
					4.f,
				},
			},
			"spr_sky.png",
			"Dream Emulator"
			}, Settings.skipIntro);

		//TestList();

		//    Shader shader = LoadShader(0, "../../../assets/shaders/gui/menu.fs");
		//    int shaderTime = GetShaderLocation(shader, "iTime");
		//    int shaderResolution = GetShaderLocation(shader, "iResolution");
		//
		//    Vector3 size = { WIDTH * 0.001, HEIGHT * 0.001, 1};
		//    SetShaderValue(shader, shaderResolution, &size, SHADER_UNIFORM_VEC3);

			// Main game loop
		while (!WindowShouldClose()) // Detect window close button or ESC key
		{
			if (LastLockFramerate != Settings.unlockFrameRate) {
				SetTargetFPS(Settings.unlockFrameRate ? 1000 : 60);
				LastLockFramerate = Settings.unlockFrameRate;
			}

			float delta = GetFrameTime();
			float time = GetTime();

			//        SetShaderValue(shader, shaderTime, &time, SHADER_UNIFORM_FLOAT);

			//        BeginShaderMode(shader);

			if (menu.UpdateAndDraw(delta)) {
				scene.update_and_render(delta);
			}

			//        EndShaderMode();
			//
		}

		UserPrefs::Save();

		SaveMagmaSettings();

		Assets::Dispose();

	}
	else {
		Image grid = GenImageChecked(WIDTH+64, WIDTH+64, 32, 32, PURPLE, DARKPURPLE);
		Texture gridTexture = LoadTextureFromImage(grid);
		UnloadImage(grid);
		float offset = 0.f;
		const char* text = "Could not find 'assets.mga'.\nPlease extract your download.";
		while (!WindowShouldClose()) {
			BeginMagmaDrawing();
				ClearBackground(BLACK);
				DrawTexture(gridTexture, -offset, -offset, WHITE);
				DrawRectangleGradientV(0, 0, WIDTH, HEIGHT+abs(sin(GetTime())*100), BLANK, PINK);
				offset += GetFrameTime() * 32.f;
				if (offset > 32){
					offset = 0;
				}
				Vector2 pos = Vector2Subtract({ WIDTH / 2,HEIGHT / 2 }, Vector2Scale(MeasureTextEx(GetFontDefault(), text, 28, 2), 0.5f));
				DrawTextEx(GetFontDefault(), text, pos, 28, 2, WHITE);
			EndMagmaDrawing();
			EndDrawing();
		}
		UnloadTexture(gridTexture);
	}
	CheckAllocations();

	CloseMagmaWindow();

	return 0;
}
