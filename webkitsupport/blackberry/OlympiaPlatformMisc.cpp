/*
 * Copyright (c) 2011, Torch Mobile (Beijing) Co. Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided that
 * the following conditions are met:
 *
 *  -- Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *  -- Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *  -- Neither the name of the Torch Mobile (Beijing) Co. Ltd. nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "OlympiaPlatformMisc.h"

#if defined(OLYMPIA_LINUX) || defined(OLYMPIA_MAC)
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#if HAVE(PTHREAD_NP_H)
#include <pthread_np.h>
#endif
#elif defined(OLYMPIA_WINDOWS)
#include <windows.h>
#include <sys/timeb.h>
#include <math.h>

#define msPerSecond 1000.0
//#elif defined(OLYMPIA_MAC)
//#include <mach/mach_init.h>
//#include <mach/mach_port.h>
//#include <mach/task.h>
//#include <mach/thread_act.h>
//#include <mach/vm_map.h>

//typedef mach_port_t PlatformThread;
#endif

#include "NotImplemented.h"
#include "OlympiaPlatformAssert.h"
#include "OlympiaPlatformPrimitives.h"
#include <QCoreApplication>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <sys/time.h>
#include <wtf/Threading.h>

using namespace std;

namespace Olympia {
namespace Platform {

// Return current UTC time in milliseconds
double currentUTCTimeMS()
{
    return tickCount() * 1000.0;
}


// See JavaScriptCore/wtf/CurrentTime.cpp : currenTime()
#if defined(OLYMPIA_LINUX) || defined(OLYMPIA_MAC)
double tickCount()
{
    struct timeval now;
    gettimeofday(&now, 0);
    return now.tv_sec + now.tv_usec / 1000000.0;
}
#elif defined(OLYMPIA_WINDOWS)

static LARGE_INTEGER qpcFrequency;
static bool syncedTime;

static double highResUpTime()
{
    // We use QPC, but only after sanity checking its result, due to bugs:
    // http://support.microsoft.com/kb/274323
    // http://support.microsoft.com/kb/895980
    // http://msdn.microsoft.com/en-us/library/ms644904.aspx ("...you can get different results on different processors due to bugs in the basic input/output system (BIOS) or the hardware abstraction layer (HAL)."

    static LARGE_INTEGER qpcLast;
    static DWORD tickCountLast;
    static bool inited;

    LARGE_INTEGER qpc;
    QueryPerformanceCounter(&qpc);
    DWORD tickCount = GetTickCount();

    if (inited) {
        __int64 qpcElapsed = ((qpc.QuadPart - qpcLast.QuadPart) * 1000) / qpcFrequency.QuadPart;
        __int64 tickCountElapsed;
        if (tickCount >= tickCountLast)
            tickCountElapsed = (tickCount - tickCountLast);
        else {
#if COMPILER(MINGW)
            __int64 tickCountLarge = tickCount + 0x100000000ULL;
#else
            __int64 tickCountLarge = tickCount + 0x100000000I64;
#endif
            tickCountElapsed = tickCountLarge - tickCountLast;
        }

        // force a re-sync if QueryPerformanceCounter differs from GetTickCount by more than 500ms.
        // (500ms value is from http://support.microsoft.com/kb/274323)
        __int64 diff = tickCountElapsed - qpcElapsed;
        if (diff > 500 || diff < -500)
            syncedTime = false;
    } else
        inited = true;

    qpcLast = qpc;
    tickCountLast = tickCount;

    return (1000.0 * qpc.QuadPart) / static_cast<double>(qpcFrequency.QuadPart);
}

static double lowResUTCTime()
{
#if OS(WINCE)
    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime);
    struct tm tmtime;
    tmtime.tm_year = systemTime.wYear - 1900;
    tmtime.tm_mon = systemTime.wMonth - 1;
    tmtime.tm_mday = systemTime.wDay;
    tmtime.tm_wday = systemTime.wDayOfWeek;
    tmtime.tm_hour = systemTime.wHour;
    tmtime.tm_min = systemTime.wMinute;
    tmtime.tm_sec = systemTime.wSecond;
    time_t timet = mktime(&tmtime);
    return timet * msPerSecond + systemTime.wMilliseconds;
#else
    struct _timeb timebuffer;
    _ftime(&timebuffer);
    return timebuffer.time * msPerSecond + timebuffer.millitm;
#endif
}

static bool qpcAvailable()
{
    static bool available;
    static bool checked;

    if (checked)
        return available;

    available = QueryPerformanceFrequency(&qpcFrequency);
    checked = true;
    return available;
}

double tickCount()
{
    // Use a combination of ftime and QueryPerformanceCounter.
    // ftime returns the information we want, but doesn't have sufficient resolution.
    // QueryPerformanceCounter has high resolution, but is only usable to measure time intervals.
    // To combine them, we call ftime and QueryPerformanceCounter initially. Later calls will use QueryPerformanceCounter
    // by itself, adding the delta to the saved ftime.  We periodically re-sync to correct for drift.
    static double syncLowResUTCTime;
    static double syncHighResUpTime;
    static double lastUTCTime;

    double lowResTime = lowResUTCTime();

    if (!qpcAvailable())
        return lowResTime / 1000.0;

    double highResTime = highResUpTime();

    if (!syncedTime) {
        timeBeginPeriod(1); // increase time resolution around low-res time getter
        syncLowResUTCTime = lowResTime = lowResUTCTime();
        timeEndPeriod(1); // restore time resolution
        syncHighResUpTime = highResTime;
        syncedTime = true;
    }

    double highResElapsed = highResTime - syncHighResUpTime;
    double utc = syncLowResUTCTime + highResElapsed;

    // force a clock re-sync if we've drifted
    double lowResElapsed = lowResTime - syncLowResUTCTime;
    const double maximumAllowedDriftMsec = 15.625 * 2.0; // 2x the typical low-res accuracy
    if (fabs(highResElapsed - lowResElapsed) > maximumAllowedDriftMsec)
        syncedTime = false;

    // make sure time doesn't run backwards (only correct if difference is < 2 seconds, since DST or clock changes could occur)
    const double backwardTimeLimit = 2000.0;
    if (utc < lastUTCTime && (lastUTCTime - utc) < backwardTimeLimit)
        return lastUTCTime / 1000.0;
    lastUTCTime = utc;
    return utc / 1000.0;
}
#endif

// This logs events to wBugDisp.
void logV(MessageLogLevel severity, const char* format, va_list va)
{
#ifdef OLYMPIA_MAC
#else
    // FIXME: The implementation here is too simple:
//fprintf(stderr, "\n*******format is :%s****\n", format);
//vprintf(format, va);
    //vfprintf(stderr, format, va);
    fprintf(stderr,"\n"); 
#endif
}

// This logs events to wBugDisp.
void log(MessageLogLevel severity, const char* format, ...)
{
    va_list va;
    va_start(va, format);
    // FIXME: The implementation here is too simple:
    vfprintf(stderr, format, va);
    fprintf(stderr,"\n"); 
    va_end(va);
}

unsigned int debugSetting()
{
    // FIXME: The implementation here is too simple:
    return 3;
}


// See JavaScriptCore/runtime/Collector.cpp : currentThreadStackBase().
void* stackBase()
{
#if defined(OLYMPIA_LINUX)// || defined(OLYMPIA_MAC)
    AtomicallyInitializedStatic(Mutex&, mutex = *new Mutex);
    MutexLocker locker(mutex);
    static void* stackBase = 0;
    static size_t stackSize = 0;
    static pthread_t stackThread;
    pthread_t thread = pthread_self();
    if (stackBase == 0 || thread != stackThread) {
        pthread_attr_t sattr;
        pthread_attr_init(&sattr);
#if HAVE(PTHREAD_NP_H) || OS(NETBSD)
        // e.g. on FreeBSD 5.4, neundorf@kde.org
        pthread_attr_get_np(thread, &sattr);
#else
        // FIXME: this function is non-portable; other POSIX systems may have different np alternatives
        pthread_getattr_np(thread, &sattr);
#endif
        int rc = pthread_attr_getstack(&sattr, &stackBase, &stackSize);
        (void)rc; // FIXME: Deal with error code somehow? Seems fatal.
        ASSERT(stackBase);
        pthread_attr_destroy(&sattr);
        stackThread = thread;
    }
    return static_cast<char*>(stackBase) + stackSize;
#elif defined(OLYMPIA_WINDOWS) && CPU(X86) && COMPILER(MSVC)
    // offset 0x18 from the FS segment register gives a pointer to
    // the thread information block for the current thread
    NT_TIB* pTib;
    __asm {
        MOV EAX, FS:[18h]
        MOV pTib, EAX
    }
    return static_cast<void*>(pTib->StackBase);
#elif defined(OLYMPIA_WINDOWS) && CPU(X86) && COMPILER(GCC)
    // offset 0x18 from the FS segment register gives a pointer to
    // the thread information block for the current thread
    NT_TIB* pTib;
    asm ( "movl %%fs:0x18, %0\n"
          : "=r" (pTib)
        );
    return static_cast<void*>(pTib->StackBase);
#elif defined(OLYMPIA_WINDOWS) && CPU(X86_64)
    PNT_TIB64 pTib = reinterpret_cast<PNT_TIB64>(NtCurrentTeb());
    return reinterpret_cast<void*>(pTib->StackBase);
#elif defined(OLYMPIA_MAC)
    pthread_t thread = pthread_self();
    return pthread_get_stackaddr_np(thread);
#endif
}



void addStackBase(void* /* base */)
{
    return;
}

void removeStackBase()
{
}

// See also: webkitsupport/olympia/OlympiaPlatformClient.h,
// Maybe we should move these to its implementation.
void willRunEventLoop()
{
#ifdef OLYMPIA_LINUX
#else
    notImplemented();
#endif
}

void processNextEvent()
{
    QCoreApplication::processEvents();
}

// This function is never called at present.
void sendMessageToJavaScriptDebugger(DebuggerMessageType type, const char* message, unsigned messageLength)
{
    OLYMPIA_CRASH();
}

// This function is never called at present.
void scheduleLazyInitialization()
{
    OLYMPIA_CRASH();
}

void scheduleCallOnMainThread(void(*callback)(void))
{
    notImplemented();
}

// This logs events to on-device log file through Java EventLogger.
void logEvent(MessageLogLevel level, const char* format, ...)
{
    va_list va;
    va_start(va, format);
    // FIXME: The implementation here is too simple:
    vfprintf(stderr, format, va);
    fprintf(stderr,"\n"); 
    va_end(va);
}

const char* environment(const char* key)
{
// #warning Please check these variables are set: ENABLE_SEGREGATED_MEMORY_CACHE, TILE_NUMBER in lauch script.
    return getenv(key);
}

#ifdef _MSC_VER
// FIXME: on device, environment is just a wrapper around getenv.  On
// simulator, we need to add strings to the environment by hand.
// see http://bugzilla-torch.rim.net/show_bug.cgi?id=707; remove this function when it is fixed
void addToEnvironment(const char* key, const char* value)
{
    // This function is never called at present.
    OLYMPIA_CRASH();
}
#endif

} // Platform
} // Olympia

void getlocaltime(uint64_t* utc, uint64_t* local, int* isDst)
{
    notImplemented();
}
