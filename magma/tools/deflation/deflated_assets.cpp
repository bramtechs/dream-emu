#include "deflated_assets.h"

static int GetAssetType(const char* name) {
	std::string ext = GetFileExtension(name);
	if (ext == ".png" || ext == ".gif" || ext == ".jpg" ||ext == ".jpeg") {
		return TEXTURE;
	}
	if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") {
		return SOUND;
	}
	if (ext == ".obj" || ext == ".fbx") {
		return MODEL;
	}
	return CUSTOM;
}

DeflationPack::DeflationPack(const char* filePath) {
	succeeded = false;
	std::ifstream stream;
	try {
		stream = std::ifstream(filePath,std::ifstream::binary);
		succeeded = true;
	}
	catch (const std::ifstream::failure& e) {
		TraceLog(LOG_ERROR,"Exception opening asset package at %s",filePath);
		return;
	}

	// read total items
	int64_t count = -1;
	char* countPtr = (char*) &count;
	stream.read(countPtr,sizeof(int64_t));

	for (auto i = 0; i < count; i++) {
		RawAsset asset = {};
		stream.read(asset.path, PATH_MAX_LEN);

		char* sizePtr = (char*)&asset.size;
		stream.read(sizePtr, sizeof(int64_t));
		assert(asset.size > 0); // TODO handle errors properly

		asset.data = (char*) MemAlloc(asset.size);
		stream.read(asset.data, asset.size);

		assets.push_back(asset);
		TraceLog(LOG_DEBUG,"Loaded asset %s", asset.path);
	}

	TraceLog(LOG_INFO,("Loaded asset pack %s", filePath));
	stream.close();
}

DeflationPack::~DeflationPack() {
	for (const auto& item : assets) {
		MemFree(item.data);
	}
	TraceLog(LOG_DEBUG,("Disposed asset pack"));
}

void DeflationPack::PrintAssetList() {
	for (const auto& item : assets) {
		TraceLog(LOG_INFO,"asset --> %s (size %d bytes)",item.path,item.size);
	}
}

bool DeflationPack::AssetExists(const char* name) {
	return QueryAsset(name) != NULL;
}

Texture DeflationPack::RequestTexture(const char* name) {
	Image img = RequestImage(name);
	return LoadTextureFromImage(img);
}

Image DeflationPack::RequestImage(const char* name) {
	// check if exists
	const RawAsset* asset = QueryAsset(name);
	if (asset == NULL) {
		TraceLog(LOG_ERROR,"Packaged image with name %s not found!", name);
		return GenImageColor(16,16,PURPLE);
	}
	if (GetAssetType(asset->path) != TEXTURE) {
		TraceLog(LOG_ERROR,"Packaged asset with name %s is not an image/texture!", name);
		return GenImageColor(16,16,PURPLE);
	}
	const char* ext = GetFileExtension(asset->path);
	return LoadImageFromMemory(ext, (const unsigned char*) asset->data, asset->size);
}

const RawAsset* DeflationPack::QueryAsset(const char* name) {
	for (const auto &item : assets) {
		auto base = GetFileNameWithoutExt(item.path);
		if (TextIsEqual(base,name)) {
			return &item;
		}
	}
	return NULL;
}
