#include "magma.h"
#include "client.hpp"

#define COLORS_PER_PALETTE 256 
struct Palette {
	const char name[64];
	int colors[COLORS_PER_PALETTE * 3];

	Color GetColor(int index);
	Color GetIndexColor(int index);
	void MapImage(Image img);
	void DrawPreview(Rectangle region);
	int MapColorLoosely(Color color);
	int MapColor(Color color);
};

Color Palette::GetIndexColor(int index) {
	index = Clamp(index, 0, 255);
	Color result = {
		index,
		0,
		0,
		255
	};
	return result;
}

Color Palette::GetColor(int index) {
	if (index < 0 || index >= COLORS_PER_PALETTE) {
		WARN("Could not get color indexed %d from palette %s", index, name);
		return {255,0,255,255};
	}
	Color result = {
		colors[index * 3 + 0],
		colors[index * 3 + 1],
		colors[index * 3 + 2],
		255
	};
	return result;
}

int Palette::MapColor(Color color) {
	for (int i = 0; i < COLORS_PER_PALETTE; i++) {
		if (colors[i * 3 + 0] == color.r &&
			colors[i * 3 + 1] == color.g &&
			colors[i * 3 + 2] == color.b) {
			return i;
		}
	}
	WARN("Could not map color into palette %s", name);
	return 0;
}

int Palette::MapColorLoosely(Color color) {
	Vector3 inCol = {
		color.r,
		color.g,
		color.b
	};

	int closestID = 0;
	float closestDiff = FLT_MAX;
	for (int i = 0; i < COLORS_PER_PALETTE; i++) {
		Vector3 palCol{
			colors[i * 3 + 0],
			colors[i * 3 + 1],
			colors[i * 3 + 2]
		};
		float diff = Vector3DistanceSqr(palCol, inCol);
		if (diff < closestDiff) {
			closestDiff = diff;
			closestID = i;
		}
	}
	return closestID;
}

void Palette::MapImage(Image img) {
	for (int y = 0; y < img.height; y++) {
		for (int x = 0; x < img.width; x++) {
			Color origColor = GetImageColor(img, x, y);
			int i = MapColorLoosely(origColor);
			Color newColor = GetIndexColor(i);
			ImageDrawPixel(&img, x,y, newColor);
		}
	}
}

void Palette::DrawPreview(Rectangle region) {
	const int DEV = 16;

	int size = region.width / DEV;
	int i = 0;
	for (int y = 0; y < DEV; y++) {
		for (int x = 0; x < DEV; x++) {
			Color col = GetColor(i);
			DrawRectangle(region.x+x*size, region.y+y*size, size, size, col);
			i++;
		}
	}
}

struct TempleGame {
	Texture blockTexture;

	Palette* palette;
	Shader shader;

	TempleGame() {
		Image blockImage = Assets::RequestImage("spr_block_2");

		// force texture into palette
		this->palette = new Palette{
			"default",
			{
				255, 0, 255,
				4, 12, 6,
				17, 35, 24,
				30, 58, 41,
				48, 93, 66,
				77, 128, 97,
				137, 162, 87,
				190, 220, 127,
				238, 255, 204,
			}
		};

		palette->MapImage(blockImage);

		blockTexture = LoadTextureFromImage(blockImage);
		UnloadImage(blockImage);

		INFO("LOADING SHADER");
		shader = LoadShader(0, "raw_assets/frag_palette_switch.fs");

		int paletteLoc = GetShaderLocation(shader, "palette");
		SetShaderValueV(shader, paletteLoc, palette->colors, SHADER_UNIFORM_IVEC3, COLORS_PER_PALETTE);
	}

	void update_and_render(float delta) {
		BeginMagmaDrawing();

		ClearBackground(SKYBLUE);

		BeginShaderMode(shader);

		DrawTexture(blockTexture, 20, 20, WHITE);

		EndShaderMode();

		EndMagmaDrawing();
		UpdateAndDrawLog();
		palette->DrawPreview({ (float)GetScreenWidth() - 150,(float)GetScreenHeight() - 150,150,150});
		EndDrawing();
	}
};

int main()
{
	SetTraceLogCallback(MagmaLogger);
	SetTraceLogLevel(LOG_DEBUG);
	assert(ChangeDirectory("X:\\temple"));

	InitMagmaWindow(WIDTH, HEIGHT, WIDTH * SCALE, HEIGHT * SCALE, "Temple Mayhem");
	SetWindowState(FLAG_WINDOW_MAXIMIZED);

	INFO("Launched at %s", GetWorkingDirectory());

	RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

	SetTargetFPS(60);

	if (Assets::Init("assets.mga") != NULL) {
		LoadMagmaSettings();

		MainMenu menu = MainMenu({
			WIDTH,
			HEIGHT,
			{
				{
					"gui_splash",
					4.f,
				},
				{
					"gui_save_warning",
					4.f,
				},
			},
			"spr_sky",
			"Dream Emulator"
			}, true);

		TempleGame game;

		while (!WindowShouldClose()) // Detect window close button or ESC key
		{
			float delta = GetFrameTime();
			float time = GetTime();

			if (menu.UpdateAndDraw(delta)) {
				// draw scene here
				game.update_and_render(delta);
			}
		}

		SaveMagmaSettings();

		Assets::Dispose();

	}
	else {
		// render fail screen until game close
		Assets::EnterFailScreen(WIDTH, HEIGHT);
	}
	CheckAllocations();

	CloseMagmaWindow();

	return 0;
}
