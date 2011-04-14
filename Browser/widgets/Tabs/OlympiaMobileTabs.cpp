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

#include "OlympiaMobileTabs.h"

#include <QStackedWidget>
#include "TabbedView.h"

//test
#include <stdio.h>
namespace Olympia {
namespace Browser {

OlympiaMobileTabs::OlympiaMobileTabs(QWidget* w)
    : Tabs(w)
    , m_tabs(new QStackedWidget(this))
{
    m_tabs->setFrameStyle(QFrame::NoFrame);
    setFocusProxy(m_tabs);
    connect(m_tabs, SIGNAL(currentChanged(int)), SLOT(slotCurrentChanged(int)));
    connect(m_tabs, SIGNAL(widgetRemoved(int)), SIGNAL(widgetRemoved(int)));
}

OlympiaMobileTabs::~OlympiaMobileTabs()
{

}

void OlympiaMobileTabs::setFixedShape(int w, int h)
{
    setFixedSize(w, h);
    if(m_tabs) {
        m_tabs->setFixedSize(w, h);
        for(int i = 0; i < m_tabs->count(); i++) {
            m_tabs->widget(i)->setFixedSize(w,h);    
        }
    }
}

void OlympiaMobileTabs::appendWidget(QWidget* w)
{
    if(m_tabs) {
        m_tabs->addWidget(w);
    }
}

void OlympiaMobileTabs::setCurrentWidget(QWidget* w)
{
    if(m_tabs)
        m_tabs->setCurrentWidget(w);
}

void OlympiaMobileTabs::setPosition(int x, int y, int w, int h)
{
    setGeometry(x, y, w, h);
    if(m_tabs)
        m_tabs->setGeometry(0, 0, w, h);
}

void OlympiaMobileTabs::slotCurrentChanged(int newIndex)
{
    emit currentChanged(m_oldWidget, newIndex);
    m_oldWidget = m_tabs->widget(newIndex);
}

QWidget* OlympiaMobileTabs::getCurrentWidget() const
{
    return m_tabs->currentWidget();
}

int OlympiaMobileTabs::count() const
{

    return m_tabs->count();
}

QWidget* OlympiaMobileTabs::getWidget(int index)
{
    if(m_tabs) {
       return m_tabs->widget(index);
    }
    return 0;
}

void OlympiaMobileTabs::slotTabSwitchedTo(int index)
{
    if(m_tabs)
        m_tabs->setCurrentIndex(index);
}

void OlympiaMobileTabs::slotRemoveWidget(int index)
{
    if(m_tabs) {
       QWidget* w = m_tabs->widget(index);
       m_tabs->removeWidget(w);
       w->deleteLater();
    }
}

void OlympiaMobileTabs::slotSetTitle(const QString&, const QString&)
{

}

void OlympiaMobileTabs::slotSetTabIcon(const QString&, const QString&, const QIcon& icon)
{

}

QString OlympiaMobileTabs::slotGetCurPageUrl()
{
    QString url("");
    TabbedView* view = dynamic_cast<TabbedView*>(m_tabs->currentWidget());
    if (view)
        url = view->url();
    
    return url;
}

QSize OlympiaMobileTabs::getContentsTabSize()
{
    return m_tabs->size();
}
} // namespace Browser
} // namespace Olympia

