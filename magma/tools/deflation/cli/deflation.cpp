#include <filesystem>
#include <fstream>
#include <system_error>
#include <map>
#include <memory>
#include <fstream>

#include "deflated_assets.h"
#include "magma.h"

// CLI: tool to generate "Deflation" packages

// TODO v2: add compression

namespace fs = std::filesystem;

std::vector<std::string> get_supported_formats() {
	return {
		".png",
		".mpg",
		".comps",
		".obj",
		".mtl",
		".wav",
		".mp3" 
	};
};

typedef std::vector<RawAsset> PackList;

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
	for (auto format : get_supported_formats()) {
		if (TextIsEqual(ext, format.c_str())) {
			return true;
		}
	}
	return false;
}

void process(PackList& pack, std::string path, std::string shortPath) {
    RawAsset asset = {};

    uint size = 0;
    asset.data = (char*) LoadFileData(path.c_str(), &size);
    asset.size = (int64_t)size;
    strcpy_s(asset.path, PATH_MAX_LEN, shortPath.c_str());

    assert(asset.size > 0);
    assert(asset.data != NULL);

    pack.push_back(asset);
}

void save(PackList pack, const char* output) {
    // save into file
    uint size = 0;

    auto buffer = std::ofstream(output,std::ofstream::binary);
    int64_t amount = pack.size();
    buffer.write((char*)&amount,sizeof(int64_t));
    for (const auto& item : pack) {
        // INFO("Writing %d bytes of %s...", item.size, item.path);
        buffer.write((char*)item.path, PATH_MAX_LEN);
        buffer.write((char*)&item.size, sizeof(int64_t));
        buffer.write((char*)item.data, item.size);
    }
    buffer.close();
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

int main2(int argc, char** argv)
{
    SetTraceLogLevel(LOG_WARNING);

    if (argc <= 2){
        ERROR("Not enough arguments provided! Got %d args.",argc);
        return -1;
    }

    const char* inputFolder = argv[1];
    const char* exportFolder = argv[2];

    deflate(inputFolder, exportFolder);

    DeflationPack("assets.mga");

    return 0;
}

int main() {
    const char* inputFolder = "X:\\raw_assets";
    const char* exportFolder = "X:\\assets.mga";

    deflate(inputFolder, exportFolder);

    auto pack = DeflationPack("X:\\assets.mga");
    if (pack.succeeded) {
		pack.PrintAssetList();
    }

    Image img = pack.RequestImage("spr_wood_12");
    
    ExportImage(img, "output.png");
}
