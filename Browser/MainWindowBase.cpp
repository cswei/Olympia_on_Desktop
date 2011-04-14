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

#include "MainWindowBase.h"

#include "TabListView.h"
// #include <QDebug>
#include <QtGui/QListWidget>
#include <QtGui/QMenuBar>
#include <QtGui/QStackedWidget>
#include <QUrl>

namespace Olympia {
namespace Browser {

static const int ThumbnailWidth = (360 * 0.4);
static const int ThumbnailHeight = (480 * 0.4);

MainWindowBase::MainWindowBase(const QRect& rect, QWidget *w, Qt::WindowFlags f)
      : QMainWindow(w, f)
      , m_tabList(0)
      , m_status(LOADED)
{
    m_widgetStack = new QStackedWidget(this);
    m_widgetStack->setFixedSize(rect.size());
    connect(&m_idleTimer, SIGNAL(timeout()), this, SLOT(onIdle()));
    m_idleTimer.start(100);
}

void MainWindowBase::setCurrentTab(TabbedView* tab)
{
    if (!tab)
        return;

    if (currentTab()) {
        disconnect(currentTab(), 0, this, 0);
        currentTab()->setWebPageVisible(false);
    }

    m_widgetStack->setCurrentWidget(tab);

    connect(tab, SIGNAL(loadStart()), this, SLOT(loadStart()));
    connect(tab, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
    connect(tab, SIGNAL(loadFinished()), this, SLOT(loadFinished()));
    connect(tab, SIGNAL(urlChanged(const QString&)), this, SLOT(onUrlChanged(const QString&)));
    connect(tab, SIGNAL(requestCreateTab(TabbedView**, int, int, int, int, unsigned)),
        this, SLOT(createTab(TabbedView**, int, int, int, int, unsigned)));
    tab->setWebPageVisible(true);
    onUrlChanged(tab->url());
}

void MainWindowBase::updateTabList()
{
    // FIXME: avoid clearing and rebuilding all item for the sake of performance.
    m_tabList->clear();

    int tabs = m_widgetStack->count();
    for(size_t i = 0; i < tabs; i++) {
        TabbedView* tab = static_cast<TabbedView*>(m_widgetStack->widget(i));
        if (tab) {
            QString title = tab->title();
            if(title.isEmpty())
                title = tab->url();
            if(title.isEmpty())
                title = tr("Untitled page");
            m_tabList->addItem(title, tab->thumbnail(QSize(ThumbnailWidth, ThumbnailHeight)));
        }
    }
}

void MainWindowBase::onTabSwitchedTo(int index)
{
    m_tabList->lower();
    m_tabList->hide();
    setCurrentTab(static_cast<TabbedView*>(m_widgetStack->widget(index)));
}

void MainWindowBase::onTabClosed(int index)
{
    setCurrentTab(0);
    QWidget* widget = m_widgetStack->widget(index);
    m_widgetStack->removeWidget(widget);
    delete widget;
}

void MainWindowBase::onRequestNewTab()
{
    m_tabList->lower();
    m_tabList->hide();
    newTab();
}

void MainWindowBase::createTab(TabbedView** tab, int, int, int, int, unsigned)
{
    newTab();
    *tab = currentTab();
}

void MainWindowBase::newTab()
{
    TabbedView* tab = new TabbedView(m_widgetStack->size(), this);
    tab->setFixedSize(m_widgetStack->size());
    m_widgetStack->addWidget(tab);
    setCurrentTab(tab);
}

void MainWindowBase::showTabList()
{
    if(!m_tabList) {
        // Overlap other widgets when show.
        m_tabList = new TabListView(this);
        connect(m_tabList, SIGNAL(tabClosed(int)), this, SLOT(onTabClosed(int)));
        connect(m_tabList, SIGNAL(tabSwithedTo(int)), this, SLOT(onTabSwitchedTo(int)));
        connect(m_tabList, SIGNAL(requestNewTab()), this, SLOT(onRequestNewTab()));
    }

    // Set size first, TabListView need a correct size to layout items.
    m_tabList->setGeometry(centralWidget()->geometry());

    updateTabList();
    m_tabList->raise();
    m_tabList->show();
}

void MainWindowBase::load(const QString& url)
{
    load(QUrl(url).toEncoded().data(), "");
}

void MainWindowBase::load(const char* url, const char* networkToken, bool isInitial)
{
    if (!currentTab())
        newTab();
    currentTab()->load(url, networkToken, isInitial);
}

void MainWindowBase::loadStart()
{
    m_status = LOADING;
}

void MainWindowBase::loadProgress(int percentage)
{
}

void MainWindowBase::loadFinished()
{
    m_status = LOADED;
}

TabbedView* MainWindowBase::currentTab() const
{
    return static_cast<TabbedView*>(m_widgetStack->currentWidget());
}

void MainWindowBase::onIdle()
{
    TabbedView* tab = currentTab();
    if (tab)
        tab->triggerRender();
}

} // namespace Browser
} // namespace Olympia

