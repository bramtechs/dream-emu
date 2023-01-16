#include "list.h"

List MakeList(){
    return List();
}

void DisposeList(List &list){
    for (const auto &it = list.begin();it != list.end();++it)
    {
        M_MemFree(it.second);
    }
}

void PushList(List* list, void* data, uint size, ItemType type){
    PushListEx(list,data,size,type,"Unknown");
}

void PushListEx(List* list, void* data, uint size, ItemType type, const char* tag){
    assert(list->data);
    assert(data);

    ListItem* newItem = (ListItem*) (list->data + list->size);

    // realloc enough memory
    list->size += sizeof(ListItem) + size;
    list->data = (char*) MemRealloc(list->data, list->size);

    newItem->size = size;
    newItem->type = type;
    strcpy(newItem->tag, tag);

    char* target = (char*) newItem + sizeof(ListItem);
    memcpy(target, data, size);
    list->size += sizeof(ListItem) + size;
    list->count++;
}

ListIterator IterateListItems(List* list){
    assert(list);
    ListItem* item = list->data;
    return (ListIterator) {
        list,
        item,
        0,
        -1
    };
}

ListIterator IterateListItemsEx(List* list, ItemType filter){
    ListIterator it = IterateListItems(list);
    it.filter = filter;
    return it;
}

bool IterateNextItemEx(ListIterator* it, ItemType* type, void** result){
    List* list = it->list;

    if (it->current == NULL){
        return false; // list is empty, do nothing
    }

    while (it->curIndex < list->count){
        // get cur item data
        ListItem* curItem = it->current;
        if (type != NULL){
            *type = curItem->type;
        }

        char* curItemPtr = curItem;
        *result = (void*) (curItemPtr + sizeof(ListItem));

        // register next item
        char* nextItemPtr = (char*) (curItemPtr + sizeof(ListItem) + curItem->size);
        it->current = (ListItem*) nextItemPtr;

        it->curIndex++;

        if (it->filter == -1 || it->filter == curItem->type){
            return true;
        }
    }

    return false; // reached end of list
}

bool IterateNextItem(ListIterator* it, void** result){
    return IterateNextItemEx(it,NULL,result);
}

List* LoadList(const char* fileName) {
    uint totalSize = 0;

    if (!FileExists(fileName)) {
        assert(false);
    }

    List* list = (List*) LoadFileData(fileName, &totalSize);
    char* rawDataPtr = (char*) list + sizeof(List);
    list->data = rawDataPtr;
    return list;
}

void SaveList(List* list, const char* fileName){
    uint size = sizeof(List)+list->size;

    char* data = M_MemAlloc(size);
    memcpy(data,list,sizeof(List)); // copy header

    char* rawDataPtr = data + sizeof(List);
    memcpy(rawDataPtr,list->data,list->size); // copy raw data

    if (SaveFileData(fileName, data, size)){
        DEBUG("Saved list to %s", fileName);
    }else{
        ERROR("Failed to export list!");
    }
    M_MemFree(data);
}

void TestList(){
    List* list = MakeList();

    for (int i = 0; i < 500; i++){
        int intNumber = i;
        PushList(list,&intNumber,sizeof(int),0);
        printf("--> %d\n",intNumber);
    }

    for (int i = 0; i < 500; i++){
        long longNumber = 1000000000 + i;
        PushList(list,&longNumber,sizeof(long),1);
        printf("--> %d\n",longNumber);
    }

    printf("done, now reading...\n");

    ListIterator it = IterateListItems(list);

    ItemType type = 0;
    void* data = NULL;
    while (IterateNextItem(&it,&type,&data)){
        switch (type){
            case 0:
                printf("<-- %d\n",*(int*)data);
                break;
            case 1:
                printf("<-- %d\n",*(long*)data);
                break;
            default:
                printf("<-- UNKNOWN TYPE\n");
                assert(false);
                break;
        }
    }
}
