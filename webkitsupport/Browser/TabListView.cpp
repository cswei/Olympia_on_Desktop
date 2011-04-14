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

#include "TabListView.h"

// #include <QDebug>
#include <QGraphicsTextItem>
#include <stdlib.h>
#include "TabThumbButton.h"

// Unit: ms
const static int MeasureInterval = 100;
const static int AnimationInteval = 50;

const static int MouseMoveThreshold = 5;
const static int MoveInertia = 7;

const static int SpacingHeight = 12;
const static int SpacingWidthMin = 10;

// FIXME: Add a interface to set the size of items.
static const int DefaultThumbnailWidth = (360 * 0.4);
static const int DefaultThumbnailHeight = (480 * 0.4);

TabListView::TabListView(QWidget* parent)
    :QGraphicsView(parent),
    m_mousePressed(false)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    connect(&m_scrollTimer, SIGNAL(timeout()), this, SLOT(animate()));

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // FIXME: remove close button on this item.
    addItem(tr("New Tab"), QPixmap("./images/newTab.png").scaled(DefaultThumbnailWidth, DefaultThumbnailHeight));
}


TabListView::~TabListView()
{
}

void TabListView::animate()
{
    if (m_speed.isNull()) {
        m_scrollTimer.stop();
        return;
    }

    QPoint newPoint = m_lastContentPos + m_speed;
    QPoint vPoint = validatCenterPoint(newPoint);
    if (m_lastContentPos == vPoint) {
        m_scrollTimer.stop();
        return;
    }

    centerOn(vPoint);

    if (newPoint.x() != vPoint.x() && newPoint.y() != vPoint.y())
        m_scrollTimer.stop();


    m_lastContentPos = newPoint;

    m_speed = QPoint(qMax(qAbs(m_speed.x()) - MoveInertia, 0) * (m_speed.x() < 0 ? -1 : 1),
        qMax(qAbs(m_speed.y()) - MoveInertia, 0) * (m_speed.y() < 0 ? -1 : 1));

}


void TabListView::mousePressEvent(QMouseEvent* e)
{
    m_mousePressed = true;
    m_scrollTimer.stop();
    m_lastMousePos = e->pos();
    m_trackPoints.clear();
    m_trackTimes.clear();
    return QGraphicsView::mousePressEvent(e);
}


void TabListView::mouseReleaseEvent(QMouseEvent* e)
{
    e->ignore();
    m_mousePressed = false;
    if (m_trackPoints.size() < 2)
        return QGraphicsView::mouseReleaseEvent(e);

    QTime now = QTime::currentTime();
    int i;
    for (i = m_trackPoints.size(); i > 0; )
        if (m_trackTimes[--i].msecsTo(now) > MeasureInterval)
            break;

    // m_trackTimes[i] is last one which less than MeasureInterval ms to now, or the last on in the vector.
    int elapsed = m_trackTimes[i].msecsTo(now);
    if (elapsed) {
        QPoint delta = m_trackPoints[i] - e->pos(); 
        m_speed = QPoint(delta.x()*AnimationInteval/elapsed, delta.y()*AnimationInteval/elapsed);
        m_scrollTimer.start(AnimationInteval);
    }

    // If it's a drag-scrolling, we don't sent mouse release event to items.
    bool isMoved = false;
    for (i = m_trackPoints.size(); i > 0; i--) {
        QPoint d = m_trackPoints[i-1] - m_trackPoints[0];
        if (qAbs(d.x()) > MouseMoveThreshold || qAbs(d.y()) > MouseMoveThreshold) {
            isMoved = true;
            break;
        }
    }
    if (isMoved)
        e->accept();
    else
        QGraphicsView::mouseReleaseEvent(e);
}


void TabListView::mouseMoveEvent(QMouseEvent* e)
{
    if (m_mousePressed) {
        QPoint newPoint = validatCenterPoint(m_lastContentPos + m_lastMousePos - e->pos()); 
        centerOn(newPoint);
        m_lastContentPos = newPoint;
        m_lastMousePos = e->pos();
        m_trackPoints.push_back(m_lastMousePos);
        m_trackTimes.push_back(QTime::currentTime());
    }
    QGraphicsView::mouseMoveEvent(e);
}

void TabListView::clear()
{
    // Keep first "New Tab" item.
    for (size_t i = 1; i < m_items.size(); i++)
        m_scene->removeItem(m_items[i]);
    m_items.erase(m_items.begin() + 1, m_items.end());
    layoutItems();
}

void TabListView::onTabSwitchedTo(TabThumbButton* tab)
{
    size_t i;
    for (i = 0; i < m_items.size(); i++)
        if (m_items[i] == tab)
           break; 
    if (i == 0)
        emit requestNewTab();
    else
        emit tabSwithedTo(i - 1);
    deleteLater();
}

void TabListView::onTabClosed(TabThumbButton* tab)
{
    size_t i;
    for (i = 0; i < m_items.size(); i++)
        if (m_items[i] == tab)
           break; 
    // "New tab" item can't be closed.
    if (i == 0)
        return;
    m_scene->removeItem(tab);
    m_items.erase(m_items.begin() + i);
    // FIXME: Animation?
    layoutItems();
    emit tabClosed(i - 1);
}

void TabListView::addItem(const QString& title, const QPixmap& icon)
{
    TabThumbButton* t = new TabThumbButton(title, icon);
    m_scene->addItem(t);
    m_items.push_back(t);

    connect(t, SIGNAL(toBeClosed(TabThumbButton*)), this, SLOT(onTabClosed(TabThumbButton*)));
    connect(t, SIGNAL(toBeSwitchedTo(TabThumbButton*)), this, SLOT(onTabSwitchedTo(TabThumbButton*)));
    layoutItems();
}

void TabListView::layoutItems()
{
    if (m_items.empty())
        return;
    TabThumbButton* t = m_items[0];
    int itemsPerRow = (width() - SpacingWidthMin) / (t->boundingRect().width() + SpacingWidthMin);
    if (itemsPerRow <= 0)
        itemsPerRow = 1;
    int gridSpacing = (width() - 2 * SpacingWidthMin - itemsPerRow * t->boundingRect().width()) / (itemsPerRow - 1);
    int gridWidth = gridSpacing + t->boundingRect().width();

    for (size_t i = 0; i < m_items.size(); i++) {
        // row and column start from zero:
        int row = i / itemsPerRow;
        int column = i % itemsPerRow;
        t = m_items[i];
        t->setX(gridWidth * column + SpacingWidthMin);
        t->setY(SpacingHeight + row * (t->boundingRect().height() + SpacingHeight));
    }
    QSize contentSize(t->boundingRect().width() * itemsPerRow + gridSpacing * (itemsPerRow - 1) + SpacingWidthMin * 2,
        (t->boundingRect().height() + SpacingHeight) * ((m_items.size() + itemsPerRow -1) / itemsPerRow) + SpacingHeight);
    m_scene->setSceneRect(QRect(QPoint(0, 0), contentSize));
}

QPoint TabListView::validatCenterPoint(const QPoint& point)
{
    QPoint p(point);
    if (p.x() < viewport()->width() / 2)
        p.setX(viewport()->width() / 2);
    if (p.y() < viewport()->height() / 2)
        p.setY(viewport()->height() / 2);

    if (p.x() > m_scene->width() - width() / 2)
        p.setX(m_scene->width() - width() / 2);
    if (p.y() > m_scene->height() - height() / 2)
        p.setY(m_scene->height() - height() / 2);

    return p;
}
