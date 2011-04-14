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
#include "OlympiaHttpStreamQt.h"

#include "NetworkRequest.h"

#include "OlympiaNetworkCookieJarQt.h"
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QList>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QSslError>
#include <QTimer>
#include <QVariant>


static const QString BASEPATH = QDir::homePath() + "/.OlympiaBrowser";
static const QString CACHEPATH = BASEPATH + "/cache";

static QNetworkAccessManager* s_instance = 0;

QNetworkAccessManager* getNetworkManagerInstance()
{
    if(!s_instance) {
        s_instance = new QNetworkAccessManager();
        s_instance->setCookieJar(new Olympia::Platform::OlympiaNetworkCookieJarQt(10 * 1024 * 1024)); //10M

        // cache
        QDir cacheDir(CACHEPATH);
        if(!cacheDir.exists())
            cacheDir.mkpath(CACHEPATH);
        QNetworkDiskCache* diskCache = new QNetworkDiskCache(s_instance);
        diskCache->setCacheDirectory(CACHEPATH);
        diskCache->setMaximumCacheSize(30 * 1024 * 1024); //30M
        s_instance->setCache(diskCache);
    }
    return s_instance;
}

namespace Olympia {

namespace Platform {


#if QT_VERSION > QT_VERSION_CHECK(4, 6, 2)
#define SIGNAL_CONN Qt::DirectConnection
#else
#define SIGNAL_CONN Qt::QueuedConnection
#endif

//#define DEBUG_PRINT(fmt, args...) fprintf(stderr, fmt, ##args)
#define DEBUG_PRINT(fmt, args, ...)

FormDataIODevice::FormDataIODevice(const NetworkRequest::FormDataList& formDataList)
    : m_formElements(formDataList)
    , m_currentFile(0)
    , m_currentDelta(0)
{
    setOpenMode(FormDataIODevice::ReadOnly);
}

FormDataIODevice::~FormDataIODevice()
{
    delete m_currentFile;
}

void FormDataIODevice::moveToNextElement()
{
   if (m_currentFile)
        m_currentFile->close();
    m_currentDelta = 0;

    m_formElements.erase(m_formElements.begin());

    if (m_formElements.empty() || m_formElements.begin()->isData)
        return;

    if (!m_currentFile)
        m_currentFile = new QFile;
       const wchar_t* filename = m_formElements.begin()->filename.c_str();
       if (sizeof(wchar_t) == sizeof(QChar))
           m_currentFile->setFileName(QString::fromUtf16((const ushort*)(m_formElements.begin()->filename.c_str())));
       else
           m_currentFile->setFileName(QString::fromUcs4((const uint*)(m_formElements.begin()->filename.c_str())));

    m_currentFile->open(QFile::ReadOnly);
}

// refer to QT porting implementation
// m_formElements.begin is the current item. If the destination buffer is
// big enough we are going to read from more than one FormDataElement
qint64 FormDataIODevice::readData(char* destination, qint64 size)
{
   if (m_formElements.empty())
        return -1;

    qint64 copied = 0;
    while (copied < size && !m_formElements.empty()) {
        const NetworkRequest::FormDataChunk& element = *m_formElements.begin();
        const qint64 left = size - copied;

        if (element.isData) {
            const qint64 toCopy = qMin<qint64>(left, element.buf.size() - m_currentDelta);
            std::copy(element.buf.begin() + m_currentDelta, element.buf.begin() + m_currentDelta + toCopy, destination + copied);
            m_currentDelta += toCopy;
            copied += toCopy;

            if (m_currentDelta == element.buf.size())
                moveToNextElement();
        } else {
            const QByteArray data = m_currentFile->read(left);
            memcpy(destination + copied, data.constData(), data.size());
            copied += data.size();

            if (m_currentFile->atEnd() || !m_currentFile->isOpen())
                moveToNextElement();
        }
    }

    return copied;
}

qint64 FormDataIODevice::writeData(const char*, qint64)
{
    return -1;
}

bool FormDataIODevice::isSequential() const
{
    return true;
}

static QNetworkRequest toNetWorkRequest(const NetworkRequest& request) {
    QNetworkRequest req;
    req.setUrl(QUrl::fromEncoded(request.getUrlRef().data()));

    const NetworkRequest::HeaderList& headers = request.getHeaderListRef();
    for(int i = 0; i < headers.size(); i++) {
        const std::pair<std::string, std::string>& head = headers[i];
        QByteArray name = QString(head.first.data()).toAscii();
        QByteArray value = QString(head.second.data()).toAscii();
        // QNetworkRequest::setRawHeader() would remove the header if the value is null
        // Make sure to set an empty header instead of null header.
        if (!value.isNull())
            req.setRawHeader(name, value);
        else
            req.setRawHeader(name, "");
    }

    switch (request.getCachePolicy()) {
    case NetworkRequest::ReloadIgnoringCacheData:
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
        break;
    case NetworkRequest::ReturnCacheDataElseLoad:
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        break;
    case NetworkRequest::ReturnCacheDataDontLoad:
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
        break;
    case NetworkRequest::UseProtocolCachePolicy:
        // QNetworkRequest::PreferNetwork
    default:
        break;
    }

    return req;
}

QNetworkAccessManager* OlympiaHttpStreamQt::getNetworkMgrInstance()
{
    return getNetworkManagerInstance();
}

OlympiaHttpStreamQt::OlympiaHttpStreamQt()
    : QObject(0)
    , m_reply(0)
    , m_request(0)
    , m_isStausNotified(false)
{
    // impl later
    // 1. authentication
    // 2. proxy
    //connect(getNetworkMgrInstance(), SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
    //        this, SLOT(?????));
}

OlympiaHttpStreamQt::~OlympiaHttpStreamQt()
{

}

void OlympiaHttpStreamQt::setRequest(const NetworkRequest& request)
{
    m_request = &request;
    if (request.getMethodRef() == "GET")
        m_method = QNetworkAccessManager::GetOperation;
    else if (request.getMethodRef() == "HEAD")
        m_method = QNetworkAccessManager::HeadOperation;
    else if (request.getMethodRef() == "POST")
        m_method = QNetworkAccessManager::PostOperation;
    else if (request.getMethodRef() == "PUT")
        m_method = QNetworkAccessManager::PutOperation;
    else
        m_method = QNetworkAccessManager::UnknownOperation;
    m_isStausNotified = false;
}

int OlympiaHttpStreamQt::open()
{
    if(!m_request)
        return -1;
    QNetworkRequest request = toNetWorkRequest(*m_request);
    switch (m_method) {
    case QNetworkAccessManager::GetOperation:
        m_reply = getNetworkMgrInstance()->get(request);
        break;
    case QNetworkAccessManager::PostOperation: {
        FormDataIODevice* postDevice = new FormDataIODevice(m_request->getFormDataListRef());
        m_reply = getNetworkMgrInstance()->post(request, postDevice);
        postDevice->setParent(m_reply);
        break;
    }
    case QNetworkAccessManager::HeadOperation:
        m_reply = getNetworkMgrInstance()->head(request);
        break;
    case QNetworkAccessManager::PutOperation: {
        FormDataIODevice* putDevice = new FormDataIODevice(m_request->getFormDataListRef());
        m_reply = getNetworkMgrInstance()->put(request, putDevice);
        putDevice->setParent(m_reply);
        break;
    }
    case QNetworkAccessManager::UnknownOperation: {
        if(m_reply) {
            m_reply->deleteLater();
            m_reply = 0;
        }
        IStreamListener* listener = const_cast<IStreamListener*>(this->listener());
        if(listener) {
            listener->notifyOpen(400, "Bad HTTP request"); // bad request
            listener->notifyDone();
        }
        return -1;
    }
    }

    m_reply->setParent(this);

    if (m_reply->error() != QNetworkReply::NoError) {
        IStreamListener* listener = const_cast<IStreamListener*>(this->listener());
        QString message = m_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        listener->notifyOpen(-1, message.toAscii());
        QTimer::singleShot(0, this, SLOT(slotFinish()));
        return -1;
    }

    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(slotDataReceived()), SIGNAL_CONN);
    connect(m_reply, SIGNAL(finished()), this, SLOT(slotFinish()), SIGNAL_CONN);
    connect(m_reply, SIGNAL(sslErrors(const QList<QSslError>&)), SLOT(slotSslErrors(const QList<QSslError>&)));
    return 0;
}

void OlympiaHttpStreamQt::slotSslErrors(const QList<QSslError>& errors)
{
    if(!m_reply)
        return;
    // FIXME: we don't have a notification to let user choose continue or stop so far,
    // which may be implemented later with App framework layer.
    // IgnoreSslErrors means continue loading when it happens with SSL errors.
    m_reply->ignoreSslErrors(errors);
}

void OlympiaHttpStreamQt::slotError(QNetworkReply::NetworkError code)
{
    if(!m_reply)
        return;
    int httpStatusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString message = m_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    IStreamListener* listener = const_cast<IStreamListener*>(this->listener());
    if(listener) {
        listener->notifyOpen(-1, "Bad HTTP request"); // bad request
        m_isStausNotified = true;
    }
}

void OlympiaHttpStreamQt::slotFinish()
{
    if(!m_reply)
        return;

    QVariant fromCache = m_reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute);
    if(!fromCache.isNull())
        DEBUG_PRINT("%s is from cache ? --- %s\n", m_reply->url().toString().data(), fromCache.toBool() ? "Yes" : "NO");

    notifyStatusAndHeardsIfNeeded();

    IStreamListener* listener = const_cast<IStreamListener*>(this->listener());
    if(listener) {
        // will destory this instance??
        listener->notifyDone();
    }
}

void OlympiaHttpStreamQt::slotDataReceived()
{
    if(!m_reply)
        return;

    IStreamListener* listener = const_cast<IStreamListener*>(this->listener());
    if(listener) {
        notifyStatusAndHeardsIfNeeded();
        //data ---> notifyDataReceived
        QByteArray data = m_reply->read(m_reply->bytesAvailable());
        if(!data.isEmpty()) {
            listener->notifyDataReceived(data.data(), data.length());
        }
    }
}

int OlympiaHttpStreamQt::cancel()
{
    if(m_reply) {
        m_reply->abort();
        disconnect(m_reply, 0, this, 0);
        QCoreApplication::removePostedEvents(this, QEvent::MetaCall);
    }
    return 0 ;
}

void OlympiaHttpStreamQt::notifyStatusAndHeardsIfNeeded()
{
    IStreamListener* listener = const_cast<IStreamListener*>(this->listener());
    if (m_reply && listener && !m_isStausNotified) {
        //status ---> notifyOpen
        int httpStatusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString message = m_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        listener->notifyOpen(httpStatusCode, message.toLatin1().data());
        //headers ---> notifyHeaderReceived
        const QList<QByteArray>& headers = m_reply->rawHeaderList();
        for (int i = 0; i < headers.size(); i++) {
            const QByteArray &headerName = headers.at(i);
            listener->notifyHeaderReceived(headerName.data(), m_reply->rawHeader(headerName).data());
        }
        m_isStausNotified = true;
    }
}

} // namespace Platform

} // namespace Olympia
