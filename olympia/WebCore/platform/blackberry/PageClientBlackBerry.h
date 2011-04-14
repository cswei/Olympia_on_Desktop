/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef PageClientBlackBerry_h
#define PageClientBlackBerry_h

#include "Cursor.h"

namespace Olympia {
namespace Platform {
    class HttpStreamDebugger;
    class NetworkStreamFactory;
}
}

class PageClientBlackBerry {
public:
    virtual void setCursor(WebCore::PlatformCursorHandle handle) = 0;
    virtual Olympia::Platform::NetworkStreamFactory* networkStreamFactory() = 0;
    virtual Olympia::Platform::HttpStreamDebugger* httpStreamDebugger() = 0;
    virtual bool runMessageLoopForJavaScript() = 0;
};

#endif
