#define MAX_ITEMS 1024

template<class T>
struct MemoryArray {
    T items[MAX_ITEMS];
    int count;

    T* get(int i) {
        assert(i >= 0 && i < MAX_ITEMS);
        return &items[i];
    }

    void clear(){
        count = 0;
    }

    void paste(T *content, int i)
    {
        assert(i >= 0 && i < MAX_ITEMS);
        items[i] = *content;
        count++;
    }

    T* push(T content)
    {
        assert(count >= 0 && count < MAX_ITEMS);
        items[count] = content;
        T* item = &items[count];
        count++;
        return item;
    }
};


