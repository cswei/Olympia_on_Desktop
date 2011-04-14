/*
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */

#include "config.h"
#include "CacheClientBlackBerry.h"

#include "OlympiaPlatformMisc.h"
#include "OlympiaPlatformSettings.h"

namespace WebCore {

CacheClientBlackBerry* CacheClientBlackBerry::get()
{
    static CacheClientBlackBerry s_cacheClient;
    return &s_cacheClient;
}

CacheClientBlackBerry::CacheClientBlackBerry()
    : m_lastCapacity(0)
{
}

void CacheClientBlackBerry::initialize()
{
#if ENABLE(OLYMPIA_DEBUG_MEMORY)
    bool isDisabled = true;
#else
    const char* enableSegregatedMemoryCache = Olympia::Platform::environment("ENABLE_SEGREGATED_MEMORY_CACHE");
    // FIXME: segregated memory cache is not implemented yet; for now we will
    // turn off caching entirely if it is requested
    bool isDisabled = enableSegregatedMemoryCache && !strcmp(enableSegregatedMemoryCache, "1");
#endif
    cache()->setDisabled(isDisabled);
    if (!isDisabled) {
        cache()->setClient(this);
        // We have to set a non-zero interval to schedule cache pruning after a CachedImage becoming dead
        cache()->setDeadDecodedDataDeletionInterval(0.01);
        updateCacheCapacity();
    }
}

void CacheClientBlackBerry::updateCacheCapacity()
{
#if ENABLE(OLYMPIA_DEBUG_MEMORY)
    // We're debugging memory usage. So keep it disabled
#else
    unsigned cacheCapacity = Olympia::Platform::Settings::get()->getSuggestedCacheCapacity(cache()->totalSize());
    if (m_lastCapacity == cacheCapacity) {
        // Suggested capacity hasn't been changed
        return;
    }

    m_lastCapacity = cacheCapacity;
    cache()->setCapacities(0, m_lastCapacity, m_lastCapacity);
#endif
}

void CacheClientBlackBerry::didAddToLiveResourcesSize(CachedResource*)
{
    // Check suggested cache capacity at this point
    updateCacheCapacity();
}

void CacheClientBlackBerry::didRemoveFromLiveResourcesSize(CachedResource*)
{
    // Check suggested cache capacity at this point
    updateCacheCapacity();
}

} // WebCore
