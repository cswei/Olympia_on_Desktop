/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
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
 *
 */

#ifndef PageClientQt_h
#define PageClientQt_h

#include "FrameView.h"
#include "GraphicsContext.h"
#include "IntRect.h"
#include "qwebframe.h"
#include "qwebframe_p.h"
#include "qwebpage.h"
#include "qwebpage_p.h"
#include "QWebPageClient.h"
#include "TiledBackingStore.h"

#include <QtCore/qmetaobject.h>
#include <QtCore/qsharedpointer.h>
#include <QtGui/qgraphicsscene.h>
#include <QtGui/qgraphicsview.h>
#include <QtGui/qgraphicswidget.h>
#include <QtGui/qscrollbar.h>
#include <QtGui/qstyleoption.h>
#include <QtGui/qwidget.h>

#include <Settings.h>


namespace WebCore {

class PageClientQWidget : public QWebPageClient {
public:
    PageClientQWidget(QWidget* view)
        : view(view)
    {
        Q_ASSERT(view);
    }

    virtual bool isQWidgetClient() const { return true; }

    virtual void scroll(int dx, int dy, const QRect&);
    virtual void update(const QRect& dirtyRect);
    virtual void setInputMethodEnabled(bool enable);
    virtual bool inputMethodEnabled() const;
#if QT_VERSION >= 0x040600
    virtual void setInputMethodHint(Qt::InputMethodHint hint, bool enable);
#endif

#ifndef QT_NO_CURSOR
    virtual QCursor cursor() const;
    virtual void updateCursor(const QCursor& cursor);
#endif

    virtual QPalette palette() const;
    virtual int screenNumber() const;
    virtual QWidget* ownerWidget() const;
    virtual QRect geometryRelativeToOwnerWidget() const;

    virtual QObject* pluginParent() const;

    virtual QStyle* style() const;
    
    virtual bool viewResizesToContentsEnabled() const { return false; }

    QWidget* view;
};

// the overlay is here for one reason only: to have the scroll-bars and other
// extra UI elements appear on top of any QGraphicsItems created by CSS compositing layers
class QGraphicsItemOverlay : public QGraphicsItem {
    public:
    QGraphicsItemOverlay(QGraphicsWidget* view, QWebPage* p)
            :QGraphicsItem(view)
            , q(view)
            , page(p)
    {
        setPos(0, 0);
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
        setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
#endif
        setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }

    QRectF boundingRect() const
    {
        return q->boundingRect();
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* options, QWidget*)
    {
        page->mainFrame()->render(painter, static_cast<QWebFrame::RenderLayer>(QWebFrame::AllLayers&(~QWebFrame::ContentsLayer)), options->exposedRect.toRect());
    }

    void prepareGraphicsItemGeometryChange()
    {
        prepareGeometryChange();
    }

    QGraphicsWidget* q;
    QWebPage* page;
};


class PageClientQGraphicsWidget : public QWebPageClient {
public:
    PageClientQGraphicsWidget(QGraphicsWidget* v, QWebPage* p)
        : view(v)
        , page(p)
        , viewResizesToContents(false)
#if USE(ACCELERATED_COMPOSITING)
        , shouldSync(false)
#endif
    {
       Q_ASSERT(view);
#if USE(ACCELERATED_COMPOSITING)
        // the overlay and stays alive for the lifetime of
        // this QGraphicsWebView as the scrollbars are needed when there's no compositing
        view->setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
        syncMetaMethod = view->metaObject()->method(view->metaObject()->indexOfMethod("syncLayers()"));
#endif
    }

    virtual ~PageClientQGraphicsWidget();

    virtual bool isQWidgetClient() const { return false; }

    virtual void scroll(int dx, int dy, const QRect&);
    virtual void update(const QRect& dirtyRect);
    virtual void setInputMethodEnabled(bool enable);
    virtual bool inputMethodEnabled() const;
#if QT_VERSION >= 0x040600
    virtual void setInputMethodHint(Qt::InputMethodHint hint, bool enable);
#endif

#ifndef QT_NO_CURSOR
    virtual QCursor cursor() const;
    virtual void updateCursor(const QCursor& cursor);
#endif

    virtual QPalette palette() const;
    virtual int screenNumber() const;
    virtual QWidget* ownerWidget() const;
    virtual QRect geometryRelativeToOwnerWidget() const;

    virtual QObject* pluginParent() const;

    virtual QStyle* style() const;

    virtual bool viewResizesToContentsEnabled() const { return viewResizesToContents; }

    void createOrDeleteOverlay();

#if ENABLE(TILED_BACKING_STORE)
    void updateTiledBackingStoreScale();
    virtual QRectF graphicsItemVisibleRect() const;
#endif

#if USE(ACCELERATED_COMPOSITING)
    virtual void setRootGraphicsLayer(QGraphicsItem* layer);
    virtual void markForSync(bool scheduleSync);
    void updateCompositingScrollPosition();
    void syncLayers();

    // QGraphicsWebView can render composited layers
    virtual bool allowsAcceleratedCompositing() const { return true; }
#endif

    QGraphicsWidget* view;
    QWebPage* page;
    bool viewResizesToContents;

#if USE(ACCELERATED_COMPOSITING)
    QWeakPointer<QGraphicsObject> rootGraphicsLayer;

    // we have to flush quite often, so we use a meta-method instead of QTimer::singleShot for putting the event in the queue
    QMetaMethod syncMetaMethod;

    // we need to sync the layers if we get a special call from the WebCore
    // compositor telling us to do so. We'll get that call from ChromeClientQt
    bool shouldSync;
#endif
    // the overlay gets instantiated when the root layer is attached, and get deleted when it's detached
    QSharedPointer<QGraphicsItemOverlay> overlay;

    // we need to put the root graphics layer behind the overlay (which contains the scrollbar)
    enum { RootGraphicsLayerZValue, OverlayZValue };
};

}
#endif // PageClientQt
