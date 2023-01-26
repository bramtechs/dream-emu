#include "magma.h"

static Assets* _Assets;

bool try_init_assets(const char* file) {
    if (FileExists(file)) {
       INFO("Found assets at file %s ...",file);
       return true;
    }
    WARN("Did not find assets at %s, keep searching...",file);
    return false;
}

Assets* Assets::Init(const char* file) {
    INFO("Loading assets...");

    if (try_init_assets(file)) {
        _Assets = new Assets(file);
        return _Assets;
    }

    // visual studio
    if (try_init_assets("../../assets.mga")) {
        _Assets = new Assets("../../assets.mga");
        return _Assets;
    }

    return NULL;
}

void Assets::Dispose() {
    delete _Assets;
}

Assets::Assets(const char* file): pack(file) {
}

Assets::~Assets() {
    for (const auto& item : textures) {
        UnloadTexture(item.second);
    }
    for (const auto& item : models) {
        UnloadModel(item.second);
    }
}

Texture Assets::RequestTexture(const char* name) {

    // ATTEMPT 1: get cached texture
    for (const auto& item : _Assets->textures) {
        if (item.first == name) {
            return item.second;
        }
    }

    // ATTEMPT 2: load texture from package
    Texture texture = {};
    if (_Assets->pack.AssetExists(name)) {
        texture = _Assets->pack.RequestTexture(name);
    }
    else {
		// ATTEMPT 3: load texture from disk
        const char* path = TextFormat("raw_assets/%s", name);
		texture = LoadTexture(name);
		if (texture.width == 0) {
			// FAILED: generate placeholder instead
			Image temp = GenImageChecked(32, 32, 4, 4, RED, WHITE);
			texture = LoadTextureFromImage(temp);
			UnloadImage(temp);
		}
    }

    // push into texture array
    _Assets->textures.insert({ name,texture });
    return texture;
}

Image Assets::RequestImage(const char* name) {

    // ATTEMPT 1: load image from package
    Image image = {};
    if (_Assets->pack.AssetExists(name)) {
        image = _Assets->pack.RequestImage(name);
    }
    else {
		// ATTEMPT 2: load image from disk
        const char* path = TextFormat("raw_assets/%s", name);
		image = LoadImage(name);
		if (image.width == 0) {
			// FAILED: generate placeholder instead
			image = GenImageChecked(32, 32, 4, 4, RED, WHITE);
		}
    }

    return image;
}

Model Assets::RequestModel(const char* name) {
    // get cached model
    for (const auto& item : _Assets->models) {
        if (item.first == name) {
            return item.second;
        }
    }

    Model model = LoadModel(name);

    // raylib automatically handles if model isn't found
    // NOTE memcopying models doesn't seem to work so you'll have to dispose these things manually for now
    
    // push into model array
    _Assets->models.insert({ name,model });
    return model;
}

Shader Assets::RequestShader(const char* name){
    Shader shader = LoadShader(0, name);
    return shader;
}

Palette Assets::ParsePalette(const char* text) {
    std::string bloat(text);
    std::stringstream stream(bloat);

    int lineIndex = 0;
    std::string line;
    while (std::getline(stream, line, '\n')) {
        if (lineIndex >= 2) {
            std::cout << line << std::endl;
        }
        lineIndex++;
    }
    return {};
}

Palette Assets::RequestPalette(const char* name) {
    // ATTEMPT 1: load palette from package
    if (_Assets->pack.AssetExists(name)) {
        RawAsset asset = _Assets->pack.RequestCustom(name, ".pal");
        assert(asset.data != NULL);

        // todo load palette in
        Palette pal = ParsePalette(asset.data);
        return pal;
    }

	// ATTEMPT 2: load image from disk
	const char* path = TextFormat("raw_assets/%s", name);
    if (FileExists(path)) {
		char* paletteText = LoadFileText(path);
		Palette pal = ParsePalette(paletteText);
		UnloadFileText(paletteText);
        return pal;
    }
    return {};
}

// TODO inline
FilePathList Assets::IndexModels(){
    FilePathList list =  LoadDirectoryFilesEx(".", ".obj", true);
    for (int i = 0; i < list.count; i++){
        DEBUG(">>> %s",list.paths[i]);
    }
    // TODO dispose
    return list;
}

void Assets::EnterFailScreen(int width, int height) {
	Image grid = GenImageChecked(width+64, width+64, 32, 32, PURPLE, DARKPURPLE);
	Texture gridTexture = LoadTextureFromImage(grid);
	UnloadImage(grid);
	float offset = 0.f;
	const char* text = "Could not find 'assets.mga'.\nPlease extract your download.";
	while (!WindowShouldClose()) {
		BeginMagmaDrawing();
			ClearBackground(BLACK);
			DrawTexture(gridTexture, -offset, -offset, WHITE);
			DrawRectangleGradientV(0, 0, width, height+abs(sin(GetTime())*100), BLANK, PINK);
			offset += GetFrameTime() * 32.f;
			if (offset > 32){
				offset = 0;
			}
			Vector2 pos = Vector2Subtract({ width * 0.5f,height * 0.5f }, Vector2Scale(MeasureTextEx(GetFontDefault(), text, 28, 2), 0.5f));
			DrawTextEx(GetFontDefault(), text, pos, 28, 2, WHITE);
		EndMagmaDrawing();
		EndDrawing();
	}
	UnloadTexture(gridTexture);
}
