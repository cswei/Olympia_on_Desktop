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

#include "TabbedView.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>

namespace Olympia {
namespace Browser {

using namespace Olympia::WebKit;

TabbedView::TabbedView(QSize viewSize, QWidget* parent, Qt::WindowFlags f)
    : WebView(viewSize, parent, f)
    , m_menuInitialized(false)
{
}

TabbedView::~TabbedView()
{
}

QVector<QAction*> TabbedView::buildActions()
{
    if (!m_menuInitialized) {
        m_menuInitialized = true;
        m_zoomIn  = new QAction("Zoom In", this);
        m_zoomOut = new QAction("Zoom Out", this);
        m_back    = new QAction("Back", this);
        m_forward = new QAction("Forward", this);
        m_find    = new QAction("Find", this);
        m_stop    = new QAction("Stop", this);
        m_refresh = new QAction("Refresh", this);
        m_select  = new QAction("Select", this);
        m_unselect= new QAction("Unselect", this);
        m_copy    = new QAction("Copy", this);
        m_paste   = new QAction("Paste", this);
        m_multiTouch = new QAction("MultiTouch", this);

        connect(m_zoomIn,  SIGNAL(triggered()), this, SLOT(zoomIn()));
        connect(m_zoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()));
        connect(m_back,    SIGNAL(triggered()), this, SLOT(goBack()));
        connect(m_forward, SIGNAL(triggered()), this, SLOT(goForward()));
        connect(m_find,    SIGNAL(triggered()), this, SLOT(find()));
        connect(m_stop,    SIGNAL(triggered()), this, SLOT(stop()));
        connect(m_refresh, SIGNAL(triggered()), this, SLOT(reload()));
        connect(m_copy,    SIGNAL(triggered()), this, SLOT(copy()));
        connect(m_paste,   SIGNAL(triggered()), this, SLOT(paste()));
        connect(m_select,  SIGNAL(triggered()), this, SLOT(setSelect()));
        connect(m_unselect,SIGNAL(triggered()), this, SLOT(unsetSelect()));
        connect(m_multiTouch,SIGNAL(triggered()), this, SLOT(setMultiTouchMode()));
    }

    m_copy->setEnabled(!selectedText().isEmpty());
    m_select->setEnabled(navigationMode() != WebView::Selection);
    m_unselect->setEnabled(navigationMode() == WebView::Selection);
    m_stop->setEnabled(isLoading());
    m_paste->setEnabled(isContentEditable() && (!QApplication::clipboard()->text().isEmpty()));
    m_back->setEnabled(currentHistoryIndex() > 0);
    m_forward->setEnabled(currentHistoryIndex() < historyLength() - 1 );

    QVector<QAction*> actions;
    actions.push_back(m_zoomIn);
    actions.push_back(m_zoomOut);
    actions.push_back(m_back);
    actions.push_back(m_forward);
    actions.push_back(m_find);
    actions.push_back(m_stop);
    actions.push_back(m_refresh);
    actions.push_back(m_copy);
    actions.push_back(m_paste);
    actions.push_back(m_select);
    actions.push_back(m_unselect);
    actions.push_back(m_multiTouch);

    return actions;
}

} // namespace WebKit
} // namespace Olympia

