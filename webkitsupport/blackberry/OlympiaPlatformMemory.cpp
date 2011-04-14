/*
 * Copyright (c) 2011, Torch Mobile (Beijing) Co. Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided that
 * the following conditions are met:
 *
 *  -- Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *  -- Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *  -- Neither the name of the Torch Mobile (Beijing) Co. Ltd. nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "OlympiaPlatformMemory.h"

#include <Collector.h>
#include <map>
#include <stdlib.h>
#if defined(OLYMPIA_LINUX)// || defined(OLYMPIA_MAC)
#include <sys/mman.h>
#elif defined(OLYMPIA_WINDOWS)
#include <windows.h>
#include <malloc.h>
#elif defined(OLYMPIA_MAC)
#include <mach/mach_init.h>
//#include <mach/mach_point.h>
#include <mach/task.h>
#include <mach/thread_act.h>
#include <mach/vm_map.h>
#endif
#include <unistd.h>
#include <OlympiaPlatformAssert.h>

using JSC::BLOCK_SIZE;
using JSC::BLOCK_OFFSET_MASK;

namespace Olympia {
namespace Platform {

#ifdef OLYMPIA_LINUX_USE_MMAP
static std::map<void*, int> s_blockSizes;
#endif

void initializeMemoryManagement()
{
    return;
}

unsigned pageSize()
{
#if defined(OLYMPIA_LINUX) || defined(OLYMPIA_MAC)
    return sysconf(_SC_PAGE_SIZE);
#elif defined(OLYMPIA_WINDOWS)
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    return system_info.dwPageSize;
#endif
}

void* allocateJSBlock(unsigned size)
{
    // See: JavaScriptCore/runtime/Collector.cpp
    OLYMPIA_ASSERT(size == BLOCK_SIZE);
#if defined(OLYMPIA_LINUX) //|| defined(OLYMPIA_MAC)
#if ENABLE(JSC_MULTIPLE_THREADS)
#error Need to initialize pagesize safely.
#endif
    static size_t pagesize = pageSize();

    size_t extra = 0;
    if (BLOCK_SIZE > pagesize)
        extra = BLOCK_SIZE - pagesize;

    void* mmapResult = mmap(NULL, BLOCK_SIZE + extra, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    uintptr_t address = reinterpret_cast<uintptr_t>(mmapResult);

    size_t adjust = 0;
    if ((address & BLOCK_OFFSET_MASK) != 0)
        adjust = BLOCK_SIZE - (address & BLOCK_OFFSET_MASK);

    if (adjust > 0)
        munmap(reinterpret_cast<char*>(address), adjust);

    if (adjust < extra)
        munmap(reinterpret_cast<char*>(address + adjust + BLOCK_SIZE), extra - adjust);

    address += adjust;
    return reinterpret_cast<void*>(address);
#elif defined(OLYMPIA_WINDOWS)
#if COMPILER(MINGW) && !COMPILER(MINGW64)
    void* address = __mingw_aligned_malloc(BLOCK_SIZE, BLOCK_SIZE);
#else
    void* address = _aligned_malloc(BLOCK_SIZE, BLOCK_SIZE);
#endif
    memset(address, 0, BLOCK_SIZE);
	return address;
#elif defined(OLYMPIA_MAC)
    vm_address_t address = 0;
    vm_map(current_task(), &address, BLOCK_SIZE, BLOCK_OFFSET_MASK, VM_FLAGS_ANYWHERE | VM_TAG_FOR_COLLECTOR_MEMORY, MEMORY_OBJECT_NULL, 0, FALSE, VM_PROT_DEFAULT, VM_PROT_DEFAULT, VM_INHERIT_DEFAULT);
    return reinterpret_cast<void*>(address);
#endif
}

void freeJSBlock(void* address)
{
#if defined(OLYMPIA_LINUX)// || defined(OLYMPIA_MAC)
    munmap(reinterpret_cast<char*>(address), BLOCK_SIZE);
#elif defined(OLYMPIA_WINDOWS)
#if COMPILER(MINGW) && !COMPILER(MINGW64)
    __mingw_aligned_free(block);
#else
    _aligned_free(address);
#endif
#elif defined(OLYMPIA_MAC)
    vm_deallocate(current_task(), reinterpret_cast<vm_address_t>(address), BLOCK_SIZE);
#endif
} 

void* reserveVirtualMemory(size_t& totalBytes)
{
#ifdef OLYMPIA_LINUX
#ifdef OLYMPIA_LINUX_USE_MMAP
    void* p = mmap(0, totalBytes, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
    if (p != MAP_FAILED)
        s_blockSizes[p] = totalBytes;
    return p;
#else
    return malloc(totalBytes);
#endif
#elif defined(OLYMPIA_WINDOWS)
    return VirtualAlloc(0, totalBytes, MEM_RESERVE, PAGE_READWRITE);
#elif defined(OLYMPIA_MAC)
    return malloc(totalBytes);
#else
    OLYMPIA_CRASH();
#endif
}

void releaseVirtualMemory(void* address)
{
#ifdef OLYMPIA_LINUX
#ifdef OLYMPIA_LINUX_USE_MMAP
    munmap(address, s_blockSizes[address]);
    s_blockSizes.erase(address);
#else
    free(address);
#endif
#elif defined(OLYMPIA_WINDOWS)
    VirtualFree(address, 0, MEM_RELEASE);
#elif defined(OLYMPIA_MAC)
    return free(address);
#else
    OLYMPIA_CRASH();
#endif
}

void* commitVirtualMemory(void* address, unsigned totalBytes)
{
#ifdef OLYMPIA_LINUX
#if defined(OLYMPIA_LINUX_USE_MMAP) && defined(__USE_BSD)
    if (!madvise(address, totalBytes, MADV_WILLNEED))
        return NULL;
#else
    return address;
#endif
#elif defined(OLYMPIA_WINDOWS)
    return VirtualAlloc(address, totalBytes, MEM_COMMIT, PAGE_READWRITE);
#elif defined(OLYMPIA_MAC)
    return address;
#else
    OLYMPIA_CRASH();
#endif
}

void decommitVirtualMemory(void* address, unsigned totalBytes)
{
#if defined(OLYMPIA_LINUX_USE_MMAP) && defined(__USE_BSD)
    madvise(address, totalBytes, MADV_DONTNEED);
#endif
    return;
}

} // namespace Platform
} // namespace Olympia

// At present, these functions are never called:
void olympiaDebugInitialize()
{
    OLYMPIA_CRASH();
    return;
}

void olympiaDebugDidAllocate(void*, unsigned)
{
    OLYMPIA_CRASH();
    return;
}

void olympiaDebugDidFree(void*)
{
    OLYMPIA_CRASH();
    return;
}
