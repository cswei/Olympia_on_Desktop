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

#include "Constant.h"
#include <QtGui/QApplication>
#include <QMainWindow>
#include <WebSettings.h>

using namespace Olympia::Browser;

QString Olympia::Browser::APP_ICON_FILEPATH = QString("/images/app.png");
QString Olympia::Browser::DEFAULT_ICON_FILEPATH = QString("/images/plainIcon.png");
QString Olympia::Browser::LOADING_ICON_FILEPATH = QString("/images/loading.gif");

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // construct the icon path after create QApplication object.
    APP_ICON_FILEPATH.prepend(QCoreApplication::applicationDirPath());
    DEFAULT_ICON_FILEPATH.prepend(QCoreApplication::applicationDirPath());
    LOADING_ICON_FILEPATH.prepend(QCoreApplication::applicationDirPath());

    Launcher launcher;
    QMainWindow* mainWindow = launcher.getMainWindow();
    app.setActiveWindow(mainWindow);
    mainWindow->show();
    return app.exec();
}
