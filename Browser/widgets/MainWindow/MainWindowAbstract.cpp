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

#include "MainWindowAbstract.h"

#include "../../History.h"
#include "AddressBar.h"
#include "BrowserWidgetsFactory.h"
#include "ConfigWidget.h"
#include "Constant.h"
#include <QDir>
#include <QIcon>
#include <QFile>
#include <QMenuBar>
#include <QPushButton>
#include <QSettings>
#include <QtGui/QListWidget>
#include <QtGui/QMenuBar>
#include <QtGui/QStackedWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <StatusBar.h>
#include <Tabs.h>
#include <WebViewQt.h>

#include <stdio.h>

namespace Olympia {
namespace Browser {

void clearDirectory(const QString& path)
{
    QDir dir(path);
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    foreach(QFileInfo info, files) {
        QFile::remove(info.absoluteFilePath());
    }
    QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QFileInfo info, dirs) {
        clearDirectory(info.absoluteFilePath());
        info.dir().rmdir(info.absoluteFilePath());
    }
}

////////////////////////////////////////////////////////////////////////////////////
MainWindowAbstract::MainWindowAbstract(QWidget* w, Qt::WindowFlags f)
    : QMainWindow(w, f)
    , m_addressBar(0)
    , m_statusBar(0)
    , m_menubar(0)
    , m_tabWidget(0)
    , m_configWidget(0)
    , m_screenSizeGrp(0)
    , m_history(new History(this))
{

}

void MainWindowAbstract::buildUI()
{
    setWindowIcon(QIcon(APP_ICON_FILEPATH));

    QWidget* centralWidget = getANewcentralWidget();
    buildMenuBar(centralWidget);
    buildAddressBar(centralWidget);
    buildTabsWidget(centralWidget);
    buildStatusBar(centralWidget);
    setCentralWidget(centralWidget);
    layoutAllWidgets();
    showStartPage();
    if(m_statusBar) {
        m_statusBar->hide();
    }

    // read saved settings into WebSettings::globalSetting();
    ConfigWidget::readToWebSettings();
}

void MainWindowAbstract::setTabWidgetSize(int width, int height)
{
    m_tabWidgetWidth = width;
    m_tabWidgetHeight = height;
}

void MainWindowAbstract::setActionGroup(QActionGroup* grp)
{
    m_screenSizeGrp = grp;
    m_screenSizeGrp->setParent(this);
}

void MainWindowAbstract::loadHomePageIfNeeded()
{
    // load home page if needed
    QSettings settings(CONF_FILEPATH, QSettings::IniFormat);
    if (settings.value("generalConf/startupPage").toString() == "Home Page") {
        this->slotLoadUrl(settings.value("generalConf/homePage").toString());
    }
}

QWidget* MainWindowAbstract::getANewcentralWidget()
{
    QWidget* centralWidget = this->centralWidget();
    if(centralWidget)
        delete centralWidget;
    return new QWidget(this);
}

MainWindowAbstract::~MainWindowAbstract()
{
}

void MainWindowAbstract::buildTabsWidget(QWidget* parent)
{
    m_tabWidget = BrowserWidgetsFactory::getInstance()->createTabs(parent);
    connect(m_tabWidget, SIGNAL(currentChanged(QWidget*, int)), SLOT(slotCurrentChanged(QWidget*, int)));
    connect(m_tabWidget, SIGNAL(widgetRemoved(int)), SLOT(slotWidgetRemoved(int)));
}

void MainWindowAbstract::slotCurrentChanged(QWidget* oldWidget, int newIndex)
{
    if(oldWidget) {
        WebView* oldView = static_cast<WebView*>(oldWidget);
        oldView->setWebPageVisible(false);
    }
    QWidget* newWidget = m_tabWidget->getWidget(newIndex);
    if(newWidget) {
        WebView* newView = static_cast<WebView*>(newWidget);
        newView->setWebPageVisible(true);
        slotUpdateAddressBarUrlText(newView->url());

        //progressbar
        if(newView->isLoading()) {
            if(m_statusBar) {
                m_statusBar->show();
                m_statusBar->setValue(newView->loadProgress());
            }
        } else {
            m_statusBar->hide();
        }
    }
}

void MainWindowAbstract::slotWidgetRemoved(int index)
{
    QWidget* widget = m_tabWidget->getWidget(index);
    if(widget) {
        WebView* view = static_cast<WebView*>(widget);
        view->setWebPageVisible(false);
    }
}

void MainWindowAbstract::buildStatusBar(QWidget* parent)
{
    m_statusBar = BrowserWidgetsFactory::getInstance()->createStatusBar(parent);
    if(m_statusBar) {
        m_statusBar->hide();
    }
}

void MainWindowAbstract::buildAddressBar(QWidget* parent)
{
    m_addressBar = BrowserWidgetsFactory::getInstance()->createAddressBar(parent);
    m_addressBar->setMatchedHistModel(m_history->matchedModel());
    connect(m_addressBar, SIGNAL(sigGo(const QString&)), SLOT(slotLoadUrl(const QString&)));
    connect(m_addressBar, SIGNAL(sigMatchInput(const QString&)),
        m_history, SLOT(slotSearchHistoryItem(const QString&)));
    connect(m_history, SIGNAL(sigUpdateMatchedHistItems(void)),
        m_addressBar, SLOT(slotUpdateMachedHistItems(void)));
}

void MainWindowAbstract::slotConfigurate()
{
    if(!m_configWidget) {
        m_configWidget = new ConfigWidget(this);
        connect(m_configWidget, SIGNAL(sigReqCurrentPageUrl(void)),
            this, SLOT(slotGetCurPageUrl(void)));
        connect(this, SIGNAL(sigCurrentPageUrl(const QString&)),
            m_configWidget, SLOT(slotSetHomePageUrl(const QString&)));
        connect(m_configWidget, SIGNAL(sigClearBrowsingData(bool, bool, bool, bool, bool)),
            this, SLOT(slotClearBrowsingData(bool, bool, bool, bool, bool)));
        connect(this, SIGNAL(sigClearBrowsingDataFinished()),
            m_configWidget, SLOT(slotClearBrowsingDataFinished()));
    }
    m_configWidget->setGeometry(0, 0, width(), height());
    m_configWidget->raise();
    m_configWidget->show();
}

void MainWindowAbstract::slotGetCurPageUrl()
{
    emit sigCurrentPageUrl(m_tabWidget->slotGetCurPageUrl());
}

void MainWindowAbstract::slotClearBrowsingData(
    bool passwd, bool history, bool cookie, bool cache, bool pushContents)
{
    if (passwd) {
        // to be implemented.
    }
    if (history) {
        m_history->clear();
    }
    if (cookie) {
        clearDirectory(DEFAULT_COOCKIE_PATH);
    }
    if (cache) {
        clearDirectory(DEFAULT_CACHE_PATH);
    }
    if (pushContents) {
        // to be implemented.
    }

    //send finished signal.
    emit sigClearBrowsingDataFinished();
}

void MainWindowAbstract::slotUpdateAddressBarUrlText(const QString& url)
{
    if (m_addressBar)
        m_addressBar->setUrlText(url);
}

void MainWindowAbstract::slotCreateNewTab(WebView** webView, int, int, int, int, unsigned)
{
    *webView = createWebView(m_tabWidget);
}

void MainWindowAbstract::slotHideStatusBar()
{
    if(m_statusBar && m_tabWidget && m_tabWidget->getCurrentWidget() == sender()) {
        m_statusBar->hide();
    }
}

void MainWindowAbstract::slotShowStatusBar()
{
    if(m_statusBar && m_tabWidget && m_tabWidget->getCurrentWidget() == sender()) {
        m_statusBar->show();
    }
}

void MainWindowAbstract::slotUpdateStatusBarProgress(int percentage)
{
    if(m_statusBar && m_tabWidget && m_tabWidget->getCurrentWidget() == sender()) {
        m_statusBar->setValue(percentage);
    }
}

void MainWindowAbstract::slotLoadUrl(const QString& url)
{
    if(m_tabWidget) {
        if(m_tabWidget->count() == 0)
            createWebView(m_tabWidget);
        if(m_tabWidget->getCurrentWidget()) {
            m_tabWidget->getCurrentWidget()->setFocus();
            static_cast<WebView*>(m_tabWidget->getCurrentWidget())->load(QUrl(url).toEncoded().data(), "", false);
        }
   }
}

void MainWindowAbstract::slotAddUrlIntoHistory(const QString& url)
{
    if (!m_history)
        return;

    m_history->slotAddHistoryUrl(url);
}

void MainWindowAbstract::slotAddTitleIntoHistory(const QString& url, const QString& title)
{
    if (!m_history || url == title)
        return;

    m_history->slotAddTitle(url, title);
}

void MainWindowAbstract::slotAddIconIntoHistory(const QString& url, const QString& iconUrl, const QIcon& icon)
{
    if (!m_history)
        return;

    m_history->slotAddIcon(url, iconUrl, icon);
}


} // namespace Browser
} // namespace Olympia

