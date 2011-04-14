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

#include "AddressBar.h"

#include "Constant.h"
#include "MatchedHistView.h"
#include <QDebug>
#include <QMovie>
#include <QPixmap>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QRect>
#include <QResizeEvent>
#include <QSqlQueryModel>
#include <QStyleOptionFrameV2>
#include <QUrl>

namespace Olympia {
namespace Browser {

static const int INNER_OFFSET = 5;
static int ADDRESSBAR_ICON_SIZE = 0;

///////////////////////////////////////////////////////////////////////////////
ClearButton::ClearButton(QWidget* parent)
  : QAbstractButton(parent)
{
    setVisible(false);
    setFocusPolicy(Qt::NoFocus);
}

void ClearButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    int height = this->height();

    painter.setRenderHint(QPainter::Antialiasing, true);
    QColor color = palette().color(QPalette::Mid);
    painter.setBrush(isDown()
                     ? palette().color(QPalette::Dark)
                     : palette().color(QPalette::Mid));
    painter.setPen(painter.brush().color());
    int size = width();
    int offset = size / 5;
    int radius = size - offset * 2;
    painter.drawEllipse(offset, offset, radius, radius);

    painter.setPen(palette().color(QPalette::Base));
    int border = offset * 2;
    painter.drawLine(border, border, width() - border, height - border);
    painter.drawLine(border, height - border, width() - border, border);
}

void ClearButton::textChanged(const QString& text)
{
    setVisible(!text.isEmpty());
}

///////////////////////////////////////////////////////////////////////////////
void ExLineEdit::focusOutEvent(QFocusEvent* event)
{
    emit sigFocusOut();
    QLineEdit::focusOutEvent(event);
}

///////////////////////////////////////////////////////////////////////////////
const int HISTORYLIST_GAP = 3;
AddressBar::AddressBar(QWidget* w)
    : QFrame(w)
    , m_urlIconLabel(new QLabel(this))
    , m_loadingMovie(new QMovie(LOADING_ICON_FILEPATH))
    , m_lineEdit(new ExLineEdit(this))
    , m_clearBtn(new ClearButton(this))
    , m_matchedHistView(new MatchedHistView(w))
{
    setFocusPolicy(m_lineEdit->focusPolicy());
    setSizePolicy(m_lineEdit->sizePolicy());
    setBackgroundRole(m_lineEdit->backgroundRole());
    setPalette(m_lineEdit->palette());

    m_lineEdit->installEventFilter(this);
    m_lineEdit->setFrame(false);
    m_lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    QPalette clearPalette = m_lineEdit->palette();
    clearPalette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    m_lineEdit->setPalette(clearPalette);
    connect(m_lineEdit, SIGNAL(textEdited(const QString&)),
        this, SIGNAL(sigMatchInput(const QString&)));
    connect(m_lineEdit, SIGNAL(textEdited(const QString&)),
        this, SLOT(slotTextEdited(const QString&)));
    connect(m_lineEdit, SIGNAL(returnPressed()),
        this, SLOT(slotReturnPressedInAddrBar()));

    QIcon defaultIcon = QIcon(DEFAULT_ICON_FILEPATH);
    ADDRESSBAR_ICON_SIZE = height() - INNER_OFFSET * 2;
    m_urlIconLabel->setPixmap(defaultIcon.pixmap(ADDRESSBAR_ICON_SIZE));
    m_urlIconLabel->setAlignment(Qt::AlignCenter);

    connect(m_clearBtn, SIGNAL(clicked()),
        this, SLOT(slotClearContents()));
    connect(m_lineEdit, SIGNAL(textChanged(QString)),
        m_clearBtn, SLOT(textChanged(QString)));

    m_matchedHistView->installEventFilter(this);
    m_matchedHistView->move(m_lineEdit->mapToParent(
        QPoint(m_lineEdit->pos().x() + HISTORYLIST_GAP, m_lineEdit->pos().y()+m_lineEdit->height())));
    m_matchedHistView->hide();
    connect(m_matchedHistView, SIGNAL(sigItemSelected(const QString&)),
        this, SLOT(slotLaunchURL(const QString&)));
    connect(m_lineEdit, SIGNAL(sigFocusOut(void)),
        m_matchedHistView, SLOT(slotHideSelfIfNeeded(void)));
}

AddressBar::~AddressBar()
{

}

void AddressBar::setUrlText(const QString& url)
{
    if(m_lineEdit) {
        m_lineEdit->setText(QUrl::fromEncoded(url.toLatin1()).toString());
        m_lineEdit->home(false);
    }
}

void AddressBar::slotSetUrlIcon(const QString&, const QString&, const QIcon& icon)
{
    m_loadingMovie->stop();
    m_urlIconLabel->setPixmap(icon.pixmap(ADDRESSBAR_ICON_SIZE));
}

void AddressBar::slotLoadFinished()
{
    // to be implemented. perhaps we gonna need this function in the future.
}

void AddressBar::slotUseDefaultUrlIcon()
{
    m_loadingMovie->stop();
    QIcon defaultIcon = QIcon(DEFAULT_ICON_FILEPATH);
    m_urlIconLabel->setPixmap(defaultIcon.pixmap(ADDRESSBAR_ICON_SIZE));
}

void AddressBar::slotClearContents()
{
    m_lineEdit->clear();
    QIcon defaultIcon = QIcon(DEFAULT_ICON_FILEPATH);
    m_urlIconLabel->setPixmap(defaultIcon.pixmap(ADDRESSBAR_ICON_SIZE));
}

void AddressBar::keyPressEvent(QKeyEvent* event)
{
    m_matchedHistView->keyPressEvent(event);
    QWidget::keyPressEvent(event);
}

void AddressBar::resizeEvent(QResizeEvent* event)
{
    updateSize(event->size());
    QWidget::resizeEvent(event);
}

void AddressBar::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QStyleOptionFrameV2 panel;
    initStyleOption(panel);
    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);
}

void AddressBar::initStyleOption(QStyleOptionFrameV2& option)
{
    option.initFrom(this);
    option.rect = contentsRect();
    option.lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &option, this);
    option.midLineWidth = 0;
    option.state |= QStyle::State_Sunken;
    option.features = QStyleOptionFrameV2::None;
}

void AddressBar::updateSize(const QSize& size)
{
    setGeometry(x(), y(), size.width(), size.height());

    ADDRESSBAR_ICON_SIZE = size.height() - INNER_OFFSET * 2;
    m_urlIconLabel->setGeometry(INNER_OFFSET, (size.height() - ADDRESSBAR_ICON_SIZE) / 2,
        ADDRESSBAR_ICON_SIZE, ADDRESSBAR_ICON_SIZE);
    m_lineEdit->setGeometry(INNER_OFFSET + m_urlIconLabel->width(), 0,
        size.width() - m_urlIconLabel->width() * 2 - INNER_OFFSET, size.height());
    m_clearBtn->setGeometry(INNER_OFFSET + m_urlIconLabel->width() + m_lineEdit->width(), m_urlIconLabel->y(),
        m_urlIconLabel->width(), m_urlIconLabel->height());
    m_matchedHistView->move(HISTORYLIST_GAP, y() + size.height());
    m_matchedHistView->setFixedWidth(size.width() - HISTORYLIST_GAP * 2);
}

void AddressBar::slotReturnPressedInAddrBar()
{
    slotLaunchURL(m_lineEdit->text());
}

void AddressBar::slotLaunchURL(const QString& url)
{
    m_matchedHistView->hide();
    m_lineEdit->setText(url);
    m_urlIconLabel->setMovie(m_loadingMovie);
    m_loadingMovie->start();
    emit sigGo(url);
}

void AddressBar::slotTextEdited(const QString&)
{
    // keep the matchedListView on the top.
    m_matchedHistView->raise();

    // set the matchedHistView's height automatically.
    QRect indexRect = m_matchedHistView->visualRect(
        m_matchedHistView->model()->index(m_matchedHistView->model()->rowCount() - 1, 0));
    m_matchedHistView->setFixedHeight(indexRect.y() + indexRect.height() + 5);
}

void AddressBar::setMatchedHistModel(QSqlQueryModel* model)
{
    m_matchedHistView->setModel(model);
}

void AddressBar::slotUpdateMachedHistItems()
{
    m_matchedHistView->UpdateMachedHistItems();
}

} // namespace Browser
} // namespace Olympia

