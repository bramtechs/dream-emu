#include "assets.hpp"

static GameAssets Assets;

inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void assets_load() {
	if (!DirectoryExists(ASSETS_PATH)) {
		TraceLog(LOG_FATAL, "No asset folder found!");
	}

	auto files = LoadDirectoryFilesEx(ASSETS_PATH, NULL, true);
	for (int i = 0; i < files.count; i++) {
		std::string path = files.paths[i];
		TraceLog(LOG_INFO, "Loading --> %s...",path.c_str());

		if (ends_with(path, "png")) {
			Texture texture = LoadTexture(path.c_str());
			Assets.textures.insert({ path,texture });
		}
	}
	UnloadDirectoryFiles(files);

	TraceLog(LOG_INFO, "Loaded assets");
}

void assets_dispose() {
	for (auto& item : Assets.models) {
		UnloadModel(item.second);
	}
	for (auto& item : Assets.meshes) {
		UnloadMesh(item.second);
	}
	for (auto& item : Assets.shaders) {
		UnloadShader(item.second);
	}
	for (auto& item : Assets.textures) {
		UnloadTexture(item.second);
	}
	for (auto& item : Assets.images) {
		UnloadImage(item.second);
	}
	TraceLog(LOG_INFO, "Disposed assets");
}
