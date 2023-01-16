#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <map>
#include "logger.h"
#include "memory.h"

#define uint unsigned int

typedef List std::multimap<uint,void*>;
typedef int ItemType;

typedef struct {
    ItemType type;
    char tag[128];
} ListItem;

List MakeList();
void DisposeList(List &list);

template <typename T>
void PushList(List &list, T* data, ItemType type);

template <typename T>
void PushListEx(List &list, T* data, ItemType type, const char* tag);

List LoadList(const char* fileName);
void SaveList(List* list, const char* fileName);

void TestList();
