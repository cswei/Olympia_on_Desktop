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

#ifndef MainWindowBase_h
#define MainWindowBase_h

#include <QtCore/QTimer>
#include <QtGui/QMainWindow>
#include <vector>
#include "TabbedView.h"

class QStackedWidget;
class TabListView;

namespace Olympia {
namespace Browser {

enum LoadStatus {
     LOADED = 0,
     LOADING
};

class MainWindowBase : public QMainWindow
{
    Q_OBJECT

public:
    MainWindowBase(const QRect& pageRect, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~MainWindowBase(){};

    void load(const char* url, const char* networkToken, bool isInitial = false);
    void load(const QString& url);
    void newTab();

protected slots:
    virtual void loadStart();
    virtual void loadProgress(int percentage);
    virtual void loadFinished();
    virtual void onUrlChanged(const QString& newUrl) = 0;
    virtual void showTabList();
    virtual void onTabSwitchedTo(int);
    virtual void onTabClosed(int);
    virtual void onRequestNewTab();
    virtual void onIdle();
    virtual void createTab(TabbedView** tab, int x, int y, int width, int height, unsigned flags);

protected:
    QStackedWidget* tabStack() const { return m_widgetStack; };
    virtual TabbedView* currentTab() const;
    virtual void setCurrentTab(TabbedView*);
    LoadStatus loadStatus() const { return m_status; };

private:
    void updateTabList();

private:
    QStackedWidget* m_widgetStack;
    TabListView* m_tabList;

    LoadStatus m_status;
    QTimer m_idleTimer;
};

} // Browser
} // Olympia
#endif // MainWindowBase_h

