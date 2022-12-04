#define MAX_ITEMS 1024

template <class T>
struct MemoryArray {
    T items[MAX_ITEMS];
    int count;
};

template <class T>
MemoryArray<T>* mem_array_new(){
    return new MemoryArray<T>();
}

template <class T>
void mem_array_free(MemoryArray<T> *pool){
    free(pool->items);
    delete pool;
}