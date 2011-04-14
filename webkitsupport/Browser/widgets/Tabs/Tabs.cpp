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

#include "Tabs.h"
#include "TabbedView.h"

#include <QApplication>
#include <QDebug>

namespace Olympia {
namespace Browser {

static const QEvent::Type TRIGGER_EVENT = static_cast<QEvent::Type> (5001);
static const int REPAINT_THRESHOLD = 1000; // Unit ms

Tabs::Tabs(QWidget* w)
    : QWidget(w)
    , m_oldWidget(0)
{
    connect(&m_repaintTimer, SIGNAL(timeout()),
        this, SLOT(slotCustomEventPostedPeriodically()));

    // repaint timer triggered 36 times in every second.
    // we still got the cpu heavily-consuming issue when loading www.sina.com.cn, when we make the interval short.
    // need further work here.
    m_repaintTimer.start(27);
}

Tabs::~Tabs()
{

}

void Tabs::slotCustomEventPostedPeriodically()
{
    WebView* view = static_cast<WebView*>(getCurrentWidget());
    QApplication::removePostedEvents(this, TRIGGER_EVENT);
    if (m_repaintInterval.elapsed() >= REPAINT_THRESHOLD) {
        if (view && view->hasIdleJobs())
            view->triggerRender();
        m_repaintInterval.restart();
    }
    else
        QApplication::postEvent(this, new QEvent(TRIGGER_EVENT), Qt::LowEventPriority);
}

void Tabs::customEvent(QEvent* e)
{
    QWidget::customEvent(e);
    if (e->type() == TRIGGER_EVENT) {
        WebView* view = static_cast<WebView*>(getCurrentWidget());
        if (view && view->hasIdleJobs())
            view->triggerRender();
        m_repaintInterval.restart();
    }
}

void Tabs::orientationChanged()
{
    emit sigOrientationChanged();
}

void Tabs::windowSizeChanged()
{
    emit sigWindowSizeChanged();
}

} // namespace Browser
} // namespace Olympia

