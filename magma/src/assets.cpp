#include "magma.h"

struct GameAssets {
	std::vector<RawAsset> assets;

	// caches
	std::map<std::string, Texture> textures;
	std::map<std::string, Model> models;
};

static GameAssets Assets = {};
static Texture PlaceholderTexture = {};

RawAsset QueryAsset(std::string name, std::string filterExt = "") {

	for (const auto& item : Assets.assets) {
		const char* base = GetFileNameWithoutExt(item.path);
		const char* ext = GetFileExtension(item.path);

		if (filterExt.empty() || TextIsEqual(ext, filterExt.c_str())) {
			if (TextIsEqual(base, name.c_str())) {
				RawAsset a;
				memcpy(&a, &item, sizeof(RawAsset));
				return a;
			}
		}

	}
	assert(false);
	return {};
}

bool LoadAssets() {
	INFO("Loading assets...");

	if (!ImportAssetPackage("assets.mga")) {
		// visual studio
		return ImportAssetPackage("../../assets.mga");
	}
	return true;
}

bool ImportAssetPackage(const char* filePath) {
	std::ifstream stream;
	try {
		stream = std::ifstream(filePath, std::ifstream::binary);
	}
	catch (const std::ifstream::failure& e) {
		TraceLog(LOG_ERROR, "Exception opening asset package at %s", filePath);
		return false;
	}

	// >> size
	int64_t count = -1;
	stream.read((char*)&count, sizeof(int64_t));

	bool isCompressed = false;
	// >> compressed flag
	stream.read((char*)&isCompressed, sizeof(bool));

	// >> items
	for (auto i = 0; i < count; i++) {
		// >> item
		RawAsset asset = {};
		stream.read(asset.path, PATH_MAX_LEN);

		stream.read((char*)&asset.size, sizeof(int64_t));
		assert(asset.size > 0); // TODO handle errors properly

		unsigned char* loaded = (unsigned char*)MemAlloc(asset.size);
		stream.read((char*)loaded, asset.size);

		if (isCompressed) {
			int size = 0;
			asset.data = (char*)DecompressData(loaded, asset.size, &size);
			asset.size = size;
			MemFree(loaded);
		}
		else {
			asset.data = (char*)loaded;
		}

		Assets.assets.push_back(asset);
		TraceLog(LOG_DEBUG, "Loaded asset %s", asset.path);
	}

	TraceLog(LOG_INFO, ("Loaded asset pack %s", filePath));
	stream.close();

	return true;
}

void DisposeAssets() {
	for (const auto& item : Assets.assets) {
		MemFree(item.data);
	}
	for (const auto& item : Assets.textures) {
		UnloadTexture(item.second);
	}
	for (const auto& item : Assets.models) {
		UnloadModel(item.second);
	}

	TraceLog(LOG_DEBUG, ("Disposed asset pack"));
	UnloadTexture(PlaceholderTexture);
}

Texture RequestTexture(const char* name) {
	// ATTEMPT 1: get cached texture
	for (const auto& item : Assets.textures) {
		if (item.first == name) {
			return item.second;
		}
	}

	// ATTEMPT 2: load texture from package
	Texture texture = {};
	if (IsAssetLoaded(name)) {
		bool succeeded = false;
		Image img = RequestImage(name, &succeeded);
		if (!succeeded || img.width == 0) { // loading image failed show placeholder
			if (PlaceholderTexture.width == 0) {
				Image img = GenImageColor(16, 16, RED);
				PlaceholderTexture = LoadTextureFromImage(img);
				UnloadImage(img);
			}
			return PlaceholderTexture;
		}
		texture = LoadTextureFromImage(img);
	}
	else {
		// ATTEMPT 3: load texture from disk
		const char* path = TextFormat("raw_assets/%s", name);
		texture = LoadTexture(name);
		if (texture.width == 0) {
			// generate placeholder on fail
			Image temp = GenImageChecked(32, 32, 4, 4, RED, WHITE);
			texture = LoadTextureFromImage(temp);
			UnloadImage(temp);
		}
	}

	// push into texture array
	Assets.textures.insert({ name,texture });
	return texture;
}

Image RequestImage(const char* name, bool* succeeded) {
	// ATTEMPT 1: load image from package
	Image image = {};
	if (IsAssetLoaded(name)) {
		// check if exists
		RawAsset asset = QueryAsset(name, ".png");
		if (asset.data == NULL) {
			TraceLog(LOG_ERROR, "Packaged image with name %s not found!", name);
			if (succeeded != NULL) *succeeded = false;
			return GenImageColor(16, 16, PURPLE);
		}
		if (GetAssetType(asset.path) != ASSET_TEXTURE) {
			TraceLog(LOG_ERROR, "Packaged asset with name %s is not an image/texture!", name);
			if (succeeded != NULL) *succeeded = false;
			return GenImageColor(16, 16, PINK);
		}
		const char* ext = GetFileExtension(asset.path);
		image = LoadImageFromMemory(ext, (const unsigned char*)asset.data, asset.size);
		if (image.width != 0) {
			if (succeeded != NULL) *succeeded = true;
		}
	}
	else {
		// ATTEMPT 2: load image from disk
		const char* path = TextFormat("raw_assets/%s", name);
		image = LoadImage(name);
		if (image.width == 0) {
			image = GenImageChecked(32, 32, 4, 4, RED, WHITE);
		}
		if (succeeded != NULL) *succeeded = false;
	}

	return image;
}

// This is a very dumb implementation, but might work
unsigned char* load_filedata_from_pack(const char* fileName, unsigned int* bytesRead) {
	const char* name = GetFileNameWithoutExt(fileName);
	const char* ext = GetFileExtension(fileName);
	char* data = RequestCustom(name, (size_t*)bytesRead, ext);
	DEBUG("Rerouting file data %s from memory...", fileName);
	return (unsigned char*)data;
}

char* load_filetext_from_pack(const char* fileName) {
	const char* name = GetFileNameWithoutExt(fileName);
	const char* ext = GetFileExtension(fileName);
	char* data = RequestCustom(name, NULL, ext);
	DEBUG("Rerouting file text %s from memory...", fileName);
	return (char*)TextFormat("%s\0", data); // add a null-terminator, just to be sure
}

Model RequestModel(const char* name) {

	// ATTEMPT 1: get cached model
	for (const auto& item : Assets.models) {
		if (item.first == name) {
			return item.second;
		}
	}

	Model model = {};
	// ATTEMPT 2: Load model from package
	if (IsAssetLoaded(name)) {
		// check if exists
		RawAsset asset = QueryAsset(name, ".obj");
		if (asset.data == NULL) {
			TraceLog(LOG_ERROR, "Packaged model with name %s not found!", name);
			return LoadModel(""); // force raylib to return default cube
		}
		if (GetAssetType(asset.path) != ASSET_MODEL) {
			TraceLog(LOG_ERROR, "Packaged asset with name %s is not a model!", name);
			return LoadModel(""); // force raylib to return default cube
		}

		// write material file to temp location
		std::string tempDir = GetTempDirectory();

		const char* origDir = GetWorkingDirectory();
		assert(ChangeDirectory(tempDir.c_str()));

		size_t size = 0;
		char* mtlData = RequestCustom(name, &size, ".mtl");
		if (size == 0) {
			const char* fileName = TextFormat("%s.mtl", name);
			SaveFileData(fileName, mtlData, size);
		}

		const char* ext = GetFileExtension(asset.path);

		// load textures from memory instead of disk
		SetLoadFileDataCallback(load_filedata_from_pack);
		SetLoadFileTextCallback(load_filetext_from_pack);

		Model model = LoadModel(asset.path);

		SetLoadFileTextCallback(NULL);
		SetLoadFileDataCallback(NULL);

		assert(ChangeDirectory(origDir));

		return model;
	}
	else {
		// ATTEMPT 3: Load model from disk
		const char* path = TextFormat("raw_assets/%s", name);
		model = LoadModel(name);
	}

	// raylib automatically handles if model isn't found

	// push into model cache array
	Assets.models.insert({ name,model });
	return model;
}

Shader RequestShader(const char* name) {
	Shader shader = LoadShader(0, name);
	return shader;
}

Palette ParsePalette(char* text) {
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
	// TODO
	assert(false);
	return {};
}

Palette RequestPalette(const char* name) {
	// ATTEMPT 1: load palette from package
	if (IsAssetLoaded(name)) {
		size_t size = 0;
		char* data = RequestCustom(name, &size, ".pal");

		// todo load palette in
		Palette pal = ParsePalette(data);
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

void ShowFailScreen(const char* text) {
	while (!WindowShouldClose()) {
		BeginMagmaDrawing();
		DrawCheckeredBackground(32, text, PURPLE, DARKPURPLE, PINK);
		EndMagmaDrawing();
		EndDrawing();
	}
}

char* RequestCustom(const char* name, size_t* size, const char* ext) {
	// check if exists
	RawAsset asset = QueryAsset(name, ext);
	if (asset.data == NULL) {
		TraceLog(LOG_ERROR, "Packaged custom with name %s not found!", name);
		return {};
	}

	// check extension
	if (ext != NULL) {
		const char* fext = ext;
		if (ext[0] != '.') {
			fext = TextFormat(".%s", ext);
		}
		if (!TextIsEqual(GetFileExtension(asset.path), fext)) {
			TraceLog(LOG_ERROR, "Custom packaged asset is not of type %s", fext);
			return {};
		}
	}

	// return memory
	if (size != NULL) {
		*size = asset.size;
	}

	return asset.data;
}

std::vector<std::string> GetAssetPaths() {
	std::vector<std::string> names;
	for (auto& item : Assets.assets) {
		names.push_back(item.path);
	}
	return names;
}

static std::string GetTempDirectory() {
	std::filesystem::path path = std::filesystem::temp_directory_path();
	std::string tempFolStr = path.string();
	return tempFolStr;
}

int GetAssetType(const char* name) {
	std::string ext = GetFileExtension(name);
	if (ext == ".png" || ext == ".gif" || ext == ".jpg" || ext == ".jpeg") {
		return ASSET_TEXTURE;
	}
	if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") {
		return ASSET_SOUND;
	}
	if (ext == ".obj" || ext == ".fbx") {
		return ASSET_MODEL;
	}
	return ASSET_CUSTOM;
}

void PrintAssetList() {
	for (const auto& item : Assets.assets) {
		TraceLog(LOG_INFO, "asset --> %s (size %d bytes)", item.path, item.size);
	}
}

size_t GetAssetCount() {
	return Assets.assets.size();
}

inline bool IsAssetLoaded(const char* name) {
	RawAsset asset = QueryAsset(name);
	return asset.data != NULL;
}
