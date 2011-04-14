/*
 * Copyright (C) 2008-2009 Torch Mobile Inc.
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

#ifndef MemoryManager_h
#define MemoryManager_h

namespace Olympia {
namespace Platform {

    class MemoryManager {
    public:
        MemoryManager();
        ~MemoryManager();

        bool allocationCanFail() const { return m_allocationCanFail; }
        void setAllocationCanFail(bool c) { m_allocationCanFail = c; }

        static void initialize();
        static void releaseAllMemory();

        static unsigned pageSize();
        static void* allocateJSBlock(unsigned size);
        static void freeJSBlock(void*);
        static void* reserveVirtualMemory(unsigned totalBytes);
        static void releaseVirtualMemory(void*);
        static void* commitVirtualMemory(void* address, unsigned totalBytes);
        static void decommitVirtualMemory(void* address, unsigned totalBytes);

    private:
        friend MemoryManager* memoryManager();

        bool m_allocationCanFail;
    };

    MemoryManager* memoryManager();

} // namespace Platform
} // namespace Olympia

#endif // MemoryManager_h
