/*
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */

#ifndef DlmallocWrapper_h
#define DlmallocWrapper_h

class DlmallocWrapper
{
public:
    typedef void* (*FuncMoreCore)(int size);

    // We reserve memory space for all dlmalloc instances' parameters in DlmallcWrapper.cpp
    // with a global array (in data segment). Increase s_numSlots when you need more dlmalloc
    // instances.
    // I know this is a little bit ugly, but I don't want to include all dlmalloc structs and macros
    // in this header file in order to let caller know the size required for a dlmalloc instance at
    // compile time.
    static const unsigned s_numSlots = 2;

    // Call this function to create a dlmalloc instance.It returns null when there's no more available
    // slot.
    static DlmallocWrapper* create(FuncMoreCore funcMoreCore);

    static void* moreCoreFailure();

    void* m_malloc(unsigned size);
    void* m_calloc(unsigned num, unsigned size);
    void* m_realloc(void* p, unsigned size);
    void m_free(void*);
    void* m_memalign(unsigned alignment, unsigned size);

private:
    DlmallocWrapper();
};

#endif // DlmallocWrapper_h
