/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "WebUIClient.h"

#include "WKAPICast.h"

using namespace WebCore;

namespace WebKit {

WebUIClient::WebUIClient()
{
    initialize(0);
}

void WebUIClient::initialize(WKPageUIClient* client)
{
    if (client && !client->version)
        m_pageUIClient = *client;
    else 
        memset(&m_pageUIClient, 0, sizeof(m_pageUIClient));
}

WebPageProxy* WebUIClient::createNewPage(WebPageProxy* page)
{
    if (!m_pageUIClient.createNewPage)
        return 0;
    
    return toWK(m_pageUIClient.createNewPage(toRef(page), m_pageUIClient.clientInfo));
} 

void WebUIClient::showPage(WebPageProxy* page)
{
    if (!m_pageUIClient.showPage)
        return;
    
    m_pageUIClient.showPage(toRef(page), m_pageUIClient.clientInfo);
}

void WebUIClient::close(WebPageProxy* page)
{
    if (!m_pageUIClient.close)
        return;
    
    m_pageUIClient.close(toRef(page), m_pageUIClient.clientInfo);
}

void WebUIClient::runJavaScriptAlert(WebPageProxy* page, StringImpl* alertText, WebFrameProxy* frame)
{
    if (!m_pageUIClient.runJavaScriptAlert)
        return;
    
    m_pageUIClient.runJavaScriptAlert(toRef(page), toRef(alertText), toRef(frame), m_pageUIClient.clientInfo);
}


} // namespace WebKit
