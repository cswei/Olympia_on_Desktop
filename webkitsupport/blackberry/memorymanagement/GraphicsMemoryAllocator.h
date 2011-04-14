/*
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */

#ifndef Olympia_GraphicsMemoryAllocator_h
#define Olympia_GraphicsMemoryAllocator_h

namespace Olympia {
namespace Platform {

    class GraphicsMemoryAllocator {
    public:
        static void initialize();
        static void* allocatePageAligned(unsigned size);
        static void freePageAligned(void*);
    };

} // namespace Platform
} //namespace Olympia

#endif // Olympia_GraphicsMemoryAllocator_h
