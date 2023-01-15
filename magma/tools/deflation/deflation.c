// deflation.c

#include <stdio.h>
#include "raylib.h"

#ifdef __unix___

#define LOG(X...)   TraceLog(LOG_INFO, X)
#define ERROR(X...) TraceLog(LOG_ERROR,X)
#define DEBUG(X...) TraceLog(LOG_DEBUG,X)

#elif defined(_WIN32) || defined(WIN32)

#define LOG(...)   TraceLog(LOG_INFO, __VA_ARGS__)
#define ERROR(...) TraceLog(LOG_ERROR,__VA_ARGS__)
#define DEBUG(...) TraceLog(LOG_DEBUG,__VA_ARGS__)

#endif

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

#define BLOCK_AMOUNT 10
void process(char const* path, char const* output) {
    unsigned int size = 0;
    unsigned char* data = LoadFileData(path,&size);

    unsigned int compSize = 0;
    unsigned char* compData = CompressData(data, size, &compSize);

    int percentage = (int) ((compSize / (float) size) * 100.f);

    char blocks[BLOCK_AMOUNT+1];
    for (int i = 0; i < BLOCK_AMOUNT; i++){
        blocks[i] = percentage / BLOCK_AMOUNT > i ? '#':' ';
    }
    blocks[BLOCK_AMOUNT] = '\0';
    LOG ("Compressed %s (%02d%%) [%s]",path, percentage, blocks);

    UnloadFileData(data);
}

void deflate(const char* input, const char* output){
    FilePathList list = LoadDirectoryFilesEx(input, NULL, true);

    for (int i = 0; i < list.count; i++){
        const char* path = list.paths[i];
        const char* ext = GetFileExtension(path);

        if (should_include(ext)){
            const char* fileName = GetFileName(path);
            const char* outputPath = TextFormat("%s/%s",output,fileName);
            process(path, outputPath);
        }
    }

    UnloadDirectoryFiles(list);
}

int main(void)
{
    SetTraceLogLevel(LOG_ALL);

    const char* inputFolder = "X:\\raw_assets";
    const char* exportFolder = "X:\\assets";
    deflate(inputFolder,exportFolder);


    return 0;
}
