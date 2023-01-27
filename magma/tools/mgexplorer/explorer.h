struct Explorer {
    const int FONT_SIZE = 18;

    float barWidth;
    float listOffsetY;

    Camera3D cam3;
    Camera2D cam2;

    size_t selectedIndex;

    std::vector<std::string> paths;

    Explorer(const char* filePath) {
        ImportAssetPackage(filePath);

        paths = GetAssetPaths();
        selectedIndex = 0;
        listOffsetY = 0;

        SetCameraMode(cam3, CAMERA_ORBITAL);
        cam3.position = { 0, 0, -10 };
        cam3.target = Vector3Zero();
        cam3.up = { 0, 1, 0 };
        cam3.fovy = 80;
        cam3.projection = CAMERA_PERSPECTIVE;
    }

    ~Explorer() {
        DisposeAssets();
    }

    void UpdateAndRender(float delta) {
        ClearBackground(BLACK);

        if (!LoggerIsOpen()){
            float y = 5.0f + listOffsetY;
            int index = 0;
            for (auto& path : paths) {
                Color col = GetAssetTypeColor(path.c_str());
                float offsetX = selectedIndex == index ? 15:5; 
                DrawButton(path.c_str(), index, offsetX, y, col);
                y += FONT_SIZE+5;
                index++;
            }
        }

        listOffsetY += GetMouseWheelMove()*FONT_SIZE;

        // confine selectedIndex
        selectedIndex = Wrap(selectedIndex, 0, GetAssetCount());

        // draw selected asset
        const char* selectedPath = paths.at(selectedIndex).c_str();
        Color typeColor = GetAssetTypeColor(selectedPath);
        DrawText(selectedPath, 400, 50, 30, typeColor);
        
        const char* selectedName = GetFileNameWithoutExt(selectedPath);
        switch (GetAssetType(selectedPath)){
            case ASSET_TEXTURE:
                Texture texture = RequestTexture(selectedName);
                DrawTexture(texture,400,150,WHITE);
                break;
            case ASSET_MODEL:
                Model model = RequestModel(selectedName);
                UpdateCamera(&cam3);
                BeginMode3D(cam3);
                    DrawModel(model,Vector3Zero(),1.0f,WHITE);
                EndMode3D();
                break;
            case ASSET_SOUND:
                break;
            case ASSET_CUSTOM:
                break;
        }
    }

    void DrawButton(const char* text, size_t i, int x, int y, Color color=WHITE) {
        Vector2 textSize = MeasureTextEx(GetFontDefault(),text,FONT_SIZE,3);
        Rectangle region = { x, y, textSize.x, textSize.y };

        bool mouseOver = CheckCollisionPointRec(GetMousePosition(), region);
        Color tint = mouseOver ? color : ColorBrightness(color,-0.3f);
        DrawText(text, x, y, FONT_SIZE, tint);

        // click on the button
        if (mouseOver && IsMouseButtonPressed(0)){
            selectedIndex = i;
        }
    }

    Color GetAssetTypeColor(const char* path){
        switch (GetAssetType(path)){
            case ASSET_CUSTOM:
                return PINK;
            case ASSET_TEXTURE:
                return GREEN;
            case ASSET_MODEL:
                return RED;
            case ASSET_SOUND:
                return BLUE;
            default:
                return WHITE;
        }
        return WHITE;
    }
};
