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

#include "OlympiaMobileMainWindow.h"

#include "OlympiaMobileAddressBar.h"
#include "OlympiaMobileStatusBar.h"
#include <qaction.h> 
#include <QMenuBar>
#include <QStackedWidget>
#include <QtXml/QDomNode>
#include <QUrl>
#include <QVBoxLayout>
#include <stdio.h>
#include "TabbedView.h"
#include "TabListView.h"
#include "Tabs.h"
#include <WebViewQt.h>



namespace Olympia {
namespace Browser {

OlympiaMobileMainWindow::OlympiaMobileMainWindow(QWidget* w, Qt::WindowFlags f)
    : MainWindowAbstract(w, f)
    , m_menu(0)
    , m_view(0)
    , m_tabList(0)
{

}

OlympiaMobileMainWindow::~OlympiaMobileMainWindow()
{

}

void OlympiaMobileMainWindow::layoutAllWidgets()
{
    centralWidget()->setFixedSize(m_tabWidgetWidth, m_tabWidgetHeight + m_addressBar->height());
    m_addressBar->setGeometry(0, 0, m_tabWidgetWidth, m_addressBar->height());
    m_tabWidget->setFixedShape(m_tabWidgetWidth, m_tabWidgetHeight);
    m_tabWidget->setPosition(0, m_addressBar->height(), m_tabWidgetWidth, m_tabWidgetHeight);
    m_statusBar->setFixedShape(m_tabWidgetWidth, m_statusBar->height());
    m_statusBar->setGeometry(0, centralWidget()->height() - m_statusBar->height(), m_tabWidgetWidth, m_statusBar->height());
    setFixedSize(m_tabWidgetWidth, centralWidget()->height() + m_menubar->heightForWidth(m_tabWidgetWidth));
    if (m_tabList) {
        m_tabList->setGeometry(0, m_menubar->heightForWidth(m_tabWidgetWidth), m_tabWidgetWidth, centralWidget()->height());
        m_tabList->layoutItems();
    }
}

void OlympiaMobileMainWindow::buildMenuBar(QWidget* parent)
{
    //m_menubar = new QMenuBar(0);
    m_menubar = menuBar();
    m_menu = m_menubar->addMenu("menu");
    m_view = m_menubar->addMenu("view");
    connect(m_menu, SIGNAL(aboutToShow()), this, SLOT(slotBuildMenu()));
    connect(m_view, SIGNAL(aboutToShow()), this, SLOT(slotBuildView()));
    //setMenuBar(m_menubar);
}

void OlympiaMobileMainWindow::showStartPage()
{
    //FIXME: read from setting;
    createWebView(m_tabWidget);
}

void OlympiaMobileMainWindow::slotBuildMenu()
{
    if (!m_tabWidget || !m_tabWidget->getCurrentWidget())
        return;
    m_menu->clear();
    static_cast<WebView*>(m_tabWidget->getCurrentWidget())->unsetMultiTouchMode();
    TabbedView* tab = static_cast<TabbedView*>(m_tabWidget->getCurrentWidget());
    // Add Tab specific menus for now
    QVector<QAction*> actions = tab->buildActions();
    for (int i = 0; i < actions.size(); i++)
        m_menu->addAction(actions[i]);

    m_menu->addSeparator();
    //TODO:  add the global menus here, like bookmark, history, Tabs, configuration, paste (conditional)
    m_menu->addAction(tr("Orientation"), this, SLOT(slotOrientation()));
    m_menu->addAction(tr("Tabs"), this, SLOT(slotShowTabList()));
    m_menu->addAction(tr("Options"), this, SLOT(slotConfigurate()));
    m_menu->addSeparator();
    m_menu->addAction(tr("Exit"), this, SLOT(close()));
}

void OlympiaMobileMainWindow::slotBuildView()
{
    static_cast<WebView*>(m_tabWidget->getCurrentWidget())->unsetMultiTouchMode();
    if (m_view->actions().size() == 0) {
        QMenu* subMenu = new QMenu(m_view);
        subMenu->setTitle(tr("Screen Size"));
        QFont font;
        font.setBold(true);
        if (m_screenSizeGrp && !m_screenSizeGrp->actions().isEmpty())
            m_screenSizeGrp->actions()[0]->setFont(font);
        subMenu->addActions(m_screenSizeGrp->actions());
        m_view->addMenu(subMenu);
        connect(m_screenSizeGrp, SIGNAL(selected(QAction*)), this, SLOT(slotChangeScreenSize(QAction*)));
    }
}

void OlympiaMobileMainWindow::slotChangeScreenSize(QAction* action)
{
    if(action->font().bold())
        return;

    for (int i = 0; i < m_screenSizeGrp->actions().count(); i++) {
        if (m_screenSizeGrp->actions().at(i)->font().bold()) {
            QFont font;
            font.setBold(false);
            m_screenSizeGrp->actions().at(i)->setFont(font);
            break;
        }
    }

    QFont font;
    font.setBold(true);
    action->setFont(font);

    QString text = action->text();
    int index = text.indexOf(":");
    text = text.remove(0, index+1);
    QStringList list = text.split("*");
    setTabWidgetSize(list.at(0).toInt(), list.at(1).toInt());
   
    text = action->iconText();
    list = text.split(";");
    QStringList sublist;
    for (int i = 0; i < list.count(); i++) {
        if ( m_addressBar && list[i].contains("addressbar")) {
            index = list[i].indexOf(":");
            list[i] = list[i].remove(0, index +1);
            sublist = list[i].split("*");
            m_addressBar->setFixedWidth(sublist.at(0).toInt());
            m_addressBar->setFixedHeight(sublist.at(1).toInt());
        } else if (m_statusBar && list[i].contains("statusbar")) {
            index = list[i].indexOf(":");
            list[i] = list[i].remove(0, index +1);
            sublist = list[i].split("*");
            m_statusBar->setFixedWidth(sublist.at(0).toInt());
            m_statusBar->setFixedHeight(sublist.at(1).toInt());
        }
    }
    layoutAllWidgets();
    m_tabWidget->windowSizeChanged();
}

void OlympiaMobileMainWindow::slotOrientation()
{
    setTabWidgetSize(m_tabWidgetHeight, m_tabWidgetWidth);
    m_addressBar->setFixedWidth(m_tabWidgetWidth);
    m_statusBar->setFixedWidth(m_tabWidgetWidth);
    layoutAllWidgets();
    m_tabWidget->orientationChanged();
}

void OlympiaMobileMainWindow::slotShowTabList()
{
    if(!m_tabList) {
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
    m_tabList->show();
}

void OlympiaMobileMainWindow::slotTabListClosed()
{
    m_tabList = 0;
}

WebView* OlympiaMobileMainWindow::createWebView(QWidget* parent)
{
    TabbedView* view = new TabbedView(m_tabWidget->size(), parent);
    view->setFixedSize(m_tabWidget->size());

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
    connect(m_tabWidget, SIGNAL(sigOrientationChanged()), view, SLOT(slotOrientationChanged()));
    connect(m_tabWidget, SIGNAL(sigWindowSizeChanged()), view, SLOT(slotWindowSizeChanged()));

   return view;
}


void OlympiaMobileMainWindow::slotCreateNewTab()
{
    if(m_statusBar) {
        m_statusBar->hide();
    }
    createWebView(m_tabWidget);
}

} // namespace Browser
} // namespace Olympia

