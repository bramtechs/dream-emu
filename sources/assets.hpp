#include "common.hpp"
#include <unordered_map>

template <typename T>
struct AssetType : std::unordered_map<std::string, T> {};

struct GameAssets {
	AssetType<Model> models;
	AssetType<Mesh> meshes;
	AssetType<Shader> shaders;
	AssetType<Texture> textures;
	AssetType<Image> images;
};

extern GameAssets Assets;

void assets_load();
void assets_dispose();