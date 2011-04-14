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

#ifndef WebChromeClient_h
#define WebChromeClient_h

#include <WebCore/ChromeClient.h>
#include <WebCore/PlatformString.h>

namespace WebKit {

class WebPage;

class WebChromeClient : public WebCore::ChromeClient {
public:
    WebChromeClient(WebPage* page)
        : m_page(page)
    {
    }
    
private:
    virtual void chromeDestroyed();
    
    virtual void setWindowRect(const WebCore::FloatRect&);
    virtual WebCore::FloatRect windowRect();
    
    virtual WebCore::FloatRect pageRect();
    
    virtual float scaleFactor();
    
    virtual void focus();
    virtual void unfocus();
    
    virtual bool canTakeFocus(WebCore::FocusDirection);
    virtual void takeFocus(WebCore::FocusDirection);

    virtual void focusedNodeChanged(WebCore::Node*);

    // The Frame pointer provides the ChromeClient with context about which
    // Frame wants to create the new Page.  Also, the newly created window
    // should not be shown to the user until the ChromeClient of the newly
    // created Page has its show method called.
    virtual WebCore::Page* createWindow(WebCore::Frame*, const WebCore::FrameLoadRequest&, const WebCore::WindowFeatures&);
    virtual void show();
    
    virtual bool canRunModal();
    virtual void runModal();
    
    virtual void setToolbarsVisible(bool);
    virtual bool toolbarsVisible();
    
    virtual void setStatusbarVisible(bool);
    virtual bool statusbarVisible();
    
    virtual void setScrollbarsVisible(bool);
    virtual bool scrollbarsVisible();
    
    virtual void setMenubarVisible(bool);
    virtual bool menubarVisible();
    
    virtual void setResizable(bool);
    
    virtual void addMessageToConsole(WebCore::MessageSource, WebCore::MessageType, WebCore::MessageLevel, const WebCore::String& message, unsigned int lineNumber, const WebCore::String& sourceID);
    
    virtual bool canRunBeforeUnloadConfirmPanel();
    virtual bool runBeforeUnloadConfirmPanel(const WebCore::String& message, WebCore::Frame* frame);
    
    virtual void closeWindowSoon();
    
    virtual void runJavaScriptAlert(WebCore::Frame*, const WebCore::String&);
    virtual bool runJavaScriptConfirm(WebCore::Frame*, const WebCore::String&);
    virtual bool runJavaScriptPrompt(WebCore::Frame*, const WebCore::String& message, const WebCore::String& defaultValue, WebCore::String& result);
    virtual void setStatusbarText(const WebCore::String&);
    virtual bool shouldInterruptJavaScript();
    virtual bool tabsToLinks() const;
    
    virtual WebCore::IntRect windowResizerRect() const;
    
    // Methods used by HostWindow.
    virtual void invalidateWindow(const WebCore::IntRect&, bool);
    virtual void invalidateContentsAndWindow(const WebCore::IntRect&, bool);
    virtual void invalidateContentsForSlowScroll(const WebCore::IntRect&, bool);
    virtual void scroll(const WebCore::IntSize& scrollDelta, const WebCore::IntRect& rectToScroll, const WebCore::IntRect& clipRect);
    virtual WebCore::IntPoint screenToWindow(const WebCore::IntPoint&) const;
    virtual WebCore::IntRect windowToScreen(const WebCore::IntRect&) const;
    virtual PlatformPageClient platformPageClient() const;
    virtual void contentsSizeChanged(WebCore::Frame*, const WebCore::IntSize&) const;
    virtual void scrollRectIntoView(const WebCore::IntRect&, const WebCore::ScrollView*) const; // Currently only Mac has a non empty implementation.
    // End methods used by HostWindow.

    virtual void scrollbarsModeDidChange() const;
    virtual void mouseDidMoveOverElement(const WebCore::HitTestResult&, unsigned modifierFlags);
    
    virtual void setToolTip(const WebCore::String&, WebCore::TextDirection);
    
    virtual void print(WebCore::Frame*);
    
#if ENABLE(DATABASE)
    virtual void exceededDatabaseQuota(WebCore::Frame*, const WebCore::String& databaseName);
#endif

#if ENABLE(OFFLINE_WEB_APPLICATIONS)
    virtual void reachedMaxAppCacheSize(int64_t spaceNeeded);
#endif

#if ENABLE(DASHBOARD_SUPPORT)
    virtual void dashboardRegionsChanged();
#endif

    virtual void populateVisitedLinks();
    
    virtual WebCore::FloatRect customHighlightRect(WebCore::Node*, const WebCore::AtomicString& type, const WebCore::FloatRect& lineRect);
    virtual void paintCustomHighlight(WebCore::Node*, const WebCore::AtomicString& type, const WebCore::FloatRect& boxRect, const WebCore::FloatRect& lineRect,
                                      bool behindText, bool entireLine);
    
    virtual bool shouldReplaceWithGeneratedFileForUpload(const WebCore::String& path, WebCore::String& generatedFilename);
    virtual WebCore::String generateReplacementFile(const WebCore::String& path);
    
    virtual bool paintCustomScrollbar(WebCore::GraphicsContext*, const WebCore::FloatRect&, WebCore::ScrollbarControlSize, 
                                      WebCore::ScrollbarControlState, WebCore::ScrollbarPart pressedPart, bool vertical,
                                      float value, float proportion, WebCore::ScrollbarControlPartMask);
    virtual bool paintCustomScrollCorner(WebCore::GraphicsContext*, const WebCore::FloatRect&);
    
    // This is an asynchronous call. The ChromeClient can display UI asking the user for permission
    // to use Geolococation. The ChromeClient must call Geolocation::setShouldClearCache() appropriately.
    virtual void requestGeolocationPermissionForFrame(WebCore::Frame*, WebCore::Geolocation*);
    virtual void cancelGeolocationPermissionRequestForFrame(WebCore::Frame*, WebCore::Geolocation*);
    
    virtual void runOpenPanel(WebCore::Frame*, PassRefPtr<WebCore::FileChooser>);
    virtual void chooseIconForFiles(const Vector<WebCore::String>&, WebCore::FileChooser*);

    virtual bool setCursor(WebCore::PlatformCursorHandle);
    
    // Notification that the given form element has changed. This function
    // will be called frequently, so handling should be very fast.
    virtual void formStateDidChange(const WebCore::Node*);
    
    virtual void formDidFocus(const WebCore::Node*);
    virtual void formDidBlur(const WebCore::Node*);
    
    virtual PassOwnPtr<WebCore::HTMLParserQuirks> createHTMLParserQuirks();

#if USE(ACCELERATED_COMPOSITING)
    virtual void attachRootGraphicsLayer(WebCore::Frame*, WebCore::GraphicsLayer*);
    virtual void setNeedsOneShotDrawingSynchronization();
    virtual void scheduleCompositingLayerSync();
#endif
    
    WebCore::String m_cachedToolTip;
    WebPage* m_page;
};

} // namespace WebKit

#endif // WebChromeClient_h
