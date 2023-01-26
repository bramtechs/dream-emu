#include "deflated_assets.h"

struct Explorer {
    const int FONT_SIZE = 18;

    float barWidth;
    float listOffsetY;

    DeflationPack* pack;
    std::vector<std::string> names;

    Explorer(const char* filePath) {
        pack = new DeflationPack(filePath);
        names = pack->GetAssetNames();
        listOffsetY = 0;
    }

    ~Explorer() {
        delete pack;
    }

    void update_and_render(float delta) {
        ClearBackground(SKYBLUE);

        float y = 5.0f + listOffsetY;
        for (auto& name : names) {
            Color col = GetAssetTypeColor(name);
            DrawButton(name.c_str(), 5, y, col);
            y += FONT_SIZE+5;
        }

        listOffsetY += GetMouseWheelMove()*FONT_SIZE;
    }

    void DrawButton(const char* text, int x, int y, Color color=WHITE) {
        Vector2 textSize = MeasureTextEx(GetFontDefault(),text,FONT_SIZE,3);
        Rectangle region = { x, y, textSize.x, textSize.y };

        bool mouseOver = CheckCollisionPointRec(GetMousePosition(), region);
        Color tint = mouseOver ? color : ColorBrightness(color,-0.3f);
        DrawText(text, x, y, FONT_SIZE, tint);
    }

    Color GetAssetTypeColor(std::string& path){
        switch (GetAssetType(path.c_str())){
            case CUSTOM:
                return PINK;
            case TEXTURE:
                return GREEN;
            case MODEL:
                return RED;
            case SOUND:
                return BLUE;
            default:
                return WHITE;
        }
        return WHITE;
    }
};
