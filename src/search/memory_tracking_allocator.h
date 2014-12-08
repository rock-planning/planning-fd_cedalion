#ifndef MEMORY_TRACKING_ALLOCATOR_H
#define MEMORY_TRACKING_ALLOCATOR_H


#include <vector>

extern unsigned long g_memory_tracking_allocated;

template<class T>
struct MemoryTrackingAllocator : std::allocator<T> {
    MemoryTrackingAllocator() {}

    template<class Other>
    MemoryTrackingAllocator( const MemoryTrackingAllocator<Other>& ) {}

    template<class U>
    struct rebind { typedef MemoryTrackingAllocator<U> other; };

    typedef std::allocator<T> base;

    typedef typename base::pointer pointer;
    typedef typename base::size_type size_type;

    pointer allocate(size_type n) {
        g_memory_tracking_allocated += n * sizeof(T);
        return this->base::allocate(n);
    }

    pointer allocate(size_type n, void const* hint) {
        g_memory_tracking_allocated += n * sizeof(T);
        return this->base::allocate(n, hint);
    }

    void deallocate(pointer p, size_type n) {
        g_memory_tracking_allocated -= n * sizeof(T);
        this->base::deallocate(p, n);
    }
};

#endif

