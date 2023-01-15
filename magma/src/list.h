#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "logger.h"
#include "memory.h"

#define uint unsigned int

// TODO clean this mess

// A serializable list that is contained in a single block. It uses relative pointers to allow navigation.

// HEADER 1 (ListItem)
//      / RAW <--- begin data block
//      | RAW
//      | RAW
// size | RAW
//      | RAW
//      \ RAW <--- end data block
// HEADER 2 (ListItem)
//      / RAW <--- begin data block
//      | RAW
//  ......etc....

typedef int ItemType;

typedef struct {
    uint size;
    ItemType type;
    char tag[128]; // optional
} ListItem;

typedef struct {
    char* data;
    uint size; // in bytes
    uint count; // number of items 
} List;

typedef struct {
    List* list;
    ListItem* current;
    unsigned int curIndex;
    ItemType filter;
} ListIterator;

List* MakeList();
void DisposeList(List* list);
void PushList(List* list, void* data, uint size, ItemType type);
void PushListEx(List* list, void* data, uint size, ItemType type, const char* tag);

ListIterator IterateListItems(List* list);
ListIterator IterateListItemsEx(List* list, ItemType filter);
bool IterateNextItem(ListIterator* it, void** result);
bool IterateNextItemEx(ListIterator* it, ItemType *type, void** result);

List* LoadList(const char* fileName);
void SaveList(List* list, const char* fileName);

void TestList();
