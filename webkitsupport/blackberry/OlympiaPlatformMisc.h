/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef OlympiaPlatformMisc_h
#define OlympiaPlatformMisc_h

#include "OlympiaPlatformPrimitives.h"

#include <stdarg.h>
#include <string>
#include <stdint.h>

#ifndef BRIDGE_IMPORT
#ifdef _MSC_VER
#define BRIDGE_IMPORT __declspec(dllimport)
#else
#define BRIDGE_IMPORT
#endif
#endif

namespace Olympia {
    namespace Platform {
        enum CursorType {
            CursorNone = 0,
            CursorAlias,
            CursorPointer,
            CursorCell,
            CursorCopy,
            CursorCross,
            CursorHand,
            CursorMove,
            CursorBeam,
            CursorWait,
            CursorHelp,
            CursorEastResize,
            CursorNorthResize,
            CursorNorthEastResize,
            CursorNorthWestResize,
            CursorSouthResize,
            CursorSouthEastResize,
            CursorSouthWestResize,
            CursorWestResize,
            CursorNorthSouthResize,
            CursorEastWestResize,
            CursorNorthEastSouthWestResize,
            CursorNorthWestSouthEastResize,
            CursorColumnResize,
            CursorRowResize,
            CursorVerticalText,
            CursorContextMenu,
            CursorNoDrop,
            CursorNotAllowed,
            CursorProgress,
            CursorZoomIn,
            CursorZoomOut,
            CursorCustomized,
            NumCursorTypes
        };

        struct OlympiaCursor {
            OlympiaCursor(const CursorType type = CursorNone, const std::string& url = std::string(), const IntPoint& point = IntPoint(0, 0)) : m_type(type), m_url(url), m_hotspot(point) {}

            CursorType type() const { return m_type; }
            std::string url() const { return m_url; }
            IntPoint hotspot() const { return m_hotspot; }

            OlympiaCursor& operator=(const OlympiaCursor& other)
            {
                m_type = other.type();
                m_url = other.url();
                m_hotspot = other.hotspot();
                return *this;
            }

            CursorType m_type;
            std::string m_url;
            IntPoint m_hotspot;
        };

        enum DebuggerMessageType {
            MessageDebug = 0,
            MessageStatus,
        };
        
        enum MessageLogLevel {
            LogLevelCritical = 0,
            LogLevelWarn,
            LogLevelInfo
        };

        // Return current UTC time in seconds
        double currentUTCTimeMS();

        double tickCount();

        // This logs events to wBugDisp.
        void logV(MessageLogLevel severity, const char* format, va_list);

        // This logs events to wBugDisp.
        void log(MessageLogLevel severity, const char* format, ...);

        unsigned int debugSetting();

        void* stackBase();
        void addStackBase(void* base);
        void removeStackBase();

        void willRunEventLoop();
        void processNextEvent();

        BRIDGE_IMPORT void sendMessageToJavaScriptDebugger(DebuggerMessageType type, const char* message, unsigned messageLength);

        void scheduleLazyInitialization();

        void scheduleCallOnMainThread(void(*callback)(void));

        // This logs events to on-device log file through Java EventLogger.
        void logEvent(MessageLogLevel level, const char* format, ...);

        const char* environment(const char* key);

#ifdef _MSC_VER
        // FIXME: on device, environment is just a wrapper around getenv.  On
        // simulator, we need to add strings to the environment by hand.
        // see http://bugzilla-torch.rim.net/show_bug.cgi?id=707; remove this function when it is fixed
        void addToEnvironment(const char* key, const char* value);
#endif

    } // Platform

} // Olympia

void getlocaltime(uint64_t* utc, uint64_t* local, int* isDst);


#endif // OlympiaPlatformMisc_h
