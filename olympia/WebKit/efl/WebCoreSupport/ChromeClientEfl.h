/*
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (C) 2008 INdT - Instituto Nokia de Tecnologia
 * Copyright (C) 2009-2010 ProFUSION embedded systems
 * Copyright (C) 2009-2010 Samsung Electronics
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
 */

#ifndef ChromeClientEfl_h
#define ChromeClientEfl_h

#include "ChromeClient.h"
#include "KURL.h"
#include <Evas.h>

namespace WebCore {

class ChromeClientEfl : public ChromeClient {
public:
    explicit ChromeClientEfl(Evas_Object* view);
    virtual ~ChromeClientEfl();

    virtual void chromeDestroyed();

    virtual void setWindowRect(const FloatRect&);
    virtual FloatRect windowRect();

    virtual FloatRect pageRect();

    virtual float scaleFactor();

    virtual void focus();
    virtual void unfocus();

    virtual bool canTakeFocus(FocusDirection);
    virtual void takeFocus(FocusDirection);

    virtual void focusedNodeChanged(Node*);

    virtual Page* createWindow(Frame*, const FrameLoadRequest&, const WindowFeatures&);
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

    virtual void addMessageToConsole(MessageSource, MessageType, MessageLevel, const String& message,
                                     unsigned int lineNumber, const String& sourceID);

    virtual bool canRunBeforeUnloadConfirmPanel();
    virtual bool runBeforeUnloadConfirmPanel(const String& message, Frame* frame);

    virtual void closeWindowSoon();

    virtual void runJavaScriptAlert(Frame*, const String&);
    virtual bool runJavaScriptConfirm(Frame*, const String&);
    virtual bool runJavaScriptPrompt(Frame*, const String& message, const String& defaultValue, String& result);
    virtual void setStatusbarText(const String&);
    virtual bool shouldInterruptJavaScript();
    virtual bool tabsToLinks() const;

    virtual IntRect windowResizerRect() const;

    virtual void contentsSizeChanged(Frame*, const IntSize&) const;
    virtual IntPoint screenToWindow(const IntPoint&) const;
    virtual IntRect windowToScreen(const IntRect&) const;
    virtual PlatformPageClient platformPageClient() const;

    virtual void scrollbarsModeDidChange() const;
    virtual void mouseDidMoveOverElement(const HitTestResult&, unsigned modifierFlags);

    virtual void setToolTip(const String&, TextDirection);

    virtual void print(Frame*);

#if ENABLE(DATABASE)
    virtual void exceededDatabaseQuota(Frame*, const String&);
#endif

#if ENABLE(OFFLINE_WEB_APPLICATIONS)
    virtual void reachedMaxAppCacheSize(int64_t spaceNeeded);
#endif

    virtual void runOpenPanel(Frame*, PassRefPtr<FileChooser>);
    virtual void chooseIconForFiles(const Vector<String>&, FileChooser*);
    virtual void formStateDidChange(const Node*);

    virtual PassOwnPtr<HTMLParserQuirks> createHTMLParserQuirks() { return 0; }

    virtual bool setCursor(PlatformCursorHandle);

    virtual void scrollRectIntoView(const IntRect&, const ScrollView*) const {}

    virtual void requestGeolocationPermissionForFrame(Frame*, Geolocation*);
    virtual void cancelGeolocationPermissionRequestForFrame(Frame*, Geolocation*);
    virtual void cancelGeolocationPermissionForFrame(Frame*, Geolocation*);

    virtual void invalidateContents(const IntRect&, bool);
    virtual void invalidateWindow(const IntRect&, bool);
    virtual void invalidateContentsAndWindow(const IntRect&, bool);
    virtual void invalidateContentsForSlowScroll(const IntRect&, bool);
    virtual void scroll(const IntSize&, const IntRect&, const IntRect&);
    virtual void cancelGeolocationPermissionRequestForFrame(Frame*);
    virtual void iconForFiles(const Vector<String, 0u>&, PassRefPtr<FileChooser>);

    Evas_Object* m_view;
    KURL m_hoveredLinkURL;
};
}

#endif // ChromeClientEfl_h
