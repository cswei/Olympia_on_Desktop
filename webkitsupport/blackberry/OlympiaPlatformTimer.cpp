/*
 * Copyright (C) 2006 George Staikos <staikos@kde.org>
 * Copyright (C) 2006 Dirk Mueller <mueller@kde.org>
 * Copyright (C) 2008 Holger Hans Peter Freyther
 *
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */


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

#include <wtf/CurrentTime.h>

#include <QBasicTimer>
#include <QCoreApplication>
#include <QDebug>
#include <QPointer>

namespace Olympia {
namespace Platform {

class OlympiaPlatformTimerQt : public QObject {
    Q_OBJECT

private:
    OlympiaPlatformTimerQt();
    ~OlympiaPlatformTimerQt();
    QBasicTimer m_timer; 
    void (*m_timerFunction)();

public:
    static OlympiaPlatformTimerQt* get();
    void start(double fireTime, void (*f)());
    void stop();

private slots:
    void destroy();

protected:
    void timerEvent(QTimerEvent *event);
};

OlympiaPlatformTimerQt::OlympiaPlatformTimerQt()
    : QObject()
    , m_timerFunction(0)
{}

OlympiaPlatformTimerQt::~OlympiaPlatformTimerQt()
{
    if (m_timer.isActive())
        (m_timerFunction)();
}

OlympiaPlatformTimerQt* OlympiaPlatformTimerQt::get()
{
    static QPointer<OlympiaPlatformTimerQt> timer;
    if (!timer) {
        timer = new OlympiaPlatformTimerQt();
        timer->connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(destroy()));
    }
    return timer;
}

void OlympiaPlatformTimerQt::destroy()
{
    delete this;
}

void OlympiaPlatformTimerQt::start(double interval, void (*function)())
{
    unsigned int intervalInMS;
    if (interval < 0)
        intervalInMS = 0;
    else {
        interval *= 1000;
        intervalInMS = (unsigned int)interval;
    }
    m_timerFunction = function;
    m_timer.start(intervalInMS, this);
}

void OlympiaPlatformTimerQt::stop()
{
    m_timer.stop();
}

void OlympiaPlatformTimerQt::timerEvent(QTimerEvent* ev)
{
    if (!m_timerFunction || ev->timerId() != m_timer.timerId())
        return;

    m_timer.stop();
    (m_timerFunction)();
}

void timerStart(double interval, void (*f)())
{
     OlympiaPlatformTimerQt::get()->start(interval,f);
}

void timerStop()
{
      OlympiaPlatformTimerQt::get()->stop();
}
} // namespace Olympia
} // namespace Platform
#include "OlympiaPlatformTimer.moc"
