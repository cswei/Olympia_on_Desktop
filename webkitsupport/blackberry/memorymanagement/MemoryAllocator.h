/*
 * Copyright (C) 2008 Torch Mobile, Inc. All rights reserved.
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */

#ifndef OlympiaMemoryAllocator_h
#define OlympiaMemoryAllocator_h

namespace Olympia {
namespace Platform {

    class MemoryAllocator
    {
    public:
        static void initialize(void* heap, unsigned size);
        static void* m_malloc(unsigned size);
        static void* m_calloc(unsigned num, unsigned size);
        static void* m_realloc(void* p, unsigned size);
        static void m_free(void*);
        static void* m_memalign(unsigned alignment, unsigned size);
        static bool resizeMemory(void* p, unsigned newSize);
    };

} // namespace Platform
} //namespace Olympia

#endif // OlympiaMemoryAllocator_h
