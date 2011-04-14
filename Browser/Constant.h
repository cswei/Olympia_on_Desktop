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

#ifndef Constant_h
#define Constant_h

#include <QCoreApplication>
#include <QDir>

namespace Olympia {
namespace Browser {

// the format is className_typeName, className is the C++ source codes you implemeted,
// and typeName is defined in xml file.
static const QString OLYMPIADESKTOPMAINWINDOW_TYPEID = "mainwindow_olympiadesktop";
static const QString OLYMPIAMOBILEMAINWINDOW_TYPEID = "mainwindow_olympiamobile";
static const QString OLYMPIAMOBILEADDRESSBAR_TYPEID = "addressbar_olympiamobile";
static const QString OLYMPIADESKTOPTABS_TYPEID = "tabs_olympiadesktop";
static const QString OLYMPIAMOBILETABS_TYPEID = "tabs_olympiamobile";
static const QString OLYMPIAMOBILESTATUSBAR_TYPEID = "statusbar_olympiamobile";

static const QString BASEPATH = QDir::homePath() + QString("/.OlympiaBrowser");
static const QString CONF_FILEPATH = BASEPATH + QString("/config.ini");
static const QString HISTORY_DB_FILEPATH = BASEPATH + QString("/history.sqlite");
static const QString DEFAULT_COOCKIE_PATH = BASEPATH + QString("/cookies");
static const QString DEFAULT_CACHE_PATH = BASEPATH + QString("/cache");
extern QString APP_ICON_FILEPATH;
extern QString DEFAULT_ICON_FILEPATH;
extern QString LOADING_ICON_FILEPATH;

} // Browser
} // Olympia
#endif // Constant_h

