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

#ifndef MainWindowAbstract_h
#define MainWindowAbstract_h

#include <QtGui/QMainWindow>

class QActionGroup;
class QIcon;
class QUrl;

namespace Olympia {
namespace Browser {

class AddressBar;
class ConfigWidget;
class History;
class StatusBar;
class Tabs;

//test
class WebView;

class MainWindowAbstract : public QMainWindow
{
    Q_OBJECT
public:
    MainWindowAbstract(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~MainWindowAbstract();

    //please use this method after constructor.
    virtual void buildUI();
    void setTabWidgetSize(int width, int height);
    void setActionGroup(QActionGroup* grp);
    void loadHomePageIfNeeded();

protected slots:
    virtual void slotLoadUrl(const QString& url);

    //addressbar
    void slotUpdateAddressBarUrlText(const QString& url);

    //progressBar
    virtual void slotHideStatusBar();
    virtual void slotShowStatusBar();
    virtual void slotUpdateStatusBarProgress(int percentage);

    //configuration dialog
    virtual void slotConfigurate();
    virtual void slotGetCurPageUrl();
    void slotClearBrowsingData(bool, bool, bool, bool, bool);

    //new tab
    virtual void slotCreateNewTab(WebView** webView, int, int, int, int, unsigned);

    //tab changed
    virtual void slotCurrentChanged(QWidget* oldWidget, int newIndex);
    virtual void slotWidgetRemoved(int index);

    //history
    void slotAddUrlIntoHistory(const QString& url);
    void slotAddTitleIntoHistory(const QString& url, const QString& title);
    void slotAddIconIntoHistory(const QString& url, const QString& iconUrl, const QIcon& icon);

signals:
    void sigCurrentPageUrl(const QString&);
    void sigClearBrowsingDataFinished();

protected:
    virtual QWidget* getANewcentralWidget();
    virtual void buildAddressBar(QWidget* parent);
    virtual void buildTabsWidget(QWidget* parent);
    virtual void buildStatusBar(QWidget* parent);
    virtual WebView* createWebView(QWidget* parent) = 0;
    virtual void buildMenuBar(QWidget* parent) = 0;
    virtual void layoutAllWidgets() = 0;
    virtual void showStartPage() = 0;

    AddressBar* m_addressBar;
    QMenuBar* m_menubar;
    StatusBar* m_statusBar;
    Tabs* m_tabWidget;
    QActionGroup* m_screenSizeGrp;
    ConfigWidget* m_configWidget;
    History* m_history;
    int m_tabWidgetWidth;
    int m_tabWidgetHeight;
};

} // Browser
} // Olympia
#endif // MainWindowAbstract_h

