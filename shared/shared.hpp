#pragma once

// TODO move things out

#include <iostream>
#include <cassert>

#include "raylib.h"
#include "raymath.h"
#include "rlights.h"

#include "arena.cpp"
#include "mem_array.cpp"

#include "logger.cpp"

#define IS_DEBUG true

typedef unsigned int uint;

// TODO replace with more efficient solutions

static uint AllocationCount;

void* vmem_malloc(uint size){
    void* ptr = malloc(size);
    AllocationCount++;
    return ptr;
}

template<class T>
T* vmem_malloc(T data){
    T* ptr = malloc(sizeof(data));
    AllocationCount++;
    return ptr;
}

template<class T>
void vmem_free(T* ptr){
    free(ptr);
    ptr = nullptr;
    AllocationCount--;
}

void vmem_check(){
    if (AllocationCount == 0){
        // TODO write fancy macro
        logger_warn(TextFormat("Things aren't %d freed properly!",AllocationCount));
    }
}

template<class T>
T *NN(T *ptr) // pointer passed was null
{
    if (ptr == nullptr && IS_DEBUG)
    {
        int *i = nullptr;
        *i = -1;
    }
    return ptr;
}

template<class T>
T POS(T val) // value passed was not positive
{
    if (val <= 0 && IS_DEBUG)
    {
        int *i = nullptr;
        *i = -1;
    }
    return val;
}
