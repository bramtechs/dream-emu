#include <filesystem>
#include <fstream>
#include <system_error>
#include <map>
#include <memory>
#include <fstream>

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
        ".mp3",
        ".pal",
        ".fs",
        ".vs",
        ".wav"
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

void deflate(std::vector<std::string>& inputFolders, std::string& output, bool compress=false) {
    // collect all file paths 
    std::vector<std::string> paths;
    for (const auto &folder : inputFolders){
        FilePathList files = LoadDirectoryFilesEx(folder.c_str(), NULL, true);
        for (int i = 0; i < files.count; i++){
            paths.push_back(files.paths[i]);
        }
        UnloadDirectoryFiles(files);
    }

    PackList pack;

    for (const auto &path : paths) {

        std::string file = GetFileName(path.c_str());
        const char* ext = GetFileExtension(file.c_str());

        if (should_include(ext)) {
            process(pack, path, file, compress);
        }
    }

    save(pack, output.c_str(), compress);
}

int run(std::vector<std::string> args) {
    if (args.size() <= 2){
        ERROR("Not enough arguments provided! Got %d args.",args.size());
        return -1;
    }

    std::vector<std::string> inputFolders;
    for (int i = 0; i <= args.size()-2; i++){
        auto path = args[i];
        inputFolders.push_back(path);
    }

    std::string exportFolder = args[args.size()-1];

    bool doCompress = false;
    if (args.size() > 3 && args[3] == "--compress") {
        doCompress = true;
        INFO("Compressing package...");
    }

    deflate(inputFolders, exportFolder, doCompress);

    INFO("Exported %scompressed package to %s", doCompress ? "":"un", exportFolder.c_str());

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

    //ChangeDirectory("X:\\temple");
    //args.push_back("raw_assets");
    //args.push_back("..\\core_assets");
    //args.push_back("assets.mga");
    //args.push_back("--compressed");
    //run(args);
}
