#include "magma.h"

static Assets* _Assets = NULL;

bool try_init_assets(const char* file) {
    if (FileExists(file)) {
       INFO("Found assets at file %s ...",file);
       return true;
    }
    WARN("Did not find assets at %s, keep searching...",file);
    return false;
}

Assets* Assets::Init(const char* folder) {
    assert(_Assets != NULL);
    _Assets = new(Assets);

    INFO("Loading assets...");

    if (try_init_assets(folder)) {
        return _Assets;
    }

    // visual studio
    if (try_init_assets("../../assets.mga")) {
        return _Assets;
    }

    return NULL;
}

void Assets::Dispose() {
    delete _Assets;
}

Assets::~Assets() {
    for (int i = 0; i < textureCount; i++){
        UnloadTexture(textures[i].texture);
    }
    for (int i = 0; i < modelCount; i++){
        UnloadModel(models[i].model);
    }
}

Texture Assets::RequestTexture(const char* name) {

    // ATTEMPT 1: get cached texture
    for (int i = 0; i < _Assets->textureCount; i++){
        TextureContainer cont = _Assets->textures[i];
        if (TextIsEqual(cont.name,name)){
            return cont.texture;
        }
    }

    // ATTEMPT 2: load texture from package
    //if (texture.width == 0) {
    //}

    // ATTEMPT 3: load texture from disk
    Texture texture = LoadTexture(name);
    if (texture.width == 0) {
        // failed, generate placeholder instead
        Image temp = GenImageChecked(32, 32, 4, 4, RED, WHITE);
        texture = LoadTextureFromImage(temp);
        UnloadImage(temp);
    }

    // push into texture array
    TextureContainer* cont = &_Assets->textures[_Assets->textureCount];
    strcpy(cont->name, name);
    cont->texture = texture;

    _Assets->textureCount++;
    return texture;
}

Model Assets::RequestModel(const char* name) {
    // get cached model
    for (int i = 0; i < _Assets->modelCount; i++){
        ModelContainer cont = _Assets->models[i];
        if (TextIsEqual(cont.name,name)){
            return cont.model;
        }
    }

    Model model = LoadModel(name);

    // raylib automatically handles if model isn't found
    // NOTE memcopying models doesn't seem to work so you'll have to dispose these things manually for now
    
    // push into model array
    ModelContainer* cont = &_Assets->models[_Assets->modelCount];
    strcpy(cont->name, name);
    cont->model = model;

    _Assets->modelCount++;
    return model;
}

Shader Assets::RequestShader(const char* name){
    Shader shader = LoadShader(0, name);
    return shader;
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
