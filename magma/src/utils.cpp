#include "utils.h"

bool CreateDirectory(const char* path){
    if (DirectoryExists(path)){
        DEBUG("Directory %s already exists, skipping folder creation...",path);
        return true;
    }

    if (!std::filesystem::create_directory(path)){
        ERROR("Failed to create directory %s! (code %d)", path);
        return false;
    }
    DEBUG("Made directory %s",path);
    return true;
}
