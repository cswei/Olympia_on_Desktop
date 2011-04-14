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

#include "config.h"
#include "OlympiaPlatformClient.h"

#include <stdio.h>
#include "NotImplemented.h"
#include "OlympiaHttpStreamQt.h"
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QStringList>
#include <QUrl>

namespace Olympia {

namespace Platform {

//#define DEBUG_PRINT(fmt, args...) fprintf(stderr, fmt, ##args)
#define DEBUG_PRINT(fmt, args, ...)

class ClientImpQt: public Client
{
public:
    void reportOutOfMemory() {
        notImplemented();
    }

    void reportLowMemory() {
        notImplemented();
    }

    bool getCookieString(int, const char* url, UnsharedArray<char>& result) {
        QNetworkAccessManager* manager = OlympiaHttpStreamQt::getNetworkMgrInstance();
        QNetworkCookieJar* jar = manager->cookieJar();
        if (!jar) {
            return false;
        }

        QUrl u(url);
        QList<QNetworkCookie> cookies = jar->cookiesForUrl(u);
        if (cookies.isEmpty()) {
            return false;
        }

        QStringList resultCookies;
        QList<QNetworkCookie>::Iterator it = cookies.begin();
        while (it != cookies.end()) {
            resultCookies.append(QString::fromAscii(
                                 it->toRawForm(QNetworkCookie::NameAndValueOnly).constData()));
            it++;
        }
        if(resultCookies.size() < 0)
            return false;

        QByteArray b = resultCookies.join(QLatin1String("; ")).toUtf8();
        char* c = b.data();
        int length = strlen(c) + 1;
        char* cRes = new char[length];
        memcpy(cRes, c, length);
        result.reset(cRes);
        DEBUG_PRINT("\n--getCookieString-- the url is %s \nthe cookies are %s\n", url, cRes);
        return true;
    }

    bool setCookieString(int, const char* url, const char* cookieString) {
        QNetworkAccessManager* manager = OlympiaHttpStreamQt::getNetworkMgrInstance();
        QNetworkCookieJar* jar = manager->cookieJar();
        if (!jar)
            return false;
        QUrl u(url);
        QList<QNetworkCookie> cookies = QNetworkCookie::parseCookies(QString(cookieString).toAscii());
        jar->setCookiesFromUrl(cookies, u);
        DEBUG_PRINT("\n--setCookieString-- the url is %s \nthe cookies are %s\n", url, cookieString);
        return true;
    }

    bool createGeoTracker(int, bool, int, int) {
        notImplemented();
        return false;
    }

    void destroyGeoTracker(int) {
        notImplemented();
    }

    void suspendGeoTracker(int) {
        notImplemented();
    }

    void resumeGeoTracker(int) {
        notImplemented();
    }

    void willRunNestedEventLoop() {
        notImplemented();
    }

    void processNextMessage() {
        notImplemented();
    }

    void scheduleCallOnMainThread(void(*callback)(void)) {
        notImplemented();
    }

    void logEvent(unsigned int, const char*) {
        notImplemented();
    }
};

Client* Client::get()
{
    static ClientImpQt s_client;
    return &s_client;
}

void Client::set(Client* client)
{
    //FIXME: we don't need set client.
    notImplemented();
    //s_client = client;
}

} // Platform

} // Olympia
