#include "deflated_assets.h"

struct Explorer {
	const int FONT_SIZE = 18;
	const float BUTTON_HEIGHT = 24;

	float barWidth;
	float barOffsetY;

	DeflationPack* pack;
	std::vector<std::string> names;

	Explorer(const char* filePath) {
		pack = new DeflationPack(filePath);
		names = pack->GetAssetNames();
		barOffsetY = 0;

		// determine width of bar
		barWidth = 100;
		for (auto& name : names) {
			float len = MeasureText(name.c_str(), FONT_SIZE);
			if (len > barWidth) {
				barWidth = len;
			}
		}
	}

	~Explorer() {
		delete pack;
	}

	void update_and_render(float delta) {
		ClearBackground(SKYBLUE);

		float y = 10;
		for (auto& name : names) {
			DrawButton(name.c_str(), { 0,y,barWidth,BUTTON_HEIGHT });
			y += BUTTON_HEIGHT+5;
		}

		barOffsetY += GetMouseWheelMove()*BUTTON_HEIGHT;
	}

	void DrawButton(const char* name, Rectangle region) {
		Vector2 mouse = GetMousePosition();

		bool mouseOver = CheckCollisionPointRec(mouse, region);
		Color tint = mouseOver ? RAYWHITE : GRAY;

		DrawRectangle(region.x,region.y+barOffsetY,region.width,region.height, tint);
		DrawText(name, region.x + 5, region.y + 5, FONT_SIZE, BLACK);
	}
};
