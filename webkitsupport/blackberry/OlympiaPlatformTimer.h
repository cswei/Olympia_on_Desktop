/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef OlympiaPlatformTimer_h
#define OlympiaPlatformTimer_h

namespace Olympia {
namespace Platform {

    class TimerClient {
    public:
        virtual bool willFireTimer() = 0;
    };

    typedef void (*TimerFunction)();

    void timerStart(double interval, TimerFunction function);
    void timerStop();
    void setTimerClient(TimerClient*);

} // namespace Olympia
} // namespace Platform

#endif // OlympiaPlatformTimer_h
