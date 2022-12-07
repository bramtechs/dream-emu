#include "linker.hpp"
#include <cstdio>
#include <dlfcn.h>

LEVEL_UPDATE_AND_STREAM level_update_and_stream;
LEVEL_LOAD level_load;

bool linker_try_load(void *handle, const char* fileName){
    handle = dlopen(fileName,RTLD_NOW | RTLD_GLOBAL);

    if (handle == nullptr){
        printf("Could not find library at %s, keep looking...!\n", fileName);
        return false;
    }
    return true;
}

bool linker_lib_link(){
    // try to free if previously loaded
    linker_lib_free();

    void *handle = nullptr;


    linker_try_load(handle, "libdreamemu_game.so");
    linker_try_load(handle, "cmake-build-debug/game/libdreamemu_game.dll");
    linker_try_load(handle, "../game/libdreamemu_game.dll");

    if (handle != nullptr)
    {
        level_update_and_stream = (LEVEL_UPDATE_AND_STREAM) dlsym(handle,"level_update_and_stream");
        level_load = (LEVEL_LOAD) dlsym(handle,"level_load");

        if (level_update_and_stream != nullptr && level_load != nullptr){
            return true;
        }
    }else{
        printf("Could not find library!\n");
    }
    return false;
}

void linker_lib_free(){

}
