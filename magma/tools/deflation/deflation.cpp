// deflation.c

#include <filesystem>
#include <fstream>
#include <system_error>
#include <map>
#include <memory>

#include "magma.h"

namespace fs = std::filesystem;

static const char* FORMATS[] = {
	".png",
	".mpg",
	".comps",
	".obj",
	".mtl",
	".wav",
	".mp3",
};
#define FORMAT_COUNT 7

struct Asset {
	char path[128];
	int64_t size;
	void* data;
};

typedef std::vector<Asset> PackList;

bool create_directory(const char* path) {
	if (DirectoryExists(path)) {
		DEBUG("Directory %s already exists, skipping folder creation...", path);
		return true;
	}

	if (fs::create_directories(path)) {
		ERROR("Failed to create directory %s!", path);
		return false;
	}
	DEBUG("Made directory %s", path);
	return true;
}

bool should_include(char const* ext) {
	for (int i = 0; i < FORMAT_COUNT; i++) {
		if (TextIsEqual(ext, FORMATS[i])) {
			return true;
		}
	}
	return false;
}

void process(PackList& pack, std::string path, std::string shortPath) {

	uint size = 0;
	unsigned char* data = LoadFileData(path.c_str(), &size);

	int compSize = 0;
	Asset asset = {};
	asset.data = (void*)CompressData(data, size, &compSize);
	strcpy_s(asset.path, 128, shortPath.c_str());
	asset.size = (int64_t)compSize;

	assert(asset.size > 0);
	assert(asset.data != NULL);

	pack.push_back(asset);

	UnloadFileData(data);
}

void save(PackList pack, const char* output) {
	// save into file
	uint size = 0;
	char* outPtr = (char*)M_MemAlloc(1);

	auto outFile = std::ofstream(output);
	for (const auto& item : pack) {
		INFO("Writing %d bytes of %s...", item.size, item.path);

		outFile.write((char*)item.path, 128);
		outFile.write((char*)&item.size, sizeof(int64_t));
		outFile.write((char*)item.data, item.size);
	}
	outFile.close();

	MemFree(outPtr);
}

void deflate(const char* input, const char* output) {
	FilePathList files = LoadDirectoryFilesEx(input, NULL, true);

	PackList pack;

	for (int i = 0; i < files.count; i++) {

		std::string shortPath = files.paths[i];
		shortPath.erase(0, strlen(input));
		// remove slashes if present
		if (shortPath[0] == '\\' || shortPath[0] == '/') {
			shortPath.erase(0, 1);
		}
		// tilt slashes forwards
		std::replace(shortPath.begin(), shortPath.end(), '\\', '/');

		std::string path = files.paths[i];
		const char* ext = GetFileExtension(path.c_str());

		if (should_include(ext)) {
			process(pack, path, shortPath);
		}
	}

	save(pack, output);

	UnloadDirectoryFiles(files);

	CheckAllocations();
}

int main(void)
{
	SetTraceLogLevel(LOG_ALL);

	const char* inputFolder = "X:\\raw_assets";
	const char* exportFolder = "X:\\assets.mga";

	deflate(inputFolder, exportFolder);

	return 0;
}
