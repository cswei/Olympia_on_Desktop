/*
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

#ifndef VirtualMemoryManager_h
#define VirtualMemoryManager_h

#include <unistd.h>

namespace Olympia {
namespace Platform {

static const unsigned s_pageSize = PAGE_SIZE;

class VirtualMemoryManager {
public:
    static bool initialize(unsigned totalVirtualMemorySize, unsigned totalPhysicalMemoryCanCommit);
    static bool commit(void* address, unsigned size);
    static void decommit(void* address, unsigned size);
    static void* allocate(unsigned size, unsigned alignment = s_pageSize, bool forceRandomization = false);

    static unsigned freeCachedPages();

    static unsigned availablePhysicalMemory();
    static unsigned totalPhysicalMemory();
    static unsigned lowPhysicalMemoryThreshold();

    static void commitLock();
    static void commitUnlock();

    // Must call commitLock before calling the following 3 functions, and call commitUnlock after.
    static bool canCommit(unsigned size);
    static void didCommit(unsigned size);
    static void didDecommit(unsigned size);

    struct CommitLocker {
        CommitLocker () { commitLock(); }
        ~CommitLocker () { commitUnlock(); }
    };
};

} // namespace Platform
} // namespace Olympia

#endif // VirtualMemoryManager_h
