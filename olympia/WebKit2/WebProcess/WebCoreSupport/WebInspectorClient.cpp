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

#include "WebInspectorClient.h"

#include "NotImplemented.h"

using namespace WebCore;

namespace WebKit {

void WebInspectorClient::inspectorDestroyed()
{
    delete this;
}

void WebInspectorClient::openInspectorFrontend(InspectorController*)
{
    notImplemented();
}

Page* WebInspectorClient::createPage()
{
    notImplemented();
    return 0;
}

String WebInspectorClient::localizedStringsURL()
{
    notImplemented();
    return String();
}

String WebInspectorClient::hiddenPanels()
{
    notImplemented();
    return String();
}

void WebInspectorClient::showWindow()
{
    notImplemented();
}

void WebInspectorClient::closeWindow()
{
    notImplemented();
}

void WebInspectorClient::attachWindow()
{
    notImplemented();
}

void WebInspectorClient::detachWindow()
{
    notImplemented();
}

void WebInspectorClient::setAttachedWindowHeight(unsigned height)
{
    notImplemented();
}

void WebInspectorClient::highlight(Node*)
{
    notImplemented();
}

void WebInspectorClient::hideHighlight()
{
    notImplemented();
}

void WebInspectorClient::inspectedURLChanged(const String& newURL)
{
    notImplemented();
}

void WebInspectorClient::populateSetting(const String& key, String*)
{
    notImplemented();
}

void WebInspectorClient::storeSetting(const String&, const String&)
{
    notImplemented();
}

void WebInspectorClient::inspectorWindowObjectCleared()
{
    notImplemented();
}

} // namespace WebKit
