/*
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

#ifndef OlympiaPlatformMemory_h
#define OlympiaPlatformMemory_h

#include <stddef.h>
#include <stdlib.h>
#ifdef __cplusplus
#include <new>
#else
#include <new.h>
#endif

#ifdef __cplusplus

namespace Olympia {
namespace Platform {

    void initializeMemoryManagement();
    unsigned pageSize();
    void* allocateJSBlock(unsigned size);
    void freeJSBlock(void*);
    void* reserveVirtualMemory(size_t& totalBytes);
    void releaseVirtualMemory(void*);
    void* commitVirtualMemory(void* address, unsigned totalBytes);
    void decommitVirtualMemory(void* address, unsigned totalBytes);

} // namespace Platform
} // namespace Olympia

void olympiaDebugInitialize();
void olympiaDebugDidAllocate(void*, unsigned);
void olympiaDebugDidFree(void*);

#endif // __cplusplus

#if defined(ENABLE_OLYMPIA_DEBUG_MEMORY) && ENABLE_OLYMPIA_DEBUG_MEMORY

// Must include this header before including openvg.h and egl.h
#define vgCreateMaskLayer olympiaVgCreateMaskLayer
#define vgCreatePath olympiaVgCreatePath
#define vgCreatePaint olympiaVgCreatePaint
#define vgCreateImage olympiaVgCreateImage
#define vgCreateFont olympiaVgCreateFont
#define vgDestroyMaskLayer olympiaVgDestroyMaskLayer
#define vgDestroyPath olympiaVgDestroyPath
#define vgDestroyPaint olympiaVgDestroyPaint
#define vgDestroyImage olympiaVgDestroyImage
#define vgDestroyFont olympiaVgDestroyFont
#define eglCreateWindowSurface olympiaEglCreateWindowSurface
#define eglCreateContext olympiaEglCreateContext
#define eglCreatePbufferSurface olympiaEglCreatePbufferSurface
#define eglCreatePixmapSurface olympiaEglCreatePixmapSurface
#define eglCreatePbufferFromClientBuffer olympiaEglCreatePbufferFromClientBuffer
#define eglDestroySurface olympiaEglDestroySurface
#define eglDestroyContext olympiaEglDestroyContext

#endif // defined(ENABLE_OLYMPIA_DEBUG_MEMORY) && ENABLE_OLYMPIA_DEBUG_MEMORY

// On Mac OS X, the VM subsystem allows tagging memory requested from mmap and vm_map
// in order to aid tools that inspect system memory use. 
#if defined(OLYMPIA_MAC)

#include <mach/vm_statistics.h>

#if !defined(TARGETING_TIGER)

#if defined(VM_MEMORY_TCMALLOC)
#define VM_TAG_FOR_TCMALLOC_MEMORY VM_MAKE_TAG(VM_MEMORY_TCMALLOC)
#else
#define VM_TAG_FOR_TCMALLOC_MEMORY VM_MAKE_TAG(53)
#endif // defined(VM_MEMORY_TCMALLOC)

#if defined(VM_MEMORY_JAVASCRIPT_JIT_EXECUTABLE_ALLOCATOR)
#define VM_TAG_FOR_EXECUTABLEALLOCATOR_MEMORY VM_MAKE_TAG(VM_MEMORY_JAVASCRIPT_JIT_EXECUTABLE_ALLOCATOR)
#else
#define VM_TAG_FOR_EXECUTABLEALLOCATOR_MEMORY VM_MAKE_TAG(64)
#endif // defined(VM_MEMORY_JAVASCRIPT_JIT_EXECUTABLE_ALLOCATOR)

#if defined(VM_MEMORY_JAVASCRIPT_JIT_REGISTER_FILE)
#define VM_TAG_FOR_REGISTERFILE_MEMORY VM_MAKE_TAG(VM_MEMORY_JAVASCRIPT_JIT_REGISTER_FILE)
#else
#define VM_TAG_FOR_REGISTERFILE_MEMORY VM_MAKE_TAG(65)
#endif // defined(VM_MEMORY_JAVASCRIPT_JIT_REGISTER_FILE)

#else // !defined(TARGETING_TIGER)

// mmap on Tiger fails with tags that work on Leopard, so fall
// back to Tiger-compatible tags (that also work on Leopard)
// when targeting Tiger.
#define VM_TAG_FOR_TCMALLOC_MEMORY -1
#define VM_TAG_FOR_EXECUTABLEALLOCATOR_MEMORY -1
#define VM_TAG_FOR_REGISTERFILE_MEMORY -1

#endif // !defined(TARGETING_TIGER)

// Tags for vm_map and vm_allocate work on both Tiger and Leopard.

#if defined(VM_MEMORY_JAVASCRIPT_CORE)
#define VM_TAG_FOR_COLLECTOR_MEMORY VM_MAKE_TAG(VM_MEMORY_JAVASCRIPT_CORE)
#else
#define VM_TAG_FOR_COLLECTOR_MEMORY VM_MAKE_TAG(63)
#endif // defined(VM_MEMORY_JAVASCRIPT_CORE)

#if defined(VM_MEMORY_WEBCORE_PURGEABLE_BUFFERS)
#define VM_TAG_FOR_WEBCORE_PURGEABLE_MEMORY VM_MAKE_TAG(VM_MEMORY_WEBCORE_PURGEABLE_BUFFERS)
#else
#define VM_TAG_FOR_WEBCORE_PURGEABLE_MEMORY VM_MAKE_TAG(69)
#endif // defined(VM_MEMORY_WEBCORE_PURGEABLE_BUFFERS)

#else // defined(OLYMPIA_MAC)

#define VM_TAG_FOR_TCMALLOC_MEMORY -1
#define VM_TAG_FOR_COLLECTOR_MEMORY -1
#define VM_TAG_FOR_EXECUTABLEALLOCATOR_MEMORY -1
#define VM_TAG_FOR_REGISTERFILE_MEMORY -1
#define VM_TAG_FOR_WEBCORE_PURGEABLE_MEMORY -1

#endif // defined(OLYMPIA_MAC)

#endif // OlympiaPlatformMemory_h
