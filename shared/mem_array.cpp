// heavy template abuse

template<typename T, int N>
struct GenericMemoryArray {
    T items[N];
    int count;

    T *get(int i)
    {
        assert(i >= 0 && i < N);
        return &items[i];
    }

    T *first()
    {
        return get(0);
    }

    T *last()
    {
        return get(count - 1);
    }

    void clear()
    {
        count = 0;
    }

    void paste(T *content, int i)
    {
        assert(i >= 0 && i < N);
        items[i] = *content;
        count++;
    }

    T *push(T content)
    {
        assert(count >= 0 && count < N);
        items[count] = content;
        T *item = &items[count];
        count++;
        return item;
    }
};

template<typename T>
struct SmallMemoryArray : GenericMemoryArray<T, 128> {
};
template<typename T>
struct MemoryArray : GenericMemoryArray<T, 1024> {
};
template<typename T>
struct BigMemoryArray : GenericMemoryArray<T, 4096> {
};