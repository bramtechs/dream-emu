// deflation.c

#include <stdio.h>
#include "raylib.h"

void deflate(const char* input, const char* output){
    FilePathList list = LoadDirectoryFilesEx(input, NULL, true);

    for (int i = 0; i < list.count; i++){
        printf("path: %s\n",list.paths[i]);
    }

    UnloadDirectoryFiles(list);
}

int main(void)
{
    const char* inputFolder = "X:\\raw_assets";
    const char* exportFolder = "X:\\assets";

    deflate(inputFolder,exportFolder);


    return 0;
}
