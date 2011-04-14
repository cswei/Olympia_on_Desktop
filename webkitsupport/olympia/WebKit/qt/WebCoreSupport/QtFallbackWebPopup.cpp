/*
 * Copyright (C) 2010 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
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
#include "config.h"
#include "QtFallbackWebPopup.h"

#ifndef QT_NO_COMBOBOX

#include "HostWindow.h"
#include "PopupMenuClient.h"
#include "QWebPageClient.h"
#include "qgraphicswebview.h"
#include <QAbstractItemView>
#include <QApplication>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QInputContext>
#include <QMouseEvent>
#include <QStandardItemModel>

#if ENABLE(SYMBIAN_DIALOG_PROVIDERS)
#include <BrCtlDialogsProvider.h>
#include <BrowserDialogsProvider.h> // S60 platform private header file
#include <e32base.h>
#endif

namespace WebCore {

QtFallbackWebPopupCombo::QtFallbackWebPopupCombo(QtFallbackWebPopup& ownerPopup)
    : m_ownerPopup(ownerPopup)
{
}

void QtFallbackWebPopupCombo::showPopup()
{
    QComboBox::showPopup();
    m_ownerPopup.m_popupVisible = true;
}

void QtFallbackWebPopupCombo::hidePopup()
{
#ifndef QT_NO_IM
    QWidget* activeFocus = QApplication::focusWidget();
    if (activeFocus && activeFocus == QComboBox::view()
        && activeFocus->testAttribute(Qt::WA_InputMethodEnabled)) {
        QInputContext* qic = activeFocus->inputContext();
        if (qic) {
            qic->reset();
            qic->setFocusWidget(0);
        }
    }
#endif // QT_NO_IM

    QComboBox::hidePopup();

    if (QGraphicsProxyWidget* proxy = graphicsProxyWidget())
        proxy->setVisible(false);

    if (!m_ownerPopup.m_popupVisible)
        return;

    m_ownerPopup.m_popupVisible = false;
    m_ownerPopup.popupDidHide();
}

// QtFallbackWebPopup

QtFallbackWebPopup::QtFallbackWebPopup()
    : QtAbstractWebPopup()
    , m_popupVisible(false)
    , m_combo(new QtFallbackWebPopupCombo(*this))
    , m_proxy(0)
{
    connect(m_combo, SIGNAL(activated(int)),
            SLOT(activeChanged(int)), Qt::QueuedConnection);
}

QtFallbackWebPopup::~QtFallbackWebPopup()
{
    // If we create a proxy, then the deletion of the proxy and the
    // combo will be done by the proxy's parent (QGraphicsWebView)
    if (!m_proxy)
        delete m_combo;
}

void QtFallbackWebPopup::show()
{
    if (!pageClient())
        return;

#if ENABLE(SYMBIAN_DIALOG_PROVIDERS)
    TRAP_IGNORE(showS60BrowserDialog());
#else
    populate();
    m_combo->setCurrentIndex(currentIndex());

    QRect rect = geometry();
    if (QGraphicsWebView *webView = qobject_cast<QGraphicsWebView*>(pageClient()->pluginParent())) {
        if (!m_proxy) {
            m_proxy = new QGraphicsProxyWidget(webView);
            m_proxy->setWidget(m_combo);
        } else
            m_proxy->setVisible(true);
        m_proxy->setGeometry(rect);
    } else {
        m_combo->setParent(pageClient()->ownerWidget());
        m_combo->setGeometry(QRect(rect.left(), rect.top(),
                               rect.width(), m_combo->sizeHint().height()));

    }

    QMouseEvent event(QEvent::MouseButtonPress, QCursor::pos(), Qt::LeftButton,
                      Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(m_combo, &event);
#endif
}

#if ENABLE(SYMBIAN_DIALOG_PROVIDERS)

static void ResetAndDestroy(TAny* aPtr)
{
    RPointerArray<HBufC>* items = reinterpret_cast<RPointerArray<HBufC>* >(aPtr);
    items->ResetAndDestroy();
}

void QtFallbackWebPopup::showS60BrowserDialog()
{
    static MBrCtlDialogsProvider* dialogs = CBrowserDialogsProvider::NewL(0);
    if (!dialogs)
        return;

    int size = itemCount();
    CArrayFix<TBrCtlSelectOptionData>* options = new CArrayFixFlat<TBrCtlSelectOptionData>(qMax(1, size));
    RPointerArray<HBufC> items(qMax(1, size));
    CleanupStack::PushL(TCleanupItem(&ResetAndDestroy, &items));

    for (int i = 0; i < size; i++) {
        if (itemType(i) == Separator) {
            TBrCtlSelectOptionData data(_L("----------"), false, false, false);
            options->AppendL(data);
        } else {
            HBufC16* itemStr = HBufC16::NewL(itemText(i).length());
            itemStr->Des().Copy((const TUint16*)itemText(i).utf16(), itemText(i).length());
            CleanupStack::PushL(itemStr);
            TBrCtlSelectOptionData data(*itemStr, i == currentIndex(), false, itemIsEnabled(i));
            options->AppendL(data);
            items.AppendL(itemStr);
            CleanupStack::Pop();
        }
    }

    dialogs->DialogSelectOptionL(KNullDesC(), (TBrCtlSelectOptionType)(ESelectTypeSingle | ESelectTypeWithFindPane), *options);

    CleanupStack::PopAndDestroy(&items);

    int newIndex;
    for (newIndex = 0; newIndex < options->Count() && !options->At(newIndex).IsSelected(); newIndex++) {}
    if (newIndex == options->Count())
        newIndex = currentIndex();
    
    m_popupVisible = false;
    popupDidHide();

    if (currentIndex() != newIndex && newIndex >= 0)
        valueChanged(newIndex);

    delete options;
}
#endif

void QtFallbackWebPopup::hide()
{
    m_combo->hidePopup();
}

void QtFallbackWebPopup::populate()
{
    m_combo->clear();

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(m_combo->model());
    Q_ASSERT(model);

#if !defined(Q_WS_S60)
    m_combo->setFont(font());
#endif
    for (int i = 0; i < itemCount(); ++i) {
        switch (itemType(i)) {
        case Separator:
            m_combo->insertSeparator(i);
            break;
        case Group:
            m_combo->insertItem(i, itemText(i));
            model->item(i)->setEnabled(false);
            break;
        case Option:
            m_combo->insertItem(i, itemText(i));
            model->item(i)->setEnabled(itemIsEnabled(i));
            break;
        }
    }
}

void QtFallbackWebPopup::activeChanged(int index)
{
    if (index < 0)
        return;

    valueChanged(index);
}

}

#endif // QT_NO_COMBOBOX
