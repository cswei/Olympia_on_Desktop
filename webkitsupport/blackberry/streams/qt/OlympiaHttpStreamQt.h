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

#ifndef OlympiaHttpStreamQt_h
#define OlympiaHttpStreamQt_h

#include "IStream.h"
#include "NetworkRequest.h"

#include <QNetworkRequest>
#include <QObject>
#include <QFile>
#include <QIODevice>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QSslError;
template <> class QList<QSslError>;

namespace Olympia {

    namespace Platform {

        class NetworkRequest;

        class OlympiaHttpStreamQt : public QObject, public IStream {
            Q_OBJECT
        public:
            static QNetworkAccessManager* getNetworkMgrInstance();
            OlympiaHttpStreamQt();
            virtual ~OlympiaHttpStreamQt();
            void setRequest(const NetworkRequest&);
            int open();
            int cancel();
        private slots:
            void slotError(QNetworkReply::NetworkError);
            void slotSslErrors(const QList<QSslError>& error);
            void slotFinish();
            void slotDataReceived();
        private:
            void notifyStatusAndHeardsIfNeeded();

            QNetworkReply* m_reply;
            const NetworkRequest* m_request;
            QNetworkAccessManager::Operation m_method;
            bool m_isStausNotified;
        };

        class FormDataIODevice : public QIODevice {
            Q_OBJECT
        public:
            FormDataIODevice(const NetworkRequest::FormDataList& formDataList);
            ~FormDataIODevice();

            bool isSequential() const;

        protected:
            qint64 readData(char*, qint64);
            qint64 writeData(const char*, qint64);

        private:
            void moveToNextElement();

        private:
            NetworkRequest::FormDataList m_formElements;
            QFile* m_currentFile;
            qint64 m_currentDelta;
        };

    } // namespace Platform

} // namespace Olympia

#endif // OlympiaHttpStreamQt_h
