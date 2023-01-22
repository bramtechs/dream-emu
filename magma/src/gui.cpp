#include "magma.h"

#define FADE_IN  0
#define DISPLAY  1
#define FADE_OUT 2

MainMenu::MainMenu(MainMenuConfig config, bool skipSplash) 
	: skipSplash(skipSplash), config(config) {

	// initialize session
	state = FADE_IN;
	curSplash = 0;
	timer = 0.f;
	alpha = 0.f;
	isDone = false;

	// load all textures
	bgTexture = Assets::RequestTexture(config.bgPath);
	for (const auto &splash : config.splashes) {
		Texture img = Assets::RequestTexture(splash.imgPath);
		splashTextures.push_back(img);
	}

	INFO("Booting main menu!");
}

void MainMenu::DrawScreenSaver(float delta) {
	ClearBackground(saveCol);
	Color* c = &saveCol;
	c->r += delta * 10.f;
	c->g -= delta * 10.f;
	c->b += delta * 20.f;
	c->a = 255;
}

void MainMenu::DrawBackground(Texture texture, Color tint) {
	Rectangle src = { 0, 0, texture.width, texture.height };
	Vector2 winSize = GetMagmaGameSize();
	Rectangle dest = { 0, 0, winSize.x, winSize.y };
	DrawTexturePro(texture, src, dest, Vector2Zero(), 0.f, tint);
}

bool MainMenu::UpdateAndDraw(float delta) {
	if (skipSplash || isDone || config.width == 0) { // skip if not booted
		return true;
	}

	float waitTime = FADE_DURATION;
	Texture texture = {};

	if (curSplash < config.splashes.size()) {
		texture = splashTextures[curSplash];
		switch (state) {
		case FADE_IN:
			alpha += delta / FADE_DURATION;
			break;
		case DISPLAY: {
				SplashScreen splash = config.splashes[curSplash];
				waitTime = splash.duration;
				alpha = 1.f;
			}
			break;
		case FADE_OUT:
			alpha -= delta / FADE_DURATION;
			break;
		default:
			assert(false);
			break;
		}

		if (timer > waitTime) {
			timer -= waitTime;
			state++;
			if (state > FADE_OUT) {
				state = FADE_IN;
				curSplash++;
			}
		}
		timer += delta;
	}
	else {
		alpha = 1.f;
		texture = bgTexture;
	}

	BeginMagmaDrawing();

	char alphaByte = Clamp(alpha * 255, 0, 255);
	Color tint = { 255, 255, 255, alphaByte };
	DrawBackground(texture, tint);

	if (curSplash >= config.splashes.size()) {
		DrawText(config.title, 20, 20, 36, WHITE);
	}

	if (IsKeyPressed(KEY_ENTER)) {
		isDone = true;
	}

	EndMagmaDrawing();
	EndDrawing();

	return false;
}
