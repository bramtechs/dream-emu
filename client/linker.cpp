// needs to be compiled separately from raylib.h to avoid conflicts
#include "linker.hpp"
#include <windows.h>
#include <cstdio>

LEVEL_UPDATE_AND_STREAM level_update_and_stream;
LEVEL_LOAD level_load;

static HINSTANCE Library = nullptr;

bool linker_try_load(HINSTANCE *ins, const char* fileName){
    if (*ins == nullptr){
        *ins = LoadLibrary(fileName);
    }
    if (*ins == nullptr){
        printf("Could not find library at %s, keep looking...!\n", fileName);
        return false;
    }
    return true;
}

void linker_lib_free(){
    if (Library == nullptr) return;

    if (FreeLibrary(Library)){
        Library = nullptr;
        printf("Released shared library\n");
    }else{
        printf("Could not release shared library!\n");
    }
}

// TODO cleanup
bool linker_lib_link(){
    // try to free if previously loaded
    linker_lib_free();

    linker_try_load(&Library, "libdreamemu_game.dll");
    linker_try_load(&Library, "cmake-build-debug/game/libdreamemu_game.dll");
    linker_try_load(&Library, "../game/libdreamemu_game.dll");

    if (Library != nullptr){
        level_update_and_stream = (LEVEL_UPDATE_AND_STREAM) GetProcAddress(Library, "level_update_and_stream");

        if (level_update_and_stream != nullptr){
            printf("Linking succeeded!\n");
            level_load = (LEVEL_LOAD) GetProcAddress(Library, "level_load");

            if (level_load != nullptr){
                printf("Linking succeeded!\n");
                return true;
            }else{
                printf("Linking failed, could not get function!\n");
            }
            return true;
        }else{
            printf("Linking failed, could not get function!\n");
        }


    }else{
        printf("Could not find library!\n");
    }
    return false;
}