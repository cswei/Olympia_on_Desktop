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

#include "MatchedHistView.h"

#include <QDebug>
#include <QListWidgetItem>
#include <QPainter>
#include <QSqlRecord>
#include <QSqlQueryModel>

namespace Olympia {
namespace Browser {

MatchedHistView::MatchedHistView(QWidget* parent)
    : QListWidget(parent)
    , m_machedModel(0)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setUniformItemSizes(true);
    setTextElideMode(Qt::ElideMiddle);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setFocusPolicy(Qt::WheelFocus);

    connect(this, SIGNAL(activated(QModelIndex)),
        this, SLOT(slotItemActivated(QModelIndex)));
}

void MatchedHistView::UpdateMachedHistItems()
{
    Q_ASSERT(m_machedModel);

    clear();

    int count = m_machedModel->rowCount();
    if (count == 0) {
        hide();
        return;
    }

    const int maxHistItemNum = 10;
    if (count > maxHistItemNum) // set the history items limitation.
        count = maxHistItemNum;

    for (int i = 0; i < count; i++) {
        HistoryItem* item = new HistoryItem(this);
        QSqlRecord rec = m_machedModel->record(i);
        QPixmap pixmap;
        pixmap.loadFromData(rec.value(0).toByteArray());
        item->setIcon(QIcon(pixmap));
        item->setTitleAndUrl(rec.value(1).toString(), rec.value(2).toString());
    }
    show();
}

void MatchedHistView::setModel(QSqlQueryModel* model)
{
    m_machedModel = model;
}

void MatchedHistView::slotHideSelfIfNeeded()
{
    if (!hasFocus())
        hide();
}

void MatchedHistView::slotItemActivated(const QModelIndex& index)
{
    emit sigItemSelected(index.data().toString());
}

void MatchedHistView::keyPressEvent(QKeyEvent* event)
{
    QListWidget::keyPressEvent(event);
}

void MatchedHistView::focusOutEvent(QFocusEvent* event)
{
    QListWidget::focusOutEvent(event);
    hide();
}

////////////////////////////////////////////////////////////////////////////////
// FIXME! need to work out on this class to implements more complicated features of the history item display.
void HistoryItem::setTitleAndUrl(const QString& title, const QString& url)
{
    //setText(title + "\n" + url);
    setText(url);
}

} // namespace Browser
} // namespace Olympia
