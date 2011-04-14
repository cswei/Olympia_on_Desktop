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

#include "BrowserWidgetsFactory.h"

#include <qaction.h>
#include <QtXml/QDomNode>

#include "Constant.h"
#include "AddressBar.h"
#include "MainWindowAbstract.h"
#include "OlympiaDesktopMainWindow.h"
#include "OlympiaDesktopTabs.h"
#include "OlympiaMobileAddressBar.h"
#include "OlympiaMobileMainWindow.h"
#include "OlympiaMobileTabs.h"
#include "OlympiaMobileStatusBar.h"
#include "Tabs.h"
#include "StatusBar.h"

#include <stdio.h>


namespace Olympia {
namespace Browser {

class PropertyNodes {
public:
    PropertyNodes()
        : type("undefined")
        , css("undefined")
        , name("undefined")
        , width(-1)
        , height(-1)
    {
    }
    friend class BrowserWidgetsFactory;

private:
    QString type;
    QString css;
    QString name;
    int width;
    int height;
};

static BrowserWidgetsFactory* s_instance = 0;

BrowserWidgetsFactory* BrowserWidgetsFactory::getInstance()
{
    if(!s_instance) {
        s_instance = new BrowserWidgetsFactory();
    }
    return s_instance;
}

BrowserWidgetsFactory::BrowserWidgetsFactory()
    : m_domTree(0)
{

}

BrowserWidgetsFactory::~BrowserWidgetsFactory()
{

}

QActionGroup* BrowserWidgetsFactory::buildScreenSizeActionGroup()
{
    QActionGroup* grp = new QActionGroup(0);
    QDomNodeList list = m_domTree->elementsByTagName("device");
    QString actionTitle;
    QString actionInfo;
    QString width;
    QString height;
    for (int i = 0; i < list.count(); i++) {
        QDomNodeList deviceNodeList = list.at(i).childNodes();
        for (int j = 0; j < deviceNodeList.count(); j++) {
            if (deviceNodeList.at(j).nodeName() == "name") {
                actionTitle = deviceNodeList.at(j).toElement().text();
            } else if (deviceNodeList.at(j).nodeName() == "mainwindow") {
                width = deviceNodeList.at(j).firstChildElement().text();
                height = deviceNodeList.at(j).lastChildElement().text();
                actionTitle = actionTitle + ":" + width + "*" + height;
            } else if (deviceNodeList.at(j).nodeName() == "addressbar") {
                actionInfo = actionInfo + "addressbar" +":";
                width = deviceNodeList.at(j).firstChildElement().text();
                height = deviceNodeList.at(j).lastChildElement().text();
                actionInfo = actionInfo + width + "*" + height + ";";
            } else if (deviceNodeList.at(j).nodeName() == "statusbar") {
                actionInfo = actionInfo + "statusbar" +":";
                width = deviceNodeList.at(j).firstChildElement().text();
                height = deviceNodeList.at(j).lastChildElement().text();
                actionInfo = actionInfo + width + "*" + height + ";";
            }
        }

        QAction* action = new QAction(actionTitle,grp);
        if (actionInfo.length()) {
            action->setIconText(actionInfo);
            actionInfo.clear();
        }
        grp->addAction(action);
    }
    return grp;

}

void BrowserWidgetsFactory::setDomDocument(const QDomDocument& domTree)
{
    m_domTree = &domTree;
}

QMainWindow* BrowserWidgetsFactory::createMainWindow(QWidget* parent)
{
    MainWindowAbstract* mainWindow = 0;
    QString type = "mainwindow_" + m_domTree->elementsByTagName("type").at(0).toElement().text(); //className + type;
    int width = m_domTree->elementsByTagName("mainwindow").at(0).firstChildElement().text().toInt();
    int height = m_domTree->elementsByTagName("mainwindow").at(0).lastChildElement().text().toInt();
    if (type == OLYMPIAMOBILEMAINWINDOW_TYPEID) {
        mainWindow = new OlympiaMobileMainWindow(parent);
        QActionGroup* grp = buildScreenSizeActionGroup();
        mainWindow->setActionGroup(grp);
    }
    else if (type == OLYMPIADESKTOPMAINWINDOW_TYPEID)
        mainWindow = new OlympiaDesktopMainWindow(parent);

    if (mainWindow) {
        mainWindow->setTabWidgetSize(width, height);

        mainWindow->buildUI();
        mainWindow->loadHomePageIfNeeded();
    }
    return mainWindow;
}

AddressBar* BrowserWidgetsFactory::createAddressBar(QWidget* parent)
{
    AddressBar* addressBar = 0;
    QString type = "addressbar_" + m_domTree->elementsByTagName("type").at(0).toElement().text();
    int width = m_domTree->elementsByTagName("addressbar").at(0).firstChildElement().text().toInt();
    int height = m_domTree->elementsByTagName("addressbar").at(0).lastChildElement().text().toInt();
    if (type == OLYMPIAMOBILEADDRESSBAR_TYPEID) {
        addressBar = new OlympiaMobileAddressBar(parent);
        if ((width > 0) && (height > 0))
            addressBar->updateSize(QSize(width, height));
    }
    return addressBar;
}

Tabs* BrowserWidgetsFactory::createTabs(QWidget* parent)
{
    Tabs* tabs = 0;
    QString type = "tabs_" + m_domTree->elementsByTagName("type").at(0).toElement().text();
    int width = m_domTree->elementsByTagName("tabs").at(0).firstChildElement().text().toInt();
    int height = m_domTree->elementsByTagName("tabs").at(0).lastChildElement().text().toInt();
    if (type == OLYMPIAMOBILETABS_TYPEID) {
        tabs = new OlympiaMobileTabs(parent);
        if ((width > 0) && (height > 0))
            tabs->setFixedShape(width, height);
    } else if (type == OLYMPIADESKTOPTABS_TYPEID) {
        tabs = new OlympiaDesktopTabs(parent);
    }
    return tabs;
}

StatusBar* BrowserWidgetsFactory::createStatusBar(QWidget* parent)
{
    StatusBar* statusBar = 0;
    QString type = "statusbar_" + m_domTree->elementsByTagName("type").at(0).toElement().text();
    int width = m_domTree->elementsByTagName("statusbar").at(0).firstChildElement().text().toInt();
    int height = m_domTree->elementsByTagName("statusbar").at(0).lastChildElement().text().toInt();
    if (type == OLYMPIAMOBILESTATUSBAR_TYPEID) {
        statusBar = new OlympiaMobileStatusBar(parent);
        if ((width > 0) && (height > 0))
            statusBar->setFixedShape(width, height);
    }
    return statusBar;
}

} // namespace Browser
} // namespace Olympia

