#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "logger.h"
#include "memory.h"

// basically growing arrays
typedef void(*ITER_FUNC)(int i, void*);

typedef struct {
    void* data;
    size_t itemSize;
    size_t capacity;
    size_t count;
} Array;

Array* MakeArray(size_t itemSize);
void DisposeArray(Array* array);

void* GetArrayItemRaw(Array* array, size_t index);

#define GetArrayItem(ARRAY,INDEX,TYPE) (TYPE*) GetArrayItemRaw(ARRAY,INDEX)

void* PushArrayRaw(Array* array, void* item);

#define PushArray(ARRAY,TYPE, ITEM_PTR) (TYPE*) PushArrayRaw(ARRAY,ITEM_PTR)

void IterateArray(Array* array, ITER_FUNC func);
