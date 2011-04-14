/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef Olympia_Platform_RSSFilterStream_h
#define Olympia_Platform_RSSFilterStream_h

#include "SharedPointer.h"
#include <string>
#include "streams/FilterStream.h"
#include "streams/ResourceItem.h"


class FeedParser;
class RSSGenerator;

namespace Olympia {

namespace Platform {

        class ResourceItem;

        class RSSFilterStream : public FilterStream {
        public:
            enum ResourceType {
                TYPE_UNKNOWN = 0,
                TYPE_NOT_RSS,
                TYPE_RSS10_CONTENT,
                TYPE_RSS20_CONTENT,
                TYPE_ATOM_CONTENT
            };

            RSSFilterStream(int playerId);
            virtual ~RSSFilterStream();

            // IStreamListener
            virtual void notifyOpen(int status, const char* message);
            virtual void notifyHeaderReceived(const char* key, const char* value);
            virtual void notifyDataReceived(const char *buf, size_t len);
            virtual void notifyDone();

            // IStream
            virtual void setRequest(const NetworkRequest&);

        private:
            std::string convertResourceToHtml();
            bool createParser(const char* key, const char* value);
            void handleRssContent();
            void sendHeaders();

        private:
            int m_playerId;
            ResourceItem m_resourceItem;
            FeedParser* m_rssParser;
            RSSGenerator* m_rssGenerator;
            ResourceType m_resourceType;
        };

    } // namespace Olympia

} // namespace Platform

#endif // Olympia_Platform_RSSFilterStream_h
