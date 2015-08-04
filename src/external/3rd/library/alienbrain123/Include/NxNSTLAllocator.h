#ifndef INC_NXN_STLALLOCATOR_H
#define INC_NXN_STLALLOCATOR_H


template<class T>
class CNxNSTLAllocator : public std::allocator<T>
{
    public:
        CNxNSTLAllocator() {}

        pointer allocate(size_type n, const void *hint)
        {
            size_type nBytes = n*sizeof(T);

            if (hint != NULL)
            {
                return (pointer)::CoTaskMemRealloc((void *)hint, nBytes);
            }

            return (pointer)::CoTaskMemAlloc(nBytes);
        }

        pointer _Charalloc(size_type nBytes)
        {
            return (pointer)::CoTaskMemAlloc(nBytes);
        }

        void deallocate(void *p, size_type n)
        {
            if (p != NULL)
            {
                ::CoTaskMemFree(p);
            }
        }
};


#endif

