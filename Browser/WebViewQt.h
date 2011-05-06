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

#ifndef WebPageClientQt_h
#define WebPageClientQt_h

#include <deque>

#include <QIcon>
#include <QtGui/QWidget>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QStatusBar>
#include <QtGui/QProgressBar>
#include <QTime>
#include <QTimer>
#include <stdint.h>
#include "WebPageClient.h"
#include <QMap>
#include <QUrl>

class QImage;
class QNetworkReply;
class QAuthenticator;

namespace Olympia {
namespace Browser {

class EventHandler;

class WebView : public QWidget, public Olympia::WebKit::WebPageClient
{
    Q_OBJECT

signals:
    void loadStart();
    void loadProgress(int percentage);
    void loadFinished();
    void loadFailed();
    void urlChanged(const QString&);
    void sigSetTitle(const QString& url, const QString& title);
    void sigSetIcon(const QString& url, const QString& iconUrl, const QIcon& icon);
    void sigNoFaviconLoaded();
    void requestCreateTab(WebView**, int, int, int, int, unsigned);

public slots:
    void stop();
    void reload();
    void goBack();
    void goForward();
    void zoomOut();
    void zoomIn();
    void setSelect();
    void unsetSelect();
    void copy();
    void paste();
    void clearEditContent();
    void dispatchTouchEvent(Olympia::Platform::TouchEvent te);
    void slotOrientationChanged();
    void slotWindowSizeChanged();
    void setMultiTouchMode();
    void unsetMultiTouchMode();

private slots:
    void slotAuthenticationRequired(QNetworkReply*, QAuthenticator*);

public:
    enum NavigationMode {
        Normal = 0,
        Selection = 1,
        MultiTouch = 2
    };

    WebView(QSize viewSize, QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~WebView();
    void load(const char* url, const char* networkToken, bool isInitial = false);
    bool isContentEditable() { return m_editable; }
    int getEditLength() { return m_editLength; }
    QString selectedText();
    void scrollBy(int deltaX, int deltaY);
    QString title() const { return m_title; };
    QString url() const { return m_url; };
    QPixmap thumbnail(const QSize& size) const;
    QIcon urlIcon() const { return m_urlIcon; }
    void setWebPageVisible(bool);
    void triggerRender();
    bool hasIdleJobs();
    int loadProgress();
    bool isLoading();
    int historyLength();
    int currentHistoryIndex();
    NavigationMode navigationMode() const { return m_naviMode; }
    void setNavigationMode(NavigationMode m);

protected:
    // WebPageClient interfaces
    virtual int getInstanceId() const;
    virtual void notifyLoadStarted();
    virtual void notifyLoadCommitted(const unsigned short* originalUrl, unsigned int originalUrlLength, const unsigned short* finalUrl, unsigned int finalUrlLength, const unsigned short* networkToken, unsigned int networkTokenLength);
    virtual void notifyLoadFailedBeforeCommit(const unsigned short* originalUrl, unsigned int originalUrlLength, const unsigned short* finalUrl, unsigned int finalUrlLength, const unsigned short* networkToken, unsigned int networkTokenLength);
    virtual void notifyLoadToAnchor(const unsigned short* url, unsigned int urlLength, const unsigned short* networkToken, unsigned int networkTokenLength);
    virtual void notifyLoadProgress(int percentage);
    virtual void notifyLoadReadyToRender();
    virtual void notifyLoadFinished(int status);
    virtual void notifyClientRedirect(const unsigned short* originalUrl, unsigned int originalUrlLength, const unsigned short* finalUrl, unsigned int finalUrlLength);

    virtual void notifyDocumentCreatedForFrame(const WebFrame frame, const bool isMainFrame, const WebDOMDocument& document, const JSContextRef context, const JSValueRef window);
    virtual void notifyFrameDetached(const WebFrame frame);

    virtual void notifyRunLayoutTestsFinished();

    virtual void addMessageToConsole(const unsigned short* message, unsigned messageLength, const unsigned short* source, unsigned sourceLength, unsigned lineNumber);
    virtual void runJavaScriptAlert(const unsigned short* message, unsigned messageLength);
    virtual bool runJavaScriptConfirm(const unsigned short* message, unsigned messageLength);
    virtual bool runJavaScriptPrompt(const unsigned short* message, unsigned messageLength, const unsigned short* defaultValue, unsigned defaultValueLength, Olympia::WebKit::String& result);
    virtual bool shouldInterruptJavaScript();

    virtual void javascriptSourceParsed(const unsigned short* url, unsigned urlLength, const unsigned short* script, unsigned scriptLength);
    virtual void javascriptParsingFailed(const unsigned short* url, unsigned urlLength, const unsigned short* error, unsigned errorLength, int lineNumber);
    virtual void javascriptPaused(const unsigned short* stack, unsigned stackLength);
    virtual void javascriptContinued();

    // All of these methods use transformed coordinates
    virtual void contentsSizeChanged(const int width, const int height) const;
    virtual void scrollChanged(const int scrollX, const int scrollY) const;
    virtual void zoomChanged(const bool isMinZoomed, const bool isMaxZoomed, const bool isAtInitialZoom, const double newZoom) const;

    virtual void setPageTitle(const unsigned short* title, unsigned titleLength);
    virtual void blitToCanvas(const int dstX,
        const int dstY,
        const int dstWidth,
        const int dstHeight,
        const int srcX,
        const int srcY,
        const int srcWidth,
        const int srcHeight);

    virtual void blitToCanvas(const int dstX,
        const int dstY,
        const unsigned char* srcImage,
        const int srcStride,
        const int srcX,
        const int srcY,
        const int srcWidth,
        const int srcHeight);

    virtual void blitFromBufferToBuffer(unsigned char* dst,
        const int dstStride,
        const int dstX,
        const int dstY,
        const unsigned char* src,
        const int srcStride,
        const int srcX,
        const int srcY,
        const int srcWidth,
        const int srcHeight);

    virtual void blendOntoCanvas(const int dstX,
        const int dstY,
        const unsigned char* srcImage,
        const int srcStride,
        const unsigned char* srcAlphaImage,
        const int srcAlphaStride,
        const char globalAlpha,
        const int srcX,
        const int srcY,
        const int srcWidth,
        const int srcHeight);

    virtual void invalidateWindow(const int dstX,
        const int dstY,
        const int dstWidth,
        const int dstHeight);

    virtual void lockCanvas();
    virtual void unlockCanvas();
    virtual void clearCanvas();

    virtual void inputFocusGained(unsigned int frameId, unsigned int inputFieldId, Olympia::Platform::OlympiaInputType type, unsigned int characterCount, unsigned int selectionStart, unsigned int selectionEnd);
    virtual void inputFocusLost(unsigned int frameId, unsigned int inputFieldId);
    virtual void inputTextChanged(unsigned int frameId, unsigned int inputFieldId, const unsigned short* text, unsigned int textLength, unsigned int selectionStart, unsigned int selectionEnd);
    virtual void inputTextForElement(unsigned int requestedFrameId, unsigned int requestedElementId, unsigned int offset, int length, int selectionStart, int selectionEnd, const unsigned short* text, unsigned int textLength);
    virtual void inputFrameCleared(unsigned int frameId);
    virtual void inputSelectionChanged(unsigned int frameId, unsigned int inputFieldId, unsigned int selectionStart, unsigned int selectionEnd);
    virtual void inputSetNavigationMode(bool);

    virtual void selectionBounds(Olympia::Platform::IntRect start, Olympia::Platform::IntRect end);

    virtual void cursorChanged(Olympia::Platform::CursorType cursorType, const char* url, const int x, const int y);

    virtual void requestGeolocationPermission(Olympia::Platform::GeoTracker*);
    virtual void cancelGeolocationPermission(Olympia::Platform::GeoTracker*);
    virtual Olympia::Platform::NetworkStreamFactory* networkStreamFactory();
    virtual Olympia::Platform::HttpStreamDebugger* httpStreamDebugger();
    virtual bool runMessageLoopForJavaScript();

    virtual void handleStringPattern(const unsigned short* pattern, unsigned int length);
    virtual void handleExternalLink(const Platform::NetworkRequest&, const unsigned short* context, unsigned int contextLength);

    virtual void resetBackForwardList(unsigned int listSize, unsigned int currentIndex);

    virtual void openPopupList(bool multiple, const int size, const ScopeArray<Olympia::WebKit::String>& labels, bool* enableds, const int* itemType, bool* selecteds);
    virtual void popupListClosed(const int size, bool* selecteds);
    virtual void popupListClosed(const int index);
    virtual void openDateTimePopup(const int type, const Olympia::WebKit::String& value, const Olympia::WebKit::String& min, const Olympia::WebKit::String& max, const double step);
    virtual void setDateTimeInput(const Olympia::WebKit::String& value);

    virtual void contextChanged(const Olympia::WebKit::Context&);

    virtual bool chooseFilenames(bool allowMultiple, const Olympia::WebKit::String& acceptTypes, const SharedArray<Olympia::WebKit::String>& initialFiles, unsigned int initialFileSize, SharedArray<Olympia::WebKit::String>& chosenFiles, unsigned int& chosenFileSize);

    virtual void loadPluginForMimetype(int, int width, int height, const char* url, bool isHtml5Video, bool hasRenderer);
    virtual void notifyPluginRectChanged(int, Platform::IntRect rectChanged);
    virtual void destroyPlugin(int);
    virtual void playMedia(int);
    virtual void pauseMedia(int);

    virtual Olympia::WebKit::WebPage* createWindow(int x, int y, int width, int height, unsigned flags);
    virtual void scheduleCloseWindow();

    // Database interface.
    virtual unsigned long long databaseQuota(const unsigned short* origin, unsigned originLength,
                                             const unsigned short* databaseName, unsigned databaseNameLength,
                                             unsigned long long totalUsage, unsigned long long originUsage,
                                             unsigned long long estimatedSize);

    virtual void setIconForUrl(const char* originalPageUrl, const char* finalPageUrl, const char* iconUrl);

    virtual Olympia::WebKit::String getErrorPage(int errorCode, const char* errorMessage, const char* url);

    virtual void willDeferLoading();
    virtual void didResumeLoading();

    // headers is a list of alternating key and value
    virtual void setMetaHeaders(const ScopeArray<Olympia::WebKit::String>& headers, unsigned int headersSize);
    // End of WebPageClient interfaces

protected:
    // QWidget inferfaces:
    virtual void paintEvent(QPaintEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void inputMethodEvent(QInputMethodEvent*);

    Olympia::WebKit::WebPage* webPage() const { return  m_page; }

private:
    QPoint validateScrollPoint(const QPoint& point);
    bool hasTwoScrollbars() const;
    void createWebPageIfNeeded();
    void changeEventHandler();

private  slots:
    void animateScroll();
    void slotIconDataFinished(QNetworkReply*);

private:
    Olympia::WebKit::WebPage* m_page;
    bool m_needsRendering;
    bool m_blendedFirstScrollbar;
    QImage* m_screenImage;
    mutable bool m_isThumbnailNeedUpdate;
    mutable QPixmap m_thumbnailCache;
    QRect m_pageRect;
    QString m_inputText;
    bool m_editable;
    bool m_clearEditContent;
    int m_editLength;
    bool m_pasteText;
    QString m_title;
    QString m_url;
    QMap<QString, QString> m_metaHeaders;
    QIcon m_urlIcon;
    QUrl m_iconUrl;
    NavigationMode m_naviMode;

    mutable QSize m_contentSize;

    // For inertial scolling:
    QPoint m_lastContentPos;
    QPoint m_nextScrollStepLength;
    QTimer m_scrollTimer;

    int m_loadProgress;  // -1: not loading
                         // 0 ~ 100, loading
    int m_historyLength;
    int m_currentHistoryIndex;

    // event handler
    EventHandler *m_eventHandler;

    friend class EventHandler;
    friend class NormalEventHandler;
    friend class SelectionEventHandler;
    friend class MultiTouchEventHandler;
};

} // namespace WebKit
} // namespace Olympia

#endif // WebPageClientQt_h
