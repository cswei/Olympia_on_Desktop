/*
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */

#ifndef OlympiaPlatformSettings_h
#define OlympiaPlatformSettings_h

#include <string>

namespace Olympia {
    namespace Platform {
        class HttpStreamDebugger;

        class Settings {
        private:
            Settings();
            ~Settings() {};

        public:

            static Settings* get();

            bool isDebugFilteringEnabled() const { return m_debugFiltering; }
            void setDebugFilteringEnabled(bool enable) { m_debugFiltering = enable; }

            bool isRSSFilteringEnabled() const { return m_rssFiltering; }
            void setRSSFilteringEnabled(bool enable) { m_rssFiltering = enable; }

            HttpStreamDebugger* streamDebugger() const { return m_streamDebugger; }
            void setStreamDebugger(HttpStreamDebugger* debugger) { m_streamDebugger = debugger; }

            unsigned getSuggestedCacheCapacity(unsigned currentUsage) const;

            const std::string& localUserSpecificStorageDirectory() const { return m_localUserStorageFolder; }

            const std::string& databaseDirectory() const { return m_databaseFolder; }

            unsigned long secondaryThreadStackSize() const { return m_secondaryThreadStackSize; }
            void setSecondaryThreadStackSize(unsigned long size) { m_secondaryThreadStackSize = size; }

            unsigned maxPixelsPerDecodedImage() const { return m_maxPixelsPerDecodedImage; }

            bool shouldReportLowMemoryToUser() const { return m_shouldReportLowMemoryToUser; }
            void setShouldReportLowMemoryToUser(bool shouldReport) { m_shouldReportLowMemoryToUser = shouldReport; }

        private:
            HttpStreamDebugger* m_streamDebugger;
            std::string m_localUserStorageFolder;
            std::string m_databaseFolder;

            unsigned long m_secondaryThreadStackSize;
            unsigned m_maxPixelsPerDecodedImage;

            bool m_debugFiltering;
            bool m_rssFiltering;

            bool m_shouldReportLowMemoryToUser;
        };
    } // namespace Platform
} // namespace Olympia

#endif // OlympiaPlatformSettings_h
