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
