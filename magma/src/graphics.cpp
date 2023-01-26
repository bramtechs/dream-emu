#include "magma.h"

#define Win Window
MagmaWindow Window = {0};

void InitMagmaWindow(int gameWidth, int gameHeight, int winWidth, int winHeight, const char* title) {
	Win.gameSize = { (float)gameWidth, (float)gameHeight };
	Win.winSize = { (float)winWidth, (float)winHeight };

	assert(gameWidth > 0 && gameHeight > 0 &&
		winWidth > 0 && winHeight > 0);

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(gameWidth, gameHeight, title);
	SetWindowMinSize(winWidth / 2, winHeight / 2);
	SetWindowPosition((GetMonitorWidth(0) - winWidth) / 2, (GetMonitorHeight(0) - winHeight) / 2);
	SetWindowSize(winWidth, winHeight);

	SetExitKey(KEY_DELETE);

	// Render texture initialization, used to hold the rendering result so we can easily resize it
	Win.renderTarget = LoadRenderTexture(gameWidth, gameHeight);
	SetTextureFilter(Win.renderTarget.texture, TEXTURE_FILTER_POINT);  // Texture scale filter to use
}

void BeginMagmaDrawing() {
	Win.scale = MIN((float)GetScreenWidth() / Win.gameSize.x,
		(float)GetScreenHeight() / Win.gameSize.y);

	// Update virtual mouse (clamped mouse value behind game screen)
	Vector2 mouse = GetMousePosition();
	Vector2 virtualMouse = { 0 };
	Win.scaledMouse.x = (mouse.x - (GetScreenWidth() - (Win.gameSize.x * Win.scale)) * 0.5f) / Win.scale;
	Win.scaledMouse.y = (mouse.y - (GetScreenHeight() - (Win.gameSize.y * Win.scale)) * 0.5f) / Win.scale;
	virtualMouse = Vector2Clamp(virtualMouse, { 0, 0 }, { (float)Win.gameSize.x, (float)Win.gameSize.y });

	BeginTextureMode(Win.renderTarget);
	ClearBackground(BLACK);

	// ....
}

void EndMagmaDrawing() {

	// ....

	EndTextureMode();

	BeginDrawing();
	ClearBackground(BLACK);     // Clear screen background

	Vector2 topLeft = GetWindowTopLeft();

	// Draw render texture to screen, properly scaled
	DrawTexturePro(Win.renderTarget.texture, { 0.0f, 0.0f, (float)Win.renderTarget.texture.width, (float)-Win.renderTarget.texture.height },
		{ topLeft.x, topLeft.y, (float)Win.gameSize.x * Win.scale, (float)Win.gameSize.y * Win.scale }, { 0, 0 }, 0.0f, WHITE);
}

void CloseMagmaWindow() {
	UnloadRenderTexture(Win.renderTarget);
	CloseWindow();
}

Vector2 GetWindowTopLeft() {
	return {
		(GetScreenWidth() - ((float)Win.gameSize.x * Win.scale)) * 0.5f,
		(GetScreenHeight() - ((float)Win.gameSize.y * Win.scale)) * 0.5f
	};
}

Ray GetWindowMouseRay(Camera camera) {
	Vector2 mouse = Win.scaledMouse;

	// TODO do some terribleness for this to work with letterboxing
	// TODO turn into own api function
	mouse = Vector2Scale(mouse, Win.scale);
	mouse = Vector2Add(mouse, GetWindowTopLeft());

	return GetMouseRay(mouse, camera);
}

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
