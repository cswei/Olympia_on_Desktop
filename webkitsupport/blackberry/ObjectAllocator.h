/*
 * Copyright (C) 2009 Torch Mobile Inc.
 * Copyright (C) Research In Motion, Limited 2009. All rights reserved.
 */

#ifndef ObjectAllocator_h
#define ObjectAllocator_h

namespace Olympia {
namespace Platform {

class ObjectAllocator
{
public:
    static ObjectAllocator* create(unsigned objectSize, int capacity);
    static unsigned collectAll();

    unsigned collectGarbage();
    void releaseObject(void* object);
    void* getObject();

    ~ObjectAllocator();

private:
    ObjectAllocator(unsigned objectSize, int capacity);
    void* popObject();
    void pushObject(void*);

    unsigned m_objectSize;
    int m_maxNumObjects;
    void* m_firstObject;
    unsigned m_numObjects;
};

void initializeObjectAllocators();

} // namespace Platform
} // namespace Olympia

#endif // ObjectAllocator_h
