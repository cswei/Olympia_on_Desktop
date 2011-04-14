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

#ifndef MatchedHistView_h
#define MatchedHistView_h

#include <QListWidget>

class QSqlQueryModel;

namespace Olympia {
namespace Browser {

class MatchedHistView : public QListWidget
{
Q_OBJECT
public:
    MatchedHistView(QWidget* parent = 0);

    void UpdateMachedHistItems();
    void setModel(QSqlQueryModel* model);

signals:
    void sigItemSelected(const QString& url);

public slots:
    void slotHideSelfIfNeeded();

private slots:
    void slotItemActivated(const QModelIndex& index);

private:
    void keyPressEvent(QKeyEvent* event);
    void focusOutEvent(QFocusEvent* event);

    QSqlQueryModel* m_machedModel;
    
    friend class AddressBar;
};

// FIXME! need to work out on this class to implements more complicated features of the history item display.
class HistoryItem : public QListWidgetItem
{
public:
    HistoryItem(QListWidget* parent = 0):QListWidgetItem(parent) {}

    void setTitleAndUrl(const QString& title, const QString& url);

private:
    void setText(const QString& str) { QListWidgetItem::setText(str); } // disable setText() function from calling outside.

private:
    QString m_title;
    QString m_url;
};

} // namespace Browser
} // namespace Olympia

#endif // MatchedHistView_h
