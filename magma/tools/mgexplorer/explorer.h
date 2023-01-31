struct Explorer {
    const int FONT_SIZE = 18;

    bool succeeded;
    bool testMode;
    bool tileMode;

    float barWidth;
    float listOffsetY;

    Camera3D cam3;
    Camera2D cam2;

    size_t selectedIndex;

    std::vector<std::string> paths;

    Explorer(const char* filePath, bool testMode = false) {
        this->testMode = testMode;
        this->succeeded = ImportAssetPackage(filePath);
        this->tileMode = false;

        paths = GetAssetPaths();
        selectedIndex = 0;
        listOffsetY = 0;

        SetCameraMode(cam3, CAMERA_ORBITAL);
        cam3.position = { 0, 10, -10 };
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

        auto& selectedPath = paths.at(selectedIndex);
        const char* selectedName = GetFileNameWithoutExt(selectedPath.c_str());

        // ===== REPRESENT CHOOSEN ASSET ====

        int assetType = GetAssetType(selectedPath.c_str());
        switch (assetType) {
        case ASSET_TEXTURE:
        {
            const char* selectedName = GetFileNameWithoutExt(selectedPath.c_str());
            Texture texture = RequestTexture(selectedName);
            if (tileMode) {
                DrawTextureRec(texture, GetScreenBounds(), Vector2Zero(), WHITE);
            }
            else {
                DrawTexture(texture, 500, 200, WHITE);
            }

            const char* tooltip = "Toggle tiling by pressing T";
            int tlen = MeasureText(tooltip, 16);
            DrawText(tooltip, GetScreenWidth() - tlen - 30, GetScreenHeight() - 30, 18, LIGHTGRAY);

            if (IsKeyPressed(KEY_T)) {
                tileMode = !tileMode;
            }
        }
        break;
        case ASSET_MODEL:
        {
            const char* selectedName = GetFileNameWithoutExt(selectedPath.c_str());
            Model model = RequestModel(selectedName);
            model.transform = MatrixIdentity();

            if (!IsKeyDown(KEY_LEFT_SHIFT)) {
                UpdateCamera(&cam3);
            }

            // center camera
            BoundingBox box = GetModelBoundingBox(model);
            Vector3 size = Vector3Subtract(box.max, box.min);
            cam3.target = Vector3Add(box.min, Vector3Scale(size, 0.5f));

            ClearBackground(SKYBLUE);
            BeginMode3D(cam3);
            DrawModel(model, Vector3Zero(), 1.0f, WHITE);
            EndMode3D();
        }
        break;
        case ASSET_SOUND:
            DrawText("Audio playback not implemented", GetScreenWidth() / 3, GetScreenHeight() / 3, 36, RED);
            break;
        case ASSET_CUSTOM:
            DrawText("Custom unsupported datatype, use your imagination.", GetScreenWidth() / 3, GetScreenHeight() / 3, 36, RED);
            break;
        }

        // ===== UI STUFF ====
        static int bgWidth;

        if (!LoggerIsOpen()) {
            Color bgCol = ColorAlpha(BLACK, assetType == ASSET_MODEL ? 0.7f : 0.3f);
            DrawRectangle(0, 0, bgWidth + 20, GetScreenHeight(), bgCol);
            bgWidth = 0;

            float y = 5.0f + listOffsetY;
            int index = 0;
            for (auto& path : paths) {
                Color col = GetAssetTypeColor(path.c_str());
                float offsetX = selectedIndex == index ? 15 : 5;
                DrawButton(path.c_str(), index, offsetX, y, col);
                y += FONT_SIZE + 5;
                index++;

                int len = MeasureText(path.c_str(), FONT_SIZE);
                if (len > bgWidth) {
                    bgWidth = len;
                }
            }
        }
        else {
            bgWidth = 400;
        }

        // TEST MODE
        if (testMode) {
            DrawText("test mode", 360, 10, 12, ORANGE);
            selectedIndex++;
            if (selectedIndex >= GetAssetCount()) {
                WARN("TEST COMPLETE");
                CloseWindow();
            }
        }

        // confine selectedIndex
        selectedIndex = Wrap(selectedIndex, 0, GetAssetCount());

        Color typeColor = GetAssetTypeColor(selectedPath.c_str());
        DrawText(selectedPath.c_str(), bgWidth + 40, 50, 30, typeColor);

        // scrolling of the list
        if (assetType != ASSET_MODEL || IsKeyDown(KEY_LEFT_SHIFT)) {
            listOffsetY += GetMouseWheelMove() * FONT_SIZE;
        }

        DrawFPS(GetScreenWidth() - 120, 10);
    }

    void DrawButton(const char* text, size_t i, int x, int y, Color color = WHITE) {
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, FONT_SIZE, 3);
        Rectangle region = { (float) x, (float) y, textSize.x, textSize.y };

        bool mouseOver = CheckCollisionPointRec(GetMousePosition(), region);
        Color tint = mouseOver ? color : ColorBrightness(color, -0.3f);
        DrawText(text, x, y, FONT_SIZE, tint);

        // click on the button
        if (mouseOver && IsMouseButtonPressed(0)) {
            selectedIndex = i;
        }
    }

    Color GetAssetTypeColor(const char* path) {
        switch (GetAssetType(path)) {
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
