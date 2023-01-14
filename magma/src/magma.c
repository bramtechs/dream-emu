#include "magma.h"

static size_t Allocations = 0;

void* M_MemAlloc(size_t size){
    void* ptr = MemAlloc(size);
    Allocations++;
    return ptr;
}

void* M_MemFree(void* ptr)
{
    MemFree(ptr);
    Allocations--;
}

void assert(bool cond) {
    if (!cond) {
        int* ptr = NULL;
        *ptr = 666;
    }
}

void CheckAllocations(){
    if (Allocations == 0){
        INFO("All allocations got freed!");
    }else{
        WARN("%d allocations did not get freed!",Allocations);
    }
}

bool CreateDirectory(const char* path){
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
