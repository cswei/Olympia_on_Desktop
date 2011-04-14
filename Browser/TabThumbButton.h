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

#ifndef TabThumbButton_H
#define TabThumbButton_H

#include <QGraphicsItem>

// FIXME: Different apperance for normal, hover and pressed?

class PixmapButton : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    enum { Type = UserType + 2 };
    int type() const { return Type; }
    
    PixmapButton(QGraphicsItem *parent = 0);
signals:
    void mouseReleased();
protected:
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
};

class TabThumbButton : public QGraphicsObject
{
    Q_OBJECT
public:
    enum { Type = UserType + 1 };
    int type() const { return Type; }
    
    TabThumbButton(const QString& title, const QPixmap& pixmap, QGraphicsItem *parent = 0);
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

signals:
    void toBeClosed(TabThumbButton*);
    void toBeSwitchedTo(TabThumbButton*);

public slots:
    void onCloseBtnReleased();
    void onThumbBtnReleased();

private:
    QSize m_size;
};

#endif // TabThumbButton_H
