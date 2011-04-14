/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef OlympiaPlatformCookieJar_h
#define OlympiaPlatformCookieJar_h

#include "UnsharedPointer.h"

namespace Olympia {
    namespace Platform {

        bool getCookieString(int playerId, const char* url, UnsharedArray<char>& result);
        bool setCookieString(int playerId, const char* url, const char* cookieString);

    } // namespace Olympia
} // namespace Platform

#endif // OlympiaPlatformCookieJar_h
