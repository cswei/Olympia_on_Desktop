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

#include "OlympiaDesktopTabs.h"

#include <QDebug>
#include <QTabWidget>
#include <QTabBar>
#include "TabbedView.h"

//test
#include <stdio.h>


namespace Olympia {
namespace Browser {

class TabOlympia : public QTabWidget
{
public:
    TabOlympia(QWidget* p)
        : QTabWidget(p)
    {
    }

    QSize getContentsTabSize() const
    {
        int w = frameSize().width();
        int h = frameSize().height() - tabBar()->frameSize().height();
        return QSize(frameSize().width(), h);
    }
};

OlympiaDesktopTabs::OlympiaDesktopTabs(QWidget* w)
    : Tabs(w)
    , m_tabs(new TabOlympia(this))
{
    setFocusProxy(m_tabs);
    m_tabs->setTabsClosable(true);
    m_tabs->setMovable(true);
    m_tabs->setUsesScrollButtons(true);
    m_tabs->setElideMode(Qt::ElideRight);
    connect(m_tabs, SIGNAL(currentChanged(int)), SLOT(slotCurrentChanged(int)));
    connect(m_tabs, SIGNAL(tabCloseRequested(int)), SLOT(slotWidgetRemoved(int)));
}

void OlympiaDesktopTabs::slotWidgetRemoved(int index)
{
    if (count() > 1 && m_tabs) {
        emit widgetRemoved(index);
        QWidget* w = m_tabs->widget(index);
        m_tabs->removeTab(index);
        w->deleteLater();
    }
}

OlympiaDesktopTabs::~OlympiaDesktopTabs()
{

}

void OlympiaDesktopTabs::setFixedShape(int w, int h)
{
    setFixedSize(w, h);
    if(m_tabs)
        m_tabs->setFixedSize(w, h);
}

void OlympiaDesktopTabs::appendWidget(QWidget* w)
{
    if(m_tabs) {
        m_tabs->addTab(w, QIcon("./images/plainIcon.png"), tr("New Tab"));
    }
}

void OlympiaDesktopTabs::setCurrentWidget(QWidget* w)
{
    if(m_tabs)
        m_tabs->setCurrentWidget(w);
}

void OlympiaDesktopTabs::setPosition(int x, int y, int w, int h)
{
    setGeometry(x, y, w, h);
    if(m_tabs)
        m_tabs->setGeometry(0, 0, w, h);
}

void OlympiaDesktopTabs::slotCurrentChanged(int newIndex)
{
    emit currentChanged(m_oldWidget, newIndex);
    m_oldWidget = m_tabs->widget(newIndex);
}

QWidget* OlympiaDesktopTabs::getCurrentWidget() const
{
    return m_tabs->currentWidget();
}

int OlympiaDesktopTabs::count() const
{

    return m_tabs->count();
}

QWidget* OlympiaDesktopTabs::getWidget(int index)
{
    if(m_tabs) {
       return m_tabs->widget(index);
    }
    return 0;
}

void OlympiaDesktopTabs::slotTabSwitchedTo(int index)
{
    if(m_tabs)
        m_tabs->setCurrentIndex(index);
}

void OlympiaDesktopTabs::slotSetTitle(const QString&, const QString& title)
{
    if (m_tabs && m_tabs->count() > 0)
        m_tabs->setTabText(m_tabs->indexOf(static_cast<QWidget*>(sender())), title);
}

void OlympiaDesktopTabs::slotRemoveWidget(int index)
{
    /*if(m_tabs) {
       QWidget* w = m_tabs->widget(index);
       w->deleteLater();
       if(w == m_oldWidget)
           m_oldWidget = 0;
       m_tabs->removeTab(index);
    }*/
}

void OlympiaDesktopTabs::slotSetTabIcon(const QString&, const QString&, const QIcon& icon)
{
    int index = m_tabs->indexOf(static_cast<QWidget*>(sender()));
    if(index != -1) {
        m_tabs->setTabIcon(index, icon);
    }
}

QString OlympiaDesktopTabs::slotGetCurPageUrl()
{
    QString url("");
    TabbedView* view = dynamic_cast<TabbedView*>(m_tabs->currentWidget());
    if (view)
        url = view->url();
    
    return url;
}

QSize OlympiaDesktopTabs::getContentsTabSize()
{
    QSize size;
    if(m_tabs) {
        size = static_cast<TabOlympia*>(m_tabs)->getContentsTabSize();
    }
	return size;
}

} // namespace Browser
} // namespace Olympia

