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

#include "OlympiaDesktopMainWindow.h"

#include "OlympiaMobileAddressBar.h"
#include "OlympiaMobileStatusBar.h"
#include <QStackedWidget>
#include <QMenuBar>
#include <QUrl>
#include <QVBoxLayout>
#include <stdio.h>
#include "TabbedView.h"
#include "TabListView.h"
#include "Tabs.h"
#include <WebViewQt.h>



namespace Olympia {
namespace Browser {

OlympiaDesktopMainWindow::OlympiaDesktopMainWindow(QWidget* w, Qt::WindowFlags f)
    : MainWindowAbstract(w, f)
    , m_menu(0)
    , m_tabList(0)
{

}

OlympiaDesktopMainWindow::~OlympiaDesktopMainWindow()
{

}

void OlympiaDesktopMainWindow::layoutAllWidgets()
{
    centralWidget()->resize(frameSize());
    m_addressBar->updateSize(QSize(centralWidget()->width(), m_addressBar->height()));
    m_tabWidget->setPosition(0, m_addressBar->frameSize().height(), centralWidget()->frameSize().width(), centralWidget()->frameSize().height() - m_addressBar->frameSize().height() - (m_menubar ? m_menubar->frameSize().height() : 0));
    m_statusBar->setGeometry(0, centralWidget()->frameSize().height() - m_statusBar->frameSize().height() - (m_menubar ? m_menubar->frameSize().height() : 0), centralWidget()->frameSize().width(), m_statusBar->frameSize().height());
}

WebView* OlympiaDesktopMainWindow::createWebView(QWidget* parent)
{
    TabbedView* view = new TabbedView(m_tabWidget->getContentsTabSize(), parent);
    m_tabWidget->appendWidget(view);
    m_tabWidget->setCurrentWidget(view);

    connect(view, SIGNAL(loadStart()), SLOT(slotShowStatusBar()));
    connect(view, SIGNAL(loadProgress(int)), SLOT(slotUpdateStatusBarProgress(int)));
    connect(view, SIGNAL(loadFinished()), SLOT(slotHideStatusBar()));
    connect(view, SIGNAL(loadFailed()), SLOT(slotHideStatusBar()));
    connect(view, SIGNAL(loadFinished()), m_addressBar, SLOT(slotLoadFinished()));
    connect(view, SIGNAL(requestCreateTab(WebView**, int, int, int, int, unsigned)),
        SLOT(slotCreateNewTab(WebView**, int, int, int, int, unsigned)));
    connect(view, SIGNAL(urlChanged(const QString&)), SLOT(slotUpdateAddressBarUrlText(const QString&)));
    connect(view, SIGNAL(urlChanged(const QString&)), SLOT(slotAddUrlIntoHistory(const QString&)));
    connect(view, SIGNAL(sigSetTitle(const QString&, const QString&)),
        SLOT(slotAddTitleIntoHistory(const QString&, const QString&)));
    connect(view, SIGNAL(sigSetTitle(const QString&, const QString&)),
        m_tabWidget, SLOT(slotSetTitle(const QString&, const QString&)));
    connect(view, SIGNAL(sigSetIcon(const QString&, const QString&, const QIcon&)),
        SLOT(slotAddIconIntoHistory(const QString&, const QString&, const QIcon&)));
    connect(view, SIGNAL(sigSetIcon(const QString&, const QString&, const QIcon&)),
        m_tabWidget, SLOT(slotSetTabIcon(const QString&, const QString&, const QIcon&)));
    connect(view, SIGNAL(sigSetIcon(const QString&, const QString&, const QIcon&)),
        m_addressBar, SLOT(slotSetUrlIcon(const QString&, const QString&, const QIcon&)));
    connect(view, SIGNAL(sigNoFaviconLoaded()), m_addressBar, SLOT(slotUseDefaultUrlIcon()));

   return view;
}

void OlympiaDesktopMainWindow::buildMenuBar(QWidget* parent)
{
    //m_menubar = new QMenuBar(0);
    #if 0

    m_menubar = menuBar();
    m_menu = m_menubar->addMenu("menu");
    connect(m_menu, SIGNAL(aboutToShow()), this, SLOT(slotBuildMenu()));
    #endif
    //setMenuBar(m_menubar);
}

void OlympiaDesktopMainWindow::showStartPage()
{
    //FIXME: read from setting;
    createWebView(m_tabWidget);
}

void OlympiaDesktopMainWindow::slotBuildMenu()
{
    if (!m_tabWidget || !m_tabWidget->getCurrentWidget())
        return;
    m_menu->clear();
    TabbedView* tab = static_cast<TabbedView*>(m_tabWidget->getCurrentWidget());
    // Add Tab specific menus for now
    QVector<QAction*> actions = tab->buildActions();
    for (int i = 0; i < actions.size(); i++)
        m_menu->addAction(actions[i]);

    m_menu->addSeparator();
    //TODO:  add the global menus here, like bookmark, history, Tabs, configuration, paste (conditional)
    m_menu->addAction(tr("Tabs"), this, SLOT(slotShowTabList()));
    m_menu->addAction(tr("Options"), this, SLOT(slotConfigurate()));
    m_menu->addSeparator();
    m_menu->addAction(tr("Exit"), this, SLOT(close()));
}

void OlympiaDesktopMainWindow::slotShowTabList()
{
    /*if(!m_tabList) {
        m_tabList = new TabListView(this);
        m_tabList->setGeometry(centralWidget()->geometry());
        connect(m_tabList, SIGNAL(destroyed()), this, SLOT(slotTabListClosed()));
        connect(m_tabList, SIGNAL(tabClosed(int)), m_tabWidget, SLOT(slotRemoveWidget(int)));
        connect(m_tabList, SIGNAL(tabSwithedTo(int)), m_tabWidget, SLOT(slotTabSwitchedTo(int)));
        connect(m_tabList, SIGNAL(requestNewTab()), this, SLOT(slotCreateNewTab()));


        if(m_tabWidget) {
            for(int i = 0; i < m_tabWidget->count(); i++) {
                TabbedView* tab = static_cast<TabbedView*>(m_tabWidget->getWidget(i));
                if (tab) {
                    QString title = tab->title();
                    if(title.isEmpty())
                        title = tab->url();
                    if(title.isEmpty())
                        title = tr("Untitled page");
                    m_tabList->addItem(title, tab->thumbnail(QSize(360 * 0.4, 480 * 0.4)));
                }
            }
        }
    }

    m_tabList->raise();
    m_tabList->show();*/
}

void OlympiaDesktopMainWindow::slotTabListClosed()
{
    m_tabList = 0;
}

void OlympiaDesktopMainWindow::resizeEvent(QResizeEvent* event)
{
    //layoutAllWidgets();
    MainWindowAbstract::resizeEvent(event);
}

} // namespace Browser
} // namespace Olympia

