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

#include "MainWindowMobile.h"

#include "ConfigWidget.h"
#include "TabbedView.h"
#include <QDebug>
#include <QtGui/QMenuBar>
#include <QtGui/QStackedWidget>
#include <QUrl>

namespace Olympia {
namespace Browser {

const static int ProgressBarHeight = 20;

MainWindowMobile::MainWindowMobile(const QRect& rect, QWidget* w, Qt::WindowFlags f)
      : MainWindowBase(rect, w, f | Qt::MSWindowsFixedSizeDialogHint)
      , m_isConfigWidgetShowing(false)
{
    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName(QString::fromLatin1("m_centralWidget"));
    m_verticalLayout = new QVBoxLayout(m_centralWidget);
    m_verticalLayout->setSpacing(2);
    m_verticalLayout->setContentsMargins(0, 0, 0, 0);
    m_verticalLayout->setObjectName(QString::fromLatin1("m_verticalLayout"));
    m_horizontalLayout = new QHBoxLayout();
    m_horizontalLayout->setSpacing(5);
    m_horizontalLayout->setObjectName(QString::fromLatin1("m_horizontalLayout"));
    m_lineEdit = new QLineEdit(m_centralWidget);
    m_lineEdit->setObjectName(QString::fromLatin1("m_lineEdit"));
    // FIXME: set focus to m_lineEdit after pressing TAB when focus is on the last link of the page.
    m_lineEdit->setFocusPolicy(Qt::ClickFocus);
    m_lineEdit->installEventFilter(this);

    m_horizontalLayout->addWidget(m_lineEdit);

    m_menuBtn = new QPushButton(m_centralWidget);
    m_menuBtn->setObjectName(QString::fromLatin1("m_menuBtn"));
    m_menuBtn->setFocusPolicy(Qt::NoFocus);
    m_horizontalLayout->addWidget(m_menuBtn);

    m_tabsBtn = new QPushButton(m_centralWidget);
    m_tabsBtn->setObjectName(QString::fromLatin1("m_tabsBtn"));
    m_tabsBtn->setFocusPolicy(Qt::NoFocus);
    m_horizontalLayout->addWidget(m_tabsBtn);

    m_menuBtn->setIcon(QIcon("./images/menu.png"));
    m_tabsBtn->setIcon(QIcon("./images/tabs.png"));

    m_verticalLayout->addLayout(m_horizontalLayout);

    m_verticalLayout->addWidget(tabStack());

    setCentralWidget(m_centralWidget);

    m_menu = menuBar()->addMenu("menu");

    m_progressBar = new QProgressBar(this);

    connect(m_menu, SIGNAL(aboutToShow()), this, SLOT(buildMenu()));
    connect(m_tabsBtn, SIGNAL(clicked()), this, SLOT(showTabList()));
    m_progressBar->hide();
}

void MainWindowMobile::buildMenu()
{
    if (!currentTab()) 
        return;
    m_menu->clear();
    TabbedView* tab = static_cast<TabbedView*>(currentTab());
    // Add Tab specific menus for now
    QVector<QAction*> actions = tab->buildActions();
    for (int i = 0; i < actions.size(); i++)
        m_menu->addAction(actions[i]);

    m_menu->addSeparator();
    //TODO:  add the global menus here, like bookmark, history, Tabs, configuration, paste (conditional)
    m_menu->addAction(tr("Tabs"), this, SLOT(showTabList()));
    m_menu->addAction(tr("Options"), this, SLOT(configurate()));
    m_menu->addSeparator();
    m_menu->addAction(tr("Exit"), this, SLOT(close()));
}

void MainWindowMobile::loadStart()
{
    QRect r = centralWidget()->geometry();
    m_progressBar->setGeometry(r.x(), r.bottom() - ProgressBarHeight, r.width(), ProgressBarHeight);
    m_progressBar->show();
}

void MainWindowMobile::loadProgress(int percentage)
{
    if (m_progressBar)
        m_progressBar->setValue(percentage);
}

void MainWindowMobile::loadFinished()
{
    m_progressBar->hide();
}

bool MainWindowMobile::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_lineEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
            if (currentTab()) {
                currentTab()->setFocus(Qt::TabFocusReason);
                load(m_lineEdit->text());
                return true;
            }
        }
    }
    return MainWindowBase::eventFilter(obj, event);
}

void MainWindowMobile::onUrlChanged(const QString& newUrl)
{
    m_lineEdit->setText(QUrl::fromEncoded(newUrl.toLatin1()).toString());
    m_lineEdit->home(false);
}

void MainWindowMobile::configurate()
{
    if (m_isConfigWidgetShowing)
        return;
    ConfigWidget* configWidget = new ConfigWidget(this);
    configWidget->setGeometry(centralWidget()->geometry());
    configWidget->raise();
    configWidget->show();
    m_isConfigWidgetShowing = true;
    connect(configWidget, SIGNAL(destroyed()), this, SLOT(onConfigWidgetClosed()));
    // ConfigWidget will close and delete itself.
}

void MainWindowMobile::onConfigWidgetClosed()
{
    m_isConfigWidgetShowing = false;
}

} // namespace Browser
} // namespace Olympia

