#pragma once

#include <iostream>
#include <fstream>

#include "magma.h"

#define PATH_MAX_LEN 128

// API to access "Deflation" asset packs

#define CUSTOM	-1
#define TEXTURE 0
#define MODEL	1
#define SOUND	2

struct RawAsset {
    char path[PATH_MAX_LEN];
    int64_t size;
    char* data;
};

class DeflationPack {

public:
	bool succeeded;
	std::vector<RawAsset> assets;

	DeflationPack(const char* filePath);
	~DeflationPack();

	void PrintAssetList();

	Image RequestImage(const char* name);
	Texture RequestTexture(const char* name); // NOTE: GL context required!

private:
	const RawAsset* QueryAsset(const char* name);
};

// Implementation
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
		ERROR("Exception opening asset package at %s",filePath);
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

		asset.data = (char*) M_MemAlloc(asset.size);
		stream.read(asset.data, asset.size);

		assets.push_back(asset);
		DEBUG("Loaded asset %s", asset.path);
	}

	INFO("Loaded asset pack %s", filePath);
	stream.close();
}

DeflationPack::~DeflationPack() {
	for (const auto& item : assets) {
		M_MemFree(item.data);
	}
	DEBUG("Disposed asset pack");
}

void DeflationPack::PrintAssetList() {
	for (const auto& item : assets) {
		INFO("asset --> %s (size %d bytes)",item.path,item.size);
	}
}

Texture DeflationPack::RequestTexture(const char* name) {
	Image img = RequestImage(name);
	return LoadTextureFromImage(img);
}

Image DeflationPack::RequestImage(const char* name) {
	// check if exists
	const RawAsset* asset = QueryAsset(name);
	if (asset == NULL) {
		ERROR("Packaged image with name %s not found!", name);
		return GenImageColor(16,16,PURPLE);
	}
	if (GetAssetType(asset->path) != TEXTURE) {
		ERROR("Packaged asset with name %s is not an image/texture!", name);
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
