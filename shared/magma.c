#include "magma.h"

static size_t Allocations = 0;

void* M_MemAlloc(size_t size){
    void* ptr = MemAlloc(size);
    Allocations++;
    return ptr;
}

void CheckAllocations(){
    if (Allocations == 0){
        INFO("All allocations got freed!");
    }else{
        WARN("%d allocations did not get freed!",Allocations);
    }
}
