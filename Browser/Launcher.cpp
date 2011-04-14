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

#include "Launcher.h"

#include "BrowserWidgetsFactory.h"
#include <QCoreApplication>
#include <QtXml/QDomDocument>
#include <QFile>
#include <QMainWindow>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include "WebSettings.h"

namespace Olympia {
namespace Browser {

static const QString DEFAULT_CONFIGSTRING = "\
<?xml version=\"1.0\"?> \
<config> \
    <type>olympiamobile</type> \
    <device> \
        <name>Torch</name> \
        <mainwindow> \
            <width>360</width> \
            <height>480</height> \
        </mainwindow> \
        <addressbar> \
            <width>360</width> \
            <height>30</height> \
        </addressbar> \
        <statusbar> \
            <width>360</width> \
            <height>30</height> \
        </statusbar> \
    </device> \
</config> \
";

bool Launcher::readConfigFile(const QString& xmlFileName)
{
    bool isReadError = false;
    if(m_domTree) {
        delete m_domTree;
    }
    m_domTree = new QDomDocument;

    //FIXME: the xml is very short, if it is very big, we have to change this algorithm.
    QFile file(xmlFileName);
    if (!file.open(QFile::ReadOnly | QFile::Text) || !m_domTree->setContent(&file)) {
        isReadError = true;
    }
    file.close();
    if(isReadError && m_domTree->setContent(DEFAULT_CONFIGSTRING)) {
        isReadError = false;
    }
    return !isReadError;
}

Launcher::Launcher()
    : m_domTree(0)
    , m_mainWindow(0)
{
    // debug build crashed without UA.
    const char* str = "Mozilla/5.0 (BlackBerry; U; BlackBerry 9800; zh-CN) AppleWebKit/534.1+ (KHTML, like Gecko) Version/6.0.0.246 Mobile Safari/534.1+";
    Olympia::WebKit::WebSettings::globalSettings()->setUserAgentString(str);

    //read xml data to determine
    QString file = QCoreApplication::applicationDirPath() + "/config/olympiaMobile.xml";
    if (readConfigFile(file)) { // read nothing to use default config
        BrowserWidgetsFactory::getInstance()->setDomDocument(*m_domTree);
        m_mainWindow = BrowserWidgetsFactory::getInstance()->createMainWindow(0);
    }
}

Launcher::~Launcher()
{
}

QMainWindow* Launcher::getMainWindow()
{
    return m_mainWindow ? m_mainWindow : new QMainWindow();
}

} // namespace Browser
} // namespace Olympia

