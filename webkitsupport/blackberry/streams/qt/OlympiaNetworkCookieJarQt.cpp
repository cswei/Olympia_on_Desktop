/*
 * Copyright (C) 2010 Torch Mobile(Beijing) CO. Ltd. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "OlympiaNetworkCookieJarQt.h"

#include <stdio.h>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QLatin1String>
#include <QUrl>

namespace Olympia {

namespace Platform {

//#define DEBUG_PRINT(fmt, args...) fprintf(stderr, fmt, ##args)
#define DEBUG_PRINT(fmt, args, ...)

static const QString BASEPATH = QDir::homePath() + "/.OlympiaBrowser";
static const QString COOKIEPATH = BASEPATH + "/cookies";

static QDataStream& operator>>(QDataStream& in, QNetworkCookie& networkCookie)
{
    QByteArray name;
    QByteArray value;
    QDateTime date;
    QString domain;
    QString path;
    bool secure;
    in >> name >> value >> date >> domain >> path >> secure;
    networkCookie.setName(name);
    networkCookie.setValue(value);
    networkCookie.setExpirationDate(date);
    networkCookie.setDomain(domain);
    networkCookie.setPath(path);
    networkCookie.setSecure(secure);
    return in;
}

static QDataStream& operator<<(QDataStream& out, const QNetworkCookie& networkCookie)
{
    out << networkCookie.name()
        << networkCookie.value()
        << networkCookie.expirationDate()
        << networkCookie.domain()
        << networkCookie.path()
        << networkCookie.isSecure();
    return out;
}

static QLatin1String generateFileName(const QNetworkCookie& networkCookie)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(networkCookie.toRawForm());
    return QLatin1String(hash.result().toHex());
}

static bool writeCookieToDisk(const QNetworkCookie& networkCookie)
{
    if(networkCookie.domain().isEmpty() || networkCookie.path().isEmpty())
       return false;

    QFile cookiefile(COOKIEPATH + "/" + generateFileName(networkCookie));
    if(cookiefile.exists())
        cookiefile.remove();
    if(!cookiefile.open(QIODevice::WriteOnly)) {
        DEBUG_PRINT("\nERROR: can not write file of %s\n", cookiefile.fileName().toUtf8().data());
        return false;
    }
    QDataStream out(&cookiefile);
    out << networkCookie;
    cookiefile.close();
    DEBUG_PRINT("\nthe file of %s is saved to disk\n", cookiefile.fileName().toUtf8().data());
    return true;
}

static bool deleteCookiefromDisk(const QNetworkCookie& networkCookie)
{
    if(networkCookie.domain().isEmpty() || networkCookie.path().isEmpty())
       return false;

    QFile cookiefile(COOKIEPATH + "/" + generateFileName(networkCookie));
    if(cookiefile.exists())
        cookiefile.remove();
    return true;
}

static unsigned int calculateSize(const QList<QNetworkCookie>& cookieList)
{
    unsigned int size = 0;
    for(int i = 0; i < cookieList.size(); i++) {
        size += cookieList.at(i).toRawForm().size();
    }
    return size;
}

OlympiaNetworkCookieJarQt::OlympiaNetworkCookieJarQt(unsigned int capacity)
    : QNetworkCookieJar(0)
    , m_capacity(capacity)
{
    QDir cookieDir(COOKIEPATH);
    if(!cookieDir.exists())
        cookieDir.mkpath(COOKIEPATH);
    readCookiesFromDisk();
    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotSaveCookiesToDisk()));
}

OlympiaNetworkCookieJarQt::~OlympiaNetworkCookieJarQt()
{

}

bool OlympiaNetworkCookieJarQt::readCookiesFromDisk()
{
    QDir cookieDir(COOKIEPATH);
    if(!cookieDir.exists()) {
        cookieDir.mkpath(COOKIEPATH);
        return false;
    }

    m_cookieList.clear();
    cookieDir.setFilter(QDir::Files);
    cookieDir.setSorting(QDir::Time | QDir::Reversed);
    QFileInfoList list = cookieDir.entryInfoList();
    unsigned int loadSize = 0;
    for(int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        DEBUG_PRINT("the file name is %s\n",fileInfo.fileName().toUtf8().data());

        // if the size is larger than the capacity, we will remove cookies from disk
        loadSize += fileInfo.size();
        if(loadSize >= m_capacity) {
            cookieDir.remove(fileInfo.fileName());
            continue;
        }

        // read cookies
        QFile cookiefile(COOKIEPATH + "/" + fileInfo.fileName());
        if(!cookiefile.open(QIODevice::ReadOnly)) {
            DEBUG_PRINT("\nERROR: can not read file of %s\n", cookiefile.fileName());
            continue;
        }
        QDataStream in(&cookiefile);
        QNetworkCookie networkCookie;
        in >> networkCookie;
        QDateTime now = QDateTime::currentDateTime();
        if(networkCookie.expirationDate() > now) {
            m_cookieList.append(networkCookie);
            cookiefile.close();
        } else {
            cookiefile.close();
            deleteCookiefromDisk(networkCookie);
        }
    }
    setAllCookies(m_cookieList);
    return true;
}

unsigned int OlympiaNetworkCookieJarQt::purgeStorage(unsigned int requireSize)
{
    unsigned int reduceSize = 0;
    bool isChanged = false;
    QDateTime now = QDateTime::currentDateTime();
    QList<QNetworkCookie>::Iterator it = m_cookieList.begin();
    QList<QNetworkCookie>::Iterator end = m_cookieList.end();
    for ( ; it != end; ++it) {
        if(it->expirationDate() < now) {
            reduceSize += it->toRawForm().size();
            m_cookieList.erase(it);
            isChanged = true;
        }
    }

    if(reduceSize < requireSize) {
        it = m_cookieList.begin();
        end = m_cookieList.end();
        for ( ; it != end; ++it) {
            reduceSize += it->toRawForm().size();
            m_cookieList.erase(it);
            isChanged = true;
            if(reduceSize >= requireSize)
                break;
        }
    }
    if(isChanged)
        setAllCookies(m_cookieList);
    return reduceSize;
}

bool OlympiaNetworkCookieJarQt::setCookiesFromUrl(const QList<QNetworkCookie>& cookieList, const QUrl &url)
{
    unsigned int requireSize = calculateSize(cookieList);
    unsigned int currentSize = calculateSize(m_cookieList);
    if(requireSize + currentSize > m_capacity) {
        if(purgeStorage(requireSize) < requireSize)
            return false;
    }
    return QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
}

void OlympiaNetworkCookieJarQt::slotSaveCookiesToDisk()
{
    QList<QNetworkCookie> cookies = allCookies();
    QList<QNetworkCookie> willAddCookiesList;
    QList<QNetworkCookie> willDeleteCookiesList;
    int i = 0;
    int j = 0;
    while(true) {
        if(i == m_cookieList.size()) {
            for(int k = j; k < cookies.size(); k++)
                willAddCookiesList.append(cookies.at(k));
            break;
        }

        if(j == cookies.size()) {
            for(int k = i; k < m_cookieList.size(); k++)
                willDeleteCookiesList.append(m_cookieList.at(k));
            break;
        }

        if (m_cookieList.at(i) != cookies.at(j)) {
            willDeleteCookiesList.append(m_cookieList.at(i));
            i++;
            continue;
        }
        i++;
        j++;
    }

    for(int i = 0; i < willDeleteCookiesList.size(); i++)
        deleteCookiefromDisk(willDeleteCookiesList.at(i));

    for(int i = 0; i < willAddCookiesList.size(); i++) {
        QNetworkCookie c = willAddCookiesList.at(i);
        QDateTime now = QDateTime::currentDateTime();
        if(!c.isSessionCookie() && c.expirationDate() > now)
           writeCookieToDisk(willAddCookiesList.at(i));
    }
}

} // namespace Platform

} // namespace Olympia
