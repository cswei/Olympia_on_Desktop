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

#include "TabThumbButton.h"

// #include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

// Tune the UI by modified these factors:
static const int MarginWidth = 8;
static const int CloseBtnMarginWidth = 4;
static const int SpacingWidth = 10;
static const int TextHeight = 16;
static const int RectRadius = 6;

PixmapButton::PixmapButton(QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent)
{
}

void PixmapButton::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
    event->accept();
}

void PixmapButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    emit mouseReleased();
    QGraphicsItem::mouseReleaseEvent(event);
    event->accept();
}

TabThumbButton::TabThumbButton(const QString& title, const QPixmap& pixmap, QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    m_size = pixmap.size() + QSize(MarginWidth*2, MarginWidth*2 + SpacingWidth + TextHeight);
    PixmapButton* thumbBtn = new PixmapButton(this);
    thumbBtn->setPixmap(pixmap);
    thumbBtn->setX(MarginWidth);
    thumbBtn->setY(MarginWidth);
    connect(thumbBtn, SIGNAL(mouseReleased()), this, SLOT(onThumbBtnReleased()));

    PixmapButton* closeBtn = new PixmapButton(this);
    closeBtn->setPixmap(QPixmap("./images/close.png"));
    closeBtn->setY(CloseBtnMarginWidth);
    closeBtn->setX(m_size.width() - CloseBtnMarginWidth - closeBtn->boundingRect().width());
    connect(closeBtn, SIGNAL(mouseReleased()), this, SLOT(onCloseBtnReleased()));

    QGraphicsSimpleTextItem* titleItem = new QGraphicsSimpleTextItem(this);
    // FIXME: adjust font.
    // FIXME: truncate string by width not by string length.
    titleItem->setText(title.left(8));
    titleItem->setX(MarginWidth);
    titleItem->setY(m_size.height() -  MarginWidth - TextHeight);
}

void TabThumbButton::onCloseBtnReleased()
{
    emit toBeClosed(this);
}

void TabThumbButton::onThumbBtnReleased()
{
    emit toBeSwitchedTo(this);
}
QRectF TabThumbButton::boundingRect() const
{
    return QRectF(QPointF(0, 0), m_size);
}

void TabThumbButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
    painter->save();
    QPen pen(QColor("#3C499E"));
    pen.setWidth(1);
    painter->setPen(pen);
    QLinearGradient g(0, 0, 0, m_size.height());
    g.setColorAt(0, QColor("#303F96"));
    g.setColorAt(1, QColor("#89C4F5"));
    painter->setBrush(g);

    painter->drawRoundedRect(0, 0, m_size.width(), m_size.height(), RectRadius, RectRadius);
    painter->restore();
}

