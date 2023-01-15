// deflation.c

#include "magma.h"

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

bool create_directory(const char* path){
    if (DirectoryExists(path)){
        DEBUG("Directory %s already exists, skipping folder creation...",path);
        return true;
    }

    int result = mkdir(path, 0777);
    if (result == -1){
        ERROR("Failed to create directory %s! (code %d)", path, result);
        return false;
    }
    DEBUG("Made directory %s",path);
    return true;
}

bool should_include(char const* ext){
    for (int i = 0; i < FORMAT_COUNT; i++){
        if (TextIsEqual(ext,FORMATS[i])){
            return true;
        }
    }
    return false;
}

void process(List* list, char const* path) {
    unsigned int size = 0;
    unsigned char* data = LoadFileData(path,&size);

    unsigned int compSize = 0;
    unsigned char* compData = CompressData(data, size, &compSize);

    MemFree(compData);
    UnloadFileData(data);

    PushListEx(list, compData, compSize, 0, path);
}

void deflate(const char* input, const char* output){
    FilePathList files = LoadDirectoryFilesEx(input, NULL, true);

    List* list = MakeList();

    for (int i = 0; i < files.count; i++){
        const char* path = files.paths[i];
        const char* ext = GetFileExtension(path);

        if (should_include(ext)){
            process(list, path);
        }
    }

    UnloadDirectoryFiles(files);

    // save result
    SaveList(list, output);
}

int main(void)
{
    SetTraceLogLevel(LOG_ALL);

    const char* inputFolder = "X:\\raw_assets";
    const char* exportFolder = "X:\\assets.mga";

    deflate(inputFolder,exportFolder);

    return 0;
}
