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

#ifndef TabbedView_h
#define TabbedView_h

#include "WebViewQt.h"

namespace Olympia {
namespace Browser {

class TabbedView : public WebView 
{
    Q_OBJECT

public:
    TabbedView(QSize viewSize, QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~TabbedView();
    
    QVector<QAction*> buildActions();

//private:
    // From WebView
    //virtual Olympia::WebKit::WebPage* createWindow(int x, int y, int width, int height, unsigned flags);
    // Make it uncopyable
    TabbedView(const TabbedView&);
    TabbedView& operator = (const TabbedView&);

private:
    QAction* m_zoomIn;
    QAction* m_zoomOut;
    QAction* m_back;
    QAction* m_forward;
    QAction* m_find;
    QAction* m_stop;
    QAction* m_refresh;
    QAction* m_copy;
    QAction* m_paste;
    QAction* m_select;
    QAction* m_unselect;
    QAction* m_multiTouch;
    bool m_menuInitialized;
};

} // namespace WebKit 
} // namespace Olympia 

#endif // TabbedView_h
