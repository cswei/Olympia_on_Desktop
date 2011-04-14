/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef OlympiaPlatformGeoTracker_h
#define OlympiaPlatformGeoTracker_h

#include "OlympiaPlatformGeoTrackerListener.h"

namespace Olympia {
namespace Platform {

    class GeoTracker {
    public:
        static GeoTracker* create(GeoTrackerListener* listener, bool highAccuracy, int timeout, int maxAge);

        virtual void destroy() = 0;
        virtual void suspend() = 0;
        virtual void resume() = 0;

    protected:
        virtual ~GeoTracker();
    };

} // namespace Olympia
} // namespace Platform

#endif // OlympiaPlatformGeoTracker_h
