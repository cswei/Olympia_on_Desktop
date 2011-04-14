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

#ifndef TabListView_h
#define TabListView_h
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QTime>
#include <QTimer>
#include <vector>

class TabThumbButton;

class TabListView : public QGraphicsView
{
    Q_OBJECT

public:
    TabListView(QWidget* parent = 0);
    ~TabListView();
    void addItem(const QString& title, const QPixmap& icon);
    void clear();
    void layoutItems();
signals:
    void tabClosed(int); // Index starts from 0.
    void tabSwithedTo(int); // Index starts from 0.
    void requestNewTab();
public slots:
    void animate();
    void onTabClosed(TabThumbButton*);
    void onTabSwitchedTo(TabThumbButton*);
protected:
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
private:
    QPoint validatCenterPoint(const QPoint& point);
private:
    QGraphicsScene* m_scene;
    QPoint m_lastMousePos;
    QPoint m_lastContentPos;
    QPoint m_speed;
    bool m_mousePressed;
    QTimer m_scrollTimer;
    std::vector<QPoint> m_trackPoints;
    std::vector<QTime> m_trackTimes;
    std::vector<TabThumbButton*> m_items;
};

#endif // TabListView_h
