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
#include "WebViewEventHandler.h"

#include "OlympiaPlatformKeyboardCodes.h"
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QtGui/QPainter>
#include <QWheelEvent>
#include "WebPage.h"
#include "WebViewQt.h"

#if COMPILER(MSVC)
#define DEBUG_PRINT(fmt, args,...) fprintf(stderr, fmt, ##args)
#else
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, fmt, ##args)
#endif

namespace Olympia {
namespace Browser {

// Unit: ms
const static int MeasureInterval = 500;
const static int AnimationInteval = 50;
const static int MaxTrackPointSize = 100;
const static int MouseMoveThreshold = 5;
const static int MuiltiTouchRoamInteval = 50;

using namespace Olympia::Platform;

EventHandler::EventHandler(WebView* view)
    : QObject(view)
    , m_webView(view)
{
    Q_ASSERT(m_webView);
}

EventHandler::~EventHandler()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/// normal browse event handler's implementations.
NormalEventHandler::NormalEventHandler(WebView* view)
    : EventHandler(view)
{
    connect(&m_touchPressDispatcher, SIGNAL(sigTimeout(Olympia::Platform::TouchEvent)),
        m_webView, SLOT(dispatchTouchEvent(Olympia::Platform::TouchEvent)));
    connect(&m_touchReleaseDispatcher, SIGNAL(sigTimeout(Olympia::Platform::TouchEvent)),
        m_webView, SLOT(dispatchTouchEvent(Olympia::Platform::TouchEvent)));
}

NormalEventHandler::~NormalEventHandler()
{
    m_touchPressDispatcher.cancel();
    m_touchPressDispatcher.disconnect(SIGNAL(timeout(TouchEvent)));
    m_touchReleaseDispatcher.cancel();
    m_touchReleaseDispatcher.disconnect(SIGNAL(timeout(TouchEvent)));
}

void NormalEventHandler::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (!m_webView->m_page)
        return;

    m_webView->m_page->blockZoom(e->x(), e->y());
    m_touchEventType = TouchEvent::TouchCancel;
}

void NormalEventHandler::mousePressEvent(QMouseEvent* e)
{
    if (!m_webView->m_page)
        return;

    WebDOMNode node = m_webView->m_page->nodeAtPoint(e->x(), e->y());

    m_lastTouchPoint = e->pos();
    m_webView->m_scrollTimer.stop();
    m_lastMousePos = e->pos();
    m_mouseTrack.clear();

    TouchPoint tp;
    tp.m_state = TouchPoint::TouchPressed;
    tp.m_screenPos = IntPoint(e->x(), e->y());
    tp.m_pos = tp.m_screenPos;

    TouchEvent te;
    te.m_type = TouchEvent::TouchStart;
    m_touchEventType = TouchEvent::TouchStart;
    te.m_singleType = TouchEvent::SinglePressed;
    te.m_points.push_back(tp);
    m_webView->m_page->touchEvent(te);
}

void NormalEventHandler::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_webView->m_page)
        return;

    // Handle inertial scrolling
    if (m_mouseTrack.size() > 1) {
        QTime now = m_mouseTrack[m_mouseTrack.size() - 1].second;
        int i;
        for (i = m_mouseTrack.size() - 1; i > 0; i--)
            if (m_mouseTrack[i].second.msecsTo(now) > MeasureInterval)
                break;

        // m_trackTimes[i] is first one which larger than MeasureInterval ms to now, or the last on in the vector.
        int elapsed = m_mouseTrack[i].second.msecsTo(now);
        if (elapsed) {
            QPoint delta = m_mouseTrack[i].first - e->pos();
            m_webView->m_nextScrollStepLength = QPoint(delta.x() * AnimationInteval / elapsed, delta.y() * AnimationInteval / elapsed);
            m_webView->m_scrollTimer.start(AnimationInteval);
        }
    }

    // Handle normal mouse release event:

    // Send touch event:
    m_lastTouchPoint = e->pos();
    TouchPoint tp;
    tp.m_state = TouchPoint::TouchReleased;
    tp.m_screenPos = IntPoint(e->x(), e->y());
    tp.m_pos = tp.m_screenPos;

    TouchEvent te;
    if (m_touchEventType == TouchEvent::TouchStart)
        m_touchEventType = TouchEvent::TouchEnd;
    te.m_type = m_touchEventType;
    te.m_singleType = TouchEvent::SingleReleased;
    te.m_points.push_back(tp);
    m_touchReleaseDispatcher.setTouchEvent(te);
    m_touchReleaseDispatcher.start(QApplication::doubleClickInterval());
}

void NormalEventHandler::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_webView->m_page)
        return;

    static const int treatAsZeroDistance = 4;
    if (e->buttons() == Qt::LeftButton) {
        QPoint newPoint = m_webView->validateScrollPoint(m_webView->m_lastContentPos + m_lastMousePos - e->pos());
        m_webView->m_lastContentPos = newPoint;
        m_lastMousePos = e->pos();
        if (m_mouseTrack.size() > MaxTrackPointSize) {
            m_mouseTrack.pop_front();
        }
        m_mouseTrack.push_back(std::make_pair(e->pos(), QTime::currentTime()));

        QPoint delta = e->pos() - m_lastTouchPoint;
        if (delta.manhattanLength() <= treatAsZeroDistance)
            return;

        m_lastTouchPoint = e->pos();
        m_webView->scrollBy(delta.x(), delta.y());

        TouchPoint tp;
        tp.m_state = TouchPoint::TouchMoved;
        tp.m_screenPos = IntPoint(e->x(), e->y());
        tp.m_pos = tp.m_screenPos;

        TouchEvent te;
        te.m_type = TouchEvent::TouchMove;
        te.m_singleType = TouchEvent::SingleMoved;
        te.m_points.push_back(tp);
        m_webView->m_page->touchEvent(te);
        m_touchEventType = TouchEvent::TouchCancel;
    }

    // FIXME: Should we handle mouse move event, this is common on PC, but useless on a touch screen device.
    // m_page->mouseEvent(Olympia::WebKit::MouseEventMoved, Olympia::Platform::IntPoint(event->x(), event->y()));
}

void NormalEventHandler::wheelEvent(QWheelEvent* e)
{
    if (!m_webView->m_page)
        return;

    if (e->modifiers() == Qt::ControlModifier) {
        e->delta() > 0 ? m_webView->zoomIn() : m_webView->zoomOut();
        return;
    }

    // FIXME: this adjusting is hardcoded.
    const static int granularity = 2;
    if (e->modifiers() == Qt::ShiftModifier) // defined as horizontal scrolling when shift key was pressed.
        m_webView->scrollBy(e->delta() / granularity, 0);
    else
        m_webView->scrollBy(0, e->delta() / granularity);
}

void NormalEventHandler::keyPressEvent(QKeyEvent* e)
{
    if (!m_webView->m_page)
        return;

    QString text = e->text();
    KeyboardEvent::Type type = KeyboardEvent::KeyDown;
    bool isShift = e->modifiers() == Qt::ShiftModifier;
    if (!text.isEmpty())
        m_webView->m_page->keyEvent(type, text.unicode()->unicode(), isShift);
    else {
        switch (e->key()) {
        case Qt::Key_Up:
            m_webView->m_page->navigationMoveEvent(KEY_CONTROL_UP, isShift);
            break;
        case Qt::Key_Down:
            m_webView->m_page->navigationMoveEvent(KEY_CONTROL_DOWN, isShift);
            break;
        case Qt::Key_Right:
            if (e->modifiers() == Qt::AltModifier)
                m_webView->m_page->goBackOrForward(1);
            m_webView->m_page->navigationMoveEvent(KEY_CONTROL_RIGHT, isShift);
            break;
        case Qt::Key_Left:
            if (e->modifiers() == Qt::AltModifier)
                m_webView->m_page->goBackOrForward(-1);
            m_webView->m_page->navigationMoveEvent(KEY_CONTROL_LEFT, isShift);
            break;
        default:
            m_webView->m_page->keyEvent(type, 0, isShift);
        }
    }
    //DEBUG_PRINT("\nWebView::keyPressEvent character=%x shift=%d\n", text.isEmpty() ? 0 : text.unicode()->unicode(), isShift);
}

void NormalEventHandler::extraPaintJob()
{
}


/////////////////////////////////////////////////////////////////////////////////////
/// selection event handler's implementations.
SelectionEventHandler::SelectionEventHandler(WebView* view)
    : EventHandler(view)
    , m_secondSelect(false)
{
}

SelectionEventHandler::~SelectionEventHandler()
{
}

void SelectionEventHandler::terminateCurrentWork()
{
    if (m_webView->m_page)
        m_webView->m_page->selectionCancelled();
}

void SelectionEventHandler::mouseDoubleClickEvent(QMouseEvent* e)
{
}

void SelectionEventHandler::mousePressEvent(QMouseEvent* e)
{
    if (!m_webView->m_page)
        return;

    IntPoint p(e->x(), e->y());
    if (!m_secondSelect) {
        m_first = p;
        m_webView->m_page->selectAtPoint(p);
    } else
        m_webView->m_page->setSelection(m_first, p);
    m_secondSelect = !m_secondSelect;
}

void SelectionEventHandler::mouseReleaseEvent(QMouseEvent* e)
{
}

void SelectionEventHandler::mouseMoveEvent(QMouseEvent* e)
{
}

void SelectionEventHandler::wheelEvent(QWheelEvent* e)
{
}

void SelectionEventHandler::keyPressEvent(QKeyEvent* e)
{
}

void SelectionEventHandler::extraPaintJob()
{
}


/////////////////////////////////////////////////////////////////////////////////////
/// multi-touch event handler's implementations.
MultiTouchEventHandler::MultiTouchEventHandler(WebView* view)
    : EventHandler(view)
    , m_firstMousePath(0)
    , m_secondMousePath(0)
{
}

MultiTouchEventHandler::~MultiTouchEventHandler()
{
    if (m_firstMousePath) {
        delete m_firstMousePath;
        m_firstMousePath = 0;
    }
    if (m_secondMousePath) {
        delete m_secondMousePath;
        m_secondMousePath = 0;
    }
}

void MultiTouchEventHandler::mouseDoubleClickEvent(QMouseEvent* e)
{
    m_multiTouchStage = None;
    m_webView->unsetMultiTouchMode();
}

void MultiTouchEventHandler::mousePressEvent(QMouseEvent* e)
{
    m_lastMousePos = e->pos();

    if (m_multiTouchStage == SecondMousePressed) {
        if (!m_secondMousePath)
            m_secondMousePath = new QPainterPath();
    }
    else {
        if (!m_firstMousePath)
            m_firstMousePath = new QPainterPath();
        m_multiTouchStage = FirstMousePressed;
    }
    m_webView->repaint(QRegion(m_lastMousePos.x(), m_lastMousePos.y(), 5, 5, QRegion::Ellipse));
}

void MultiTouchEventHandler::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_multiTouchStage == FirstMousePressed) {
        m_multiTouchStage = SecondMousePressed;
    }
    else {
        m_multiTouchStage = None;
        multiTouchHandle();
        m_webView->unsetMultiTouchMode();
    }
}

void MultiTouchEventHandler::mouseMoveEvent(QMouseEvent* e)
{
    m_lastMousePos = e->pos();
    m_webView->repaint(QRegion(m_lastMousePos.x(), m_lastMousePos.y(), 5, 5, QRegion::Ellipse));
}

void MultiTouchEventHandler::wheelEvent(QWheelEvent* e)
{
}

void MultiTouchEventHandler::keyPressEvent(QKeyEvent* e)
{
}

void MultiTouchEventHandler::extraPaintJob()
{
    QPainter paint(m_webView);
    QColor firstMouseColor(255, 0, 0, 128);
    QColor secondMouseColor(128, 0, 0, 128);

    if ((m_multiTouchStage == SecondMousePressed) && m_secondMousePath)
        m_secondMousePath->addEllipse(m_lastMousePos.x(), m_lastMousePos.y(), 5, 5);
    if ((m_multiTouchStage == FirstMousePressed) && m_firstMousePath)
        m_firstMousePath->addEllipse(m_lastMousePos.x(), m_lastMousePos.y(), 5, 5);

    if (m_firstMousePath && m_multiTouchStage != None) {
        paint.setPen(firstMouseColor);
        paint.setBrush(firstMouseColor);
        paint.drawPath(*m_firstMousePath);
    }
    if (m_secondMousePath && m_multiTouchStage != None) {
        paint.setPen(secondMouseColor);
        paint.setBrush(secondMouseColor);
        paint.drawPath(*m_secondMousePath);
    }
}

void MultiTouchEventHandler::multiTouchHandle()
{
    if (!m_webView || !m_webView->m_page)
        return;
    QPointF firstMouseBeginPos;
    QPointF firstMouseEndPos;
    QPointF secondMouseBeginPos;
    QPointF secondMouseEndPos;
    qreal beginPercent = 0;
    qreal endPercent = 0;
    Platform::IntPoint p;
    int MaxPointCount = m_firstMousePath->length() > m_secondMousePath->length() ? m_firstMousePath->length() : m_secondMousePath->length();

    for (int i = MuiltiTouchRoamInteval; i < MaxPointCount + 1; i = i + MuiltiTouchRoamInteval) {
        beginPercent = (i - MuiltiTouchRoamInteval) / static_cast<qreal>(MaxPointCount);
        endPercent = i / static_cast<qreal>(MaxPointCount);
        firstMouseBeginPos = m_firstMousePath->pointAtPercent(beginPercent);
        firstMouseEndPos = m_firstMousePath->pointAtPercent(endPercent);
        secondMouseBeginPos = m_secondMousePath->pointAtPercent(beginPercent);
        secondMouseEndPos = m_secondMousePath->pointAtPercent(endPercent);
        p = m_webView->m_page->scrollPosition();
        Platform::IntPoint r((firstMouseBeginPos.x() + secondMouseBeginPos.x()) / 2, (firstMouseEndPos.y() + secondMouseEndPos.y()) / 2);
        double beginPosDistance = (firstMouseBeginPos.x() - secondMouseBeginPos.x()) * (firstMouseBeginPos.x() - secondMouseBeginPos.x())
                                   + (firstMouseBeginPos.y() - secondMouseBeginPos.y()) * (firstMouseBeginPos.y() - secondMouseBeginPos.y());
        double endPosDistance = (firstMouseEndPos.x() - secondMouseEndPos.x()) * (firstMouseEndPos.x() - secondMouseEndPos.x())
                                + (firstMouseEndPos.y() - secondMouseEndPos.y()) * (firstMouseEndPos.y() - secondMouseEndPos.y());
        double firstMouseMoveLen = sqrt(static_cast<double>(firstMouseBeginPos.x() - firstMouseEndPos.x()) * (firstMouseBeginPos.x() - firstMouseEndPos.x())
                                        + (firstMouseBeginPos.y() - firstMouseEndPos.y()) * (firstMouseBeginPos.y() - firstMouseEndPos.y()));
        double secondMouseMoveLen = sqrt(static_cast<double>(secondMouseBeginPos.x() - secondMouseEndPos.x()) * (secondMouseBeginPos.x() - secondMouseEndPos.x()) 
                                         +(secondMouseBeginPos.y() - secondMouseEndPos.y()) * (secondMouseBeginPos.y() - secondMouseEndPos.y()));
        int scaleLen = m_webView->width() / 2;
        double maxMouseMoveLen = firstMouseMoveLen > secondMouseMoveLen ? firstMouseMoveLen : secondMouseMoveLen;
        float scaleFactor = 0;
        bool zoomFlag = false;
        if (MaxPointCount < i + MuiltiTouchRoamInteval)
            zoomFlag = true;

        if (beginPosDistance > endPosDistance) {
            scaleFactor = 1 - maxMouseMoveLen / scaleLen * 0.75;
            if (scaleFactor * m_webView->m_page->currentZoomLevel() <= m_webView->m_page->minimumScale())
                zoomFlag = true;
            m_webView->m_page->bitmapZoom(p.x() + r.x(), p.y() + r.y(), scaleFactor, zoomFlag);
        }
        else {
            if (m_webView->m_page->currentZoomLevel() >= m_webView->m_page->maximumScale()) {
                m_webView->repaint();
                continue;
            }
            scaleFactor = (m_webView->m_page->currentZoomLevel() + maxMouseMoveLen * 3 / scaleLen ) / m_webView->m_page->currentZoomLevel();
            if (scaleFactor * m_webView->m_page->currentZoomLevel() >= m_webView->m_page->maximumScale())
                zoomFlag = true;
            m_webView->m_page->bitmapZoom(p.x() + r.x(), p.y() + r.y(), scaleFactor, zoomFlag);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////
/// temp class implementation.
TouchEventDispatcher::TouchEventDispatcher()
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(fire()));
}

TouchEventDispatcher::~TouchEventDispatcher()
{
    m_timer.stop();
}

void TouchEventDispatcher::setTouchEvent(Olympia::Platform::TouchEvent event)
{
    m_te = event;
}

void TouchEventDispatcher::start(int timeout)
{
    if (timeout < 0)
        return;
    m_timer.start(timeout);
}

void TouchEventDispatcher::cancel()
{
    if (isPending())
        m_timer.stop();
}

void TouchEventDispatcher::fire()
{
    m_timer.stop();
    emit sigTimeout(m_te);
}

bool TouchEventDispatcher::isPending()
{
    return m_timer.isActive();
}

} // namespace Browser
} // namespace Olympia
