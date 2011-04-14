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

#ifndef Tabs_h
#define Tabs_h

#include <QWidget>
#include <QTime>
#include <QTimer>

class QIcon;

namespace Olympia {
namespace Browser {

class Tabs : public QWidget
{
    Q_OBJECT
public:
    Tabs(QWidget* parent = 0);
    virtual ~Tabs();
    virtual QWidget* getCurrentWidget() const = 0;
    virtual int count() const = 0;
    virtual void appendWidget(QWidget* w) = 0;
    virtual void setCurrentWidget(QWidget* w) = 0;
    virtual void setFixedShape(int w, int h) = 0;
    virtual void setPosition(int x, int y, int w, int h) = 0;
    virtual QWidget* getWidget(int index) = 0;
    virtual QSize getContentsTabSize() = 0;
    void orientationChanged();
    void windowSizeChanged();

public slots:
    virtual void slotRemoveWidget(int index) = 0;
    virtual void slotTabSwitchedTo(int index) = 0;
    virtual void slotSetTitle(const QString& url, const QString& title) = 0;
    virtual void slotSetTabIcon(const QString& url, const QString& icon_url, const QIcon& icon) = 0;
    virtual QString slotGetCurPageUrl() = 0;
signals:
    void currentChanged(QWidget* w, int newIndex);
    void widgetRemoved(int index);
    void sigOrientationChanged();
    void sigWindowSizeChanged();

protected:
    void customEvent(QEvent* e);
    QWidget* m_oldWidget;

private slots:
    void slotCustomEventPostedPeriodically();

private:
    QTimer m_repaintTimer;
    QTime m_repaintInterval;
};

} // Browser
} // Olympia
#endif // Tabs_h

