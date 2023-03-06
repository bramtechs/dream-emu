#include <filesystem>
#include <fstream>
#include <system_error>
#include <cstring>
#include <map>
#include <memory>
#include <fstream>

#include "magma.h"

// CLI: tool to generate "Deflation" packages
namespace fs = std::filesystem;

struct RawAsset {
    char path[PATH_MAX_LEN];
    int64_t size;
    char* data;
};

std::vector<std::string> get_supported_formats() {
    return {
        ".png",
        ".mpg",
        ".comps",
        ".obj",
        ".mtl",
        ".wav",
        ".mp3",
        ".ogg",
        ".pal",
        ".fs",
        ".vs",
        ".wav",
        ".ttf",
        ".mpeg"
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
    unsigned char* data = (unsigned char*) LoadFileData(path.c_str(), &size);

    asset.data = (char*) data;
    asset.size = size;

    strcpy(asset.path, shortPath.c_str());

    assert(asset.size > 0);
    assert(asset.data != NULL);

    pack.push_back(asset);
}

void save(PackList pack, const char* output, bool compress=false) {
    // save into file
    uint size = 0;

    std::string outputPath = compress ? TextFormat("%s_tmp",output):output;
    auto buffer = std::ofstream(outputPath, std::ofstream::binary);
    // >>> size
    int64_t amount = pack.size();
    buffer.write((char*)&amount,sizeof(int64_t));

    // >>> items
    for (const auto& item : pack) {
        // >>> item
        buffer.write((char*)item.path, PATH_MAX_LEN);
        buffer.write((char*)&item.size, sizeof(int64_t));
        buffer.write((char*)item.data, item.size);
    }
    buffer.close();

    if (compress) {
        // load uncompressed file and compress into new one
        unsigned int size = 0;
        unsigned char* data = LoadFileData(outputPath.c_str(),&size);

        INFO("Compressing %s...",outputPath.c_str());
        int compSize = 0;
        unsigned char* compData = CompressData(data, (int) size, &compSize);

        // sign file with MGA
        int compSignedSize = sizeof(char)*3+compSize;
        char* compSignedData = (char*) malloc(compSignedSize);
        compSignedData[0] = 0x4d;
        compSignedData[1] = 0x47;
        compSignedData[2] = 0x41;
        memcpy(&compSignedData[3],compData,compSize);

        UnloadFileData(data);
        MemFree((void*)compData);

        if (SaveFileData(output, (void*)compSignedData, compSignedSize)){
            int percentage = (int)(compSize / (float)size * 100.f);
            INFO("Wrote compressed package to %s (%d %%)",output,percentage);
        }
        else{
            ERROR("Failed to write compressed package to path %s",output);
        }

        MemFree(compSignedData);

        // delete uncompressed version
        if (std::remove(outputPath.c_str()) != 0){
            WARN("Could not removed uncompressed version located at %s",outputPath);
        }
    }
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
            process(pack, path, file);
        }
    }

    save(pack, output.c_str(), compress);
}

int run(std::vector<std::string> args) {

    if (args.size() <= 2){
        ERROR("Not enough arguments provided! Got %d args.",args.size());
        return -1;
    }

    bool doCompress = false;
    if (args[args.size()-1] == "--compress" || args[args.size()-1] == "--compressed"){
        doCompress = true;
        args.pop_back();
        INFO("Compressing package...");
    }

    std::vector<std::string> inputFolders;
    for (int i = 0; i <= args.size()-2; i++){
        auto path = args[i];
        inputFolders.push_back(path);
    }

    std::string exportFolder = args[args.size()-1];

    // check if an argument isn't being used as in-out folder
    for (const auto& folder : inputFolders){
        if (folder[0] == '-'){
            ERROR("An argument is being treated as input folder!");
            return -1;
        }
    }
    if (exportFolder[0] == '-'){
        ERROR("An argument is being treated as export folder!");
        return -1;
    }

    deflate(inputFolders, exportFolder, doCompress);

    INFO("Exported %s compressed package to %s", doCompress ? "":"un", exportFolder.c_str());

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
}
