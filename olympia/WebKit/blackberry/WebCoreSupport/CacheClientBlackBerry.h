/*
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */

#ifndef CacheClientBlackBerry_h
#define CacheClientBlackBerry_h

#include "Cache.h"

namespace WebCore {

class CacheClientBlackBerry: public CacheClient {
public:
    static CacheClientBlackBerry* get();

    void initialize();
    void updateCacheCapacity();

    // Derived from CacheClient
    virtual void didAddToLiveResourcesSize(CachedResource* resource);
    virtual void didRemoveFromLiveResourcesSize(CachedResource* resource);
private:
    CacheClientBlackBerry();
    unsigned m_lastCapacity;
};


} // WebCore

#endif // CacheClientBlackBerry_h
