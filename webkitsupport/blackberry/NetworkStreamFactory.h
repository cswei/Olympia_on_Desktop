/*
 * Copyright (C) 2009-2010 Research In Motion Limited. http://www.rim.com/
 */

#ifndef Olympia_Platform_NetworkStreamFactory_h
#define Olympia_Platform_NetworkStreamFactory_h

namespace Olympia {

    namespace Platform {

        class HttpStreamDebugger;
        class IStream;
        class NetworkRequest;

        class NetworkStreamFactory {
        public:

            virtual IStream* createNetworkStream(const NetworkRequest&, int playerId, HttpStreamDebugger* debugger = 0) = 0;
        };

    } // namespace Platform

} // namespace Olympia

#endif // Olympia_Platform_NetworkStreamFactory_h
