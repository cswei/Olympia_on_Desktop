/*
 * Copyright (C) 2009-2010 Research In Motion Limited. http://www.rim.com/
 */

#include "UnsharedPointer.h"

namespace Olympia {
namespace Platform {

struct Client {
    virtual void reportOutOfMemory() = 0;
    virtual void reportLowMemory() = 0;
    virtual bool getCookieString(int playerId, const char* url, UnsharedArray<char>& result) = 0;
    virtual bool setCookieString(int playerId, const char* url, const char* cookieString) = 0;
    virtual bool createGeoTracker(int token, bool highAccuracy, int timeout, int maxAge) = 0;
    virtual void destroyGeoTracker(int handle) = 0;
    virtual void suspendGeoTracker(int handle) = 0;
    virtual void resumeGeoTracker(int handle) = 0;
    virtual void willRunNestedEventLoop() = 0;
    virtual void processNextMessage() = 0;
    virtual void scheduleCallOnMainThread(void(*callback)(void)) = 0;
    virtual void logEvent(unsigned int level, const char* message) = 0;

    static Client* get();
    static void set(Client*);
};

} // Platform
} // Olympia
