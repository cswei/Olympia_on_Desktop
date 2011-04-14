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

#ifndef WebViewEventHandler_h
#define WebViewEventHandler_h

#include <cmath>
#include <deque>
#include "OlympiaPlatformInputEvents.h"
#include <QDateTime>
#include <QObject>
#include <QPoint>
#include <QtGui/QPaintEvent>
#include <QtGui/QWidget>
#include <QTimer>

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace Olympia {
namespace Browser {

class WebView;

////////////////////////////////////////////////////////////////////////////
// temp class.
class TouchEventDispatcher : public QObject
{
    Q_OBJECT
public:
    TouchEventDispatcher();
    ~TouchEventDispatcher();
    void start(int);
    void cancel();
    bool isPending();
    void setTouchEvent(Olympia::Platform::TouchEvent);

signals:
    void sigTimeout(Olympia::Platform::TouchEvent te);

public slots:
    void fire();

private:
    Olympia::Platform::TouchEvent m_te;
    QTimer m_timer;
};



/////////////////////////////////////////////////////////////////////////////
// this is the interface class of webview event handler.
class EventHandler : public QObject
{
public:
    EventHandler(WebView* view);
    virtual ~EventHandler();

    virtual void terminateCurrentWork() = 0;

    virtual void mouseDoubleClickEvent(QMouseEvent* e) = 0;
    virtual void mousePressEvent(QMouseEvent* e) = 0;
    virtual void mouseReleaseEvent(QMouseEvent* e) = 0;
    virtual void mouseMoveEvent(QMouseEvent* e) = 0;
    virtual void wheelEvent(QWheelEvent* e) = 0;
    virtual void keyPressEvent(QKeyEvent* e) = 0;
    virtual void extraPaintJob() = 0;

protected:
    WebView* m_webView;
};


/////////////////////////////////////////////////////////////////////////////
// NormalEventHandler to handle  event in normal browse mode.
class NormalEventHandler : public EventHandler
{
public:
    NormalEventHandler(WebView* view);
    virtual ~NormalEventHandler();

    virtual void terminateCurrentWork() {}

    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void wheelEvent(QWheelEvent* e);
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void extraPaintJob();

private:
    QPoint m_lastTouchPoint;
    Olympia::Platform::TouchEvent::Type m_touchEventType;
    QTime m_lastMouseTime;
    QPoint m_lastMousePos;
    std::deque< std::pair<QPoint, QTime> > m_mouseTrack;
    TouchEventDispatcher m_touchPressDispatcher;
    TouchEventDispatcher m_touchReleaseDispatcher;
};


/////////////////////////////////////////////////////////////////////////////
// SelectionEventHandler to handle event in selection mode.
class SelectionEventHandler : public EventHandler
{
public:
    SelectionEventHandler(WebView* view);
    virtual ~SelectionEventHandler();

    
    virtual void terminateCurrentWork();

    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void wheelEvent(QWheelEvent* e);
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void extraPaintJob();

private:
    Olympia::Platform::IntPoint m_first;
    bool m_secondSelect;
};


/////////////////////////////////////////////////////////////////////////////
// MultiTouchEventHandler to handle event in multi-touch mode.
class MultiTouchEventHandler : public EventHandler
{
public:
    MultiTouchEventHandler(WebView* view);
    virtual ~MultiTouchEventHandler();

    virtual void terminateCurrentWork() {}

    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void wheelEvent(QWheelEvent* e);
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void extraPaintJob();
private:
    void multiTouchHandle();
    enum MultiTouchStage {
        None = 0,
        FirstMousePressed = 1,
        SecondMousePressed = 2
    };
    MultiTouchStage m_multiTouchStage;
    QPointF m_lastMousePos;
    QPainterPath* m_firstMousePath;
    QPainterPath* m_secondMousePath;
};


} // namespace Browser
} // namespace Olympia

#endif // WebViewEventHandler_h
