/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RunLoop_h
#define RunLoop_h

#include <memory>
#include <wtf/HashMap.h>
#include <wtf/ThreadSpecific.h>
#include <wtf/Threading.h>
#include <wtf/Vector.h>

class WorkItem;

class RunLoop {
public:
    // Must be called from the main thread.
    static void initializeMainRunLoop();

    static RunLoop* current();
    static RunLoop* main();

    void scheduleWork(std::auto_ptr<WorkItem>);
    
    static void run();
    void stop();

    class TimerBase {
        friend class RunLoop;
    public:
        TimerBase(RunLoop*);
        virtual ~TimerBase();

        void startRepeating(double repeatInterval) { start(repeatInterval, repeatInterval); }
        void startOneShot(double interval) { start(interval, 0); }

        void stop();
        bool isActive() const;

        virtual void fired() = 0;

    private:
        void start(double nextFireInterval, double repeatInterval);

        RunLoop* m_runLoop;

#if PLATFORM(WIN)
        static void timerFired(RunLoop*, uint64_t ID);
        uint64_t m_ID;
#elif PLATFORM(MAC)
        static void timerFired(CFRunLoopTimerRef, void*);
        CFRunLoopTimerRef m_timer;
#endif    
    };

    template <typename TimerFiredClass>
    class Timer : public TimerBase {
    public:
        typedef void (TimerFiredClass::*TimerFiredFunction)();

        Timer(RunLoop* runLoop, TimerFiredClass* o, TimerFiredFunction f)
            : TimerBase(runLoop)
            , m_object(o)
            , m_function(f)
        {
        }

    private:
        virtual void fired() { (m_object->*m_function)(); }

        TimerFiredClass* m_object;
        TimerFiredFunction m_function;
    };

private:
    friend class WTF::ThreadSpecific<RunLoop>;

    RunLoop();
    ~RunLoop();
    
    void performWork();
    void wakeUp();

    Mutex m_workItemQueueLock;
    Vector<WorkItem*> m_workItemQueue;

#if PLATFORM(WIN)
    static bool registerRunLoopMessageWindowClass();
    static LRESULT CALLBACK RunLoopWndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    HWND m_runLoopMessageWindow;

    typedef HashMap<uint64_t, TimerBase*> TimerMap;
    TimerMap m_activeTimers;
#elif PLATFORM(MAC)
    static void performWork(void*);
    CFRunLoopRef m_runLoop;
    CFRunLoopSourceRef m_runLoopSource;
#endif
};

#endif // RunLoop_h
