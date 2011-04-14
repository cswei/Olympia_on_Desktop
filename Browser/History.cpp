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

#include "History.h"

#include "Constant.h"
#include <QBuffer>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QIcon>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QUrl>
#include <QVariant>

namespace Olympia {
namespace Browser {

extern void createPathIfNeeded(const QString& path);

QByteArray QIconToQBytesArray(const QIcon& icon)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    icon.pixmap(16).save(&buffer, "PNG");
    Q_ASSERT(bytes.size() > 0);

    return bytes;
}
/////////////////////////////////////////////////////////////////////////////
History::History(QObject* parent)
    : QObject(parent)
    , m_placeDB()
    , m_machedModel(new QSqlQueryModel())
{
    createPathIfNeeded(BASEPATH);

    m_placeDB = QSqlDatabase::addDatabase("QSQLITE");
    m_placeDB.setDatabaseName(HISTORY_DB_FILEPATH);
    if (!m_placeDB.open()) {
        qDebug() << "Failed to open database: " << m_placeDB.databaseName();
        return;
    }

    QSqlQuery q("select * from sqlite_master where type='table' and name='places'");
    if (!q.next()) {
        // database needs to be initiated.
        m_placeDB.exec("CREATE TABLE places (id INTEGER PRIMARY KEY, url LONGVARCHAR, title LONGVARCHAR, visit_count INTEGER DEFAULT 1, favicon_id INTEGER DEFAULT 1, last_visit_date INTEGER);");
        m_placeDB.exec("CREATE TABLE favicons (id INTEGER PRIMARY KEY, icon_url LONGVARCHAR UNIQUE, data BLOB);");
        m_placeDB.exec("CREATE TABLE historyvisits (id INTEGER PRIMARY KEY,  place_id INTEGER, visit_date INTEGER);");

        // insert the default icon
        QIcon icon(DEFAULT_ICON_FILEPATH);
        QSqlQuery sqlInsIcon(m_placeDB);
        sqlInsIcon.prepare("insert into favicons (id, icon_url, data) "
            "values (NULL, :icon_url, :data)");
        sqlInsIcon.bindValue(0, DEFAULT_ICON_FILEPATH);
        sqlInsIcon.bindValue(1, QIconToQBytesArray(icon));
        if (!sqlInsIcon.exec()) {
            qDebug() << "insert favicon error :" << sqlInsIcon.lastError();
            qDebug() << "original sql: " << sqlInsIcon.lastQuery();
        }
    }
    m_placeDB.close();
}

void History::clear()
{
    if (!m_placeDB.isValid() || !m_placeDB.open()) {
        qDebug() << "Failed to open database: " << m_placeDB.databaseName();
        return;
    }
    m_placeDB.exec("delete from places;");
    m_placeDB.exec("delete from favicons where id > 1;");  // we don't need to delete the default icon.
    m_placeDB.exec("delete from historyvisits;");

    m_placeDB.close();
}

void History::slotAddHistoryUrl(const QString& url)
{
    if (url.isNull())
        return;

    if (!m_placeDB.isValid() || !m_placeDB.open()) {
        qDebug() << "Failed to open database: " << m_placeDB.databaseName();
        return;
    }

    // deal with places
    int placeID(0);
    QSqlQuery sqlQueryPlace("select id from places where url='" + url +"'", m_placeDB);
    uint currentTime = QDateTime::currentDateTime().toTime_t();
    if (!sqlQueryPlace.next()) { // first time to load this url.
        QSqlQuery sqlInsPlace(m_placeDB);
        sqlInsPlace.prepare("insert into places (id, url, visit_count, last_visit_date) "
            "values (NULL, :url, :visit_count, :last_visit_date)");
        sqlInsPlace.bindValue(0, url);
        sqlInsPlace.bindValue(1, 1);
        sqlInsPlace.bindValue(2, currentTime);
        if (!sqlInsPlace.exec()) {
            qDebug() << "insert place error :" << sqlInsPlace.lastError();
            qDebug() << "original sql: " << sqlInsPlace.lastQuery();
            m_placeDB.close();
            return;
        }
        QSqlQuery sqlRequeryPlaceID("select id from places where url='" + url + "'", m_placeDB);
        if (!sqlRequeryPlaceID.next()) {
            qDebug() << "requery place error :" << sqlRequeryPlaceID.lastError();
            qDebug() << "original sql: " << sqlRequeryPlaceID.lastQuery();
            m_placeDB.close();
            return;
        }
        placeID = sqlRequeryPlaceID.value(0).toInt();
    }
    else {
        placeID = sqlQueryPlace.value(0).toInt();
        QSqlQuery sqlUpPlace(m_placeDB);
        sqlUpPlace.prepare("update places set "
            "visit_count=(select visit_count from places where id = :id)+1, last_visit_date=:last_visit_date "
            "where id = :id");
        sqlUpPlace.bindValue(0, placeID);
        sqlUpPlace.bindValue(1, currentTime);
        sqlUpPlace.bindValue(2, placeID);
        if (!sqlUpPlace.exec()) {
            qDebug() << "update place error :" << sqlUpPlace.lastError();
            qDebug() << "original sql: " << sqlUpPlace.lastQuery();
            m_placeDB.close();
            return;
        }
    }

    if (placeID <= 0) {
        m_placeDB.close();
        return;
    }

    // deal with history
    QSqlQuery sqlInsHistory(m_placeDB);
    sqlInsHistory.prepare("insert into historyvisits (id, place_id, visit_date) "
        "values (NULL, :place_id, :visit_date)");
    sqlInsHistory.bindValue(0, placeID);
    sqlInsHistory.bindValue(1, currentTime);
    if (!sqlInsHistory.exec()) {
        qDebug() << "insert history error :" << sqlInsHistory.lastError();
        qDebug() << "original sql: " << sqlInsHistory.lastQuery();
    }

    m_placeDB.close();
}

void History::slotAddTitle(const QString& url, const QString& title)
{
    if (url.isNull() || title.isNull() || url == title)
        return;

    if (!m_placeDB.isValid() || !m_placeDB.open()) {
        qDebug() << "Failed to open database: " << m_placeDB.databaseName();
        return;
    }

    // update the title of the recorded url.
    QSqlQuery sqlUpPlace(m_placeDB);
    sqlUpPlace.prepare("update places set title = :title where url = :url");
    sqlUpPlace.bindValue(0, title);
    sqlUpPlace.bindValue(1, url);
    if (!sqlUpPlace.exec()) {
        qDebug() << "update url title error :" << sqlUpPlace.lastError();
        qDebug() << "original sql: " << sqlUpPlace.lastQuery();
    }

    m_placeDB.close();
}

void History::slotAddIcon(const QString& url, const QString& iconUrl, const QIcon& icon)
{
    if (url.isNull() || iconUrl.isNull() || icon.isNull())
        return;

    if (!m_placeDB.isValid() || !m_placeDB.open()) {
        qDebug() << "Failed to open database: " << m_placeDB.databaseName();
        return;
    }

    // query and save the icon if needed.
    int faviconID(0);
    QSqlQuery sqlQueryIcon("select id from favicons where icon_url='" + iconUrl + "'", m_placeDB);
    if (!sqlQueryIcon.next()) {
        QSqlQuery sqlInsIcon(m_placeDB);
        sqlInsIcon.prepare("insert into favicons (id, icon_url, data) "
            "values (NULL, :icon_url, :data)");
        sqlInsIcon.bindValue(0, iconUrl);
        sqlInsIcon.bindValue(1, QIconToQBytesArray(icon));
        if (!sqlInsIcon.exec()) {
            qDebug() << "insert favicon error :" << sqlInsIcon.lastError();
            qDebug() << "original sql: " << sqlInsIcon.lastQuery();
            m_placeDB.close();
            return;
        }

        // then update the places table.
        QSqlQuery sqlGetIconID("select id from favicons where icon_url='" + iconUrl + "'", m_placeDB);
        if (!sqlGetIconID.next()) {
            qDebug() << "requery favicon_id error :" << sqlGetIconID.lastError();
            qDebug() << "original sql: " << sqlGetIconID.lastQuery();
            m_placeDB.close();
            return;
        }
        faviconID = sqlGetIconID.value(0).toInt();
    }
    else {
        faviconID = sqlQueryIcon.value(0).toInt();
    }
        

    QSqlQuery sqlUpPlace(m_placeDB);
    sqlUpPlace.prepare("update places set favicon_id = :favicon_id where url = :url");
    sqlUpPlace.bindValue(0, faviconID);
    sqlUpPlace.bindValue(1, url);
    if (!sqlUpPlace.exec()) {
        qDebug() << "update favicon_id in place error :" << sqlUpPlace.lastError();
        qDebug() << "original sql: " << sqlUpPlace.lastQuery();
    }

    m_placeDB.close();
}

void History::slotSearchHistoryItem(const QString& word)
{
    if (!m_placeDB.isValid())
        return;

    if (!m_placeDB.open()) {
        qDebug() << "Failed to open database: " << m_placeDB.databaseName();
        return;
    }

    QSqlQuery sqlQueryPlace("select data, title , url from places, favicons where url like '%"
        + word
        + "%' and places.favicon_id=favicons.id order by visit_count desc, last_visit_date desc"
        , m_placeDB);

    m_machedModel->setQuery(sqlQueryPlace);

    emit sigUpdateMatchedHistItems();

    m_placeDB.close();
}

} // namespace Browser
} // namespace Olympia
