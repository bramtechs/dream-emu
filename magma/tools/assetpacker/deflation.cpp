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

void process(PackList& pack, std::string path, std::string shortPath, bool compress=false) {
    RawAsset asset = {};

    uint size = 0;
    unsigned char* data = (unsigned char*) LoadFileData(path.c_str(), &size);

    if (compress) {
        unsigned char* compressed = CompressData(data, size, (int*) &size);
        UnloadFileData(data);

        asset.data = (char*) compressed;
    }
    else {
        asset.data = (char*) data;
    }
    asset.size = size;

    strcpy_s(asset.path, PATH_MAX_LEN, shortPath.c_str());

    assert(asset.size > 0);
    assert(asset.data != NULL);

    pack.push_back(asset);
}

void save(PackList pack, const char* output, bool compress=false) {
    // save into file
    uint size = 0;

    auto buffer = std::ofstream(output,std::ofstream::binary);
    // >>> size
    int64_t amount = pack.size();
    buffer.write((char*)&amount,sizeof(int64_t));
    // >>> compressed flag
    buffer.write((char*)&compress,sizeof(bool));

    // >>> items
    for (const auto& item : pack) {
        // >>> item
        DEBUG("Writing %d bytes of %s...", item.size, item.path);
        buffer.write((char*)item.path, PATH_MAX_LEN);
        buffer.write((char*)&item.size, sizeof(int64_t));
        buffer.write((char*)item.data, item.size);
    }
    buffer.close();
}

void deflate(const char* input, const char* output, bool compress=false) {
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
            process(pack, path, shortPath, compress);
        }
    }

    save(pack, output, compress);

    UnloadDirectoryFiles(files);
}

int run(std::vector<std::string> args) {
    if (args.size() <= 2){
        ERROR("Not enough arguments provided! Got %d args.",args.size());
        return -1;
    }

    std::string inputFolder = args[1];
    std::string exportFolder = args[2];

    bool doCompress = false;
    if (args.size() > 3 && args[3] == "--compress") {
        doCompress = true;
        INFO("Compressing package...");
    }

    deflate(inputFolder.c_str(), exportFolder.c_str(), doCompress);

    INFO("Exported %scompressed package to %s", doCompress ? "":"un", exportFolder);

    return 0;
}

int main(int argc, char** argv)
{
    SetTraceLogLevel(LOG_DEBUG);
    DEBUG("Launched at %s", GetWorkingDirectory());

    auto args = std::vector<std::string>();
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }
    run(args);

    //ChangeDirectory("X:\\");
    //args.push_back("assets_raw");
    //args.push_back("assets.mga");
    //args.push_back("--compressed");
    //run(args);
}
