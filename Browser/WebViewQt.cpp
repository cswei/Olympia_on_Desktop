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

#include "config.h"
#include "WebViewQt.h"

#include "BackingStore.h"
#include "IntRect.h"

#include "Constant.h"
#include "MainWindowBase.h"
#include "OlympiaPlatformKeyboardCodes.h"
#include "OlympiaPlatformReplaceText.h"
#include "OlympiaStreamFactory.h"
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QImage>
#include <QNetworkReply>
#include <QtCore/QTimer>
#include <QtGui/QImage>
#include <QtGui/QInputDialog>
#include <QtGui/QListWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QVBoxLayout>
#include <QtGui/QFileDialog>
#include <QtGui/QDesktopServices>
#include <QUrl>
#include <stdio.h>
#ifdef DEBUG_PAINTING
#include <sys/time.h>
#endif
#include <VG/openvg.h>
#include "ScopePointer.h"
#include "NetworkQt.h"
#include "WebPage.h"
#include "WebViewEventHandler.h"

#if COMPILER(MSVC)
#define DEBUG_PRINT(fmt, args,...) fprintf(stderr, fmt, ##args)
#else
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, fmt, ##args)
#endif


// 16 bits, 2 bytes, so shift == 1
#define BACKINGSTORE_IMAGE_PIXEL_SIZE_SHIFT 1

namespace Olympia {
namespace Browser {

#ifdef DEBUG_PAINTING
inline static double currentTimeMS()
{
    struct timeval now;
    gettimeofday(&now, 0);
    return now.tv_sec * 1000.0 + now.tv_usec / 1000.0;
}
#endif

using namespace Olympia::WebKit;
using namespace Olympia::Platform;

WebView::WebView(QSize viewSize, QWidget *w, Qt::WindowFlags f)
      : QWidget(w, f)
      , m_page(0)
      , m_needsRendering(false)
      , m_blendedFirstScrollbar(false)
      , m_screenImage(0)
      , m_isThumbnailNeedUpdate(true)
      , m_pageRect(QPoint(0, 0), viewSize)
      , m_contentSize(0, 0)
      , m_inputText("")
      , m_editable(false)
      , m_editLength(0)
      , m_clearEditContent(false)
      , m_pasteText(false)
      , m_naviMode(Normal)
      , m_loadProgress(-1)
      , m_historyLength(0)
      , m_currentHistoryIndex(0)
      , m_eventHandler(new NormalEventHandler(this))
{
    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(255,255,255));
    setPalette(palette);

    m_screenImage = new QImage(m_pageRect.width(), m_pageRect.height(), QImage::Format_RGB32);
    m_screenImage->fill(QColor(255, 255, 255).rgb());

    // setting focus to it when creating the view
    setFocus();
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled);

    connect(&m_scrollTimer, SIGNAL(timeout()), this, SLOT(animateScroll()));
}

WebView::~WebView()
{
    delete m_page;
    delete m_screenImage;
    delete m_eventHandler;
}

void WebView::createWebPageIfNeeded()
{
    if (!m_page) { // defer the create surface to first load.
        Olympia::Platform::IntRect r(rect().x(), rect().y(), rect().width(), rect().height());
        m_page = new WebPage(this, r);
        m_page->backingStore()->createSurface();
        m_page->setDefaultLayoutSize(m_pageRect.width(), m_pageRect.height());
        m_page->setPlatformScreenSize(rect().width(), rect().height());
        m_page->setVisible(true);
    }
}

void WebView::load(const char* url, const char* networkToken, bool isInitial)
{
    emit sigSetTitle(QString(url), QString(url));
    createWebPageIfNeeded();
    m_page->load(url, networkToken, isInitial);
}

void WebView::stop()
{
    if (!m_page)
        return;
    m_page->stopLoading();
}

void WebView::reload()
{
    if (!m_page)
        return;
    m_page->reload();
}

void WebView::goBack()
{
    if (!m_page)
        return;
    m_page->goBackOrForward(-1);
}

void WebView::goForward()
{
    if (!m_page)
        return;
    m_page->goBackOrForward(1);
}

void WebView::paste()
{
    m_pasteText = true;
}

void WebView::clearEditContent()
{
    m_clearEditContent = true;
}

void WebView::zoomOut()
{
    if (!m_page)
        return;
    Platform::IntPoint p = m_page->scrollPosition();
    QRect r = contentsRect();
    m_page->bitmapZoom(p.x()+r.width()/2, p.y()+r.height()/2, 0.8, true);
}

void WebView::zoomIn()
{
    if (!m_page)
        return;
    Platform::IntPoint p = m_page->scrollPosition();
    QRect r = contentsRect();
    m_page->bitmapZoom(p.x()+r.width()/2, p.y()+r.height()/2, 1.2, true);
}

int WebView::getInstanceId() const
{
    return 0;
}

void WebView::notifyLoadStarted()
{
    m_loadProgress = 0;
    emit loadStart();
    emit loadProgress(m_loadProgress);
}

void WebView::notifyLoadCommitted(const unsigned short* originalUrl, unsigned int originalUrlLength, const unsigned short* finalUrl, unsigned int finalUrlLength, const unsigned short* networkToken, unsigned int networkTokenLength)
{
    // qWarning() << "notifyLoadFailedBeforeCommit() originalUrl: " << QString((const QChar*)originalUrl, originalUrlLength) << " finalUrl: " << QString((const QChar*)finalUrl, finalUrlLength) << " networkToken:" << QString((const QChar*)networkToken, networkTokenLength);
    m_url = QString((const QChar*)finalUrl, finalUrlLength);
    emit urlChanged(m_url);
    if(m_title.isEmpty() && !m_url.isEmpty()) {
        emit sigSetTitle(m_url, m_url);
    }
}

void WebView::notifyLoadFailedBeforeCommit(const unsigned short* originalUrl, unsigned int originalUrlLength, const unsigned short* finalUrl, unsigned int finalUrlLength, const unsigned short* networkToken, unsigned int networkTokenLength)
{
    // qWarning() << "notifyLoadFailedBeforeCommit() originalUrl: " << QString((const QChar*)originalUrl, originalUrlLength) << " finalUrl: " << QString((const QChar*)finalUrl, finalUrlLength) << " networkToken:" << QString((const QChar*)networkToken, networkTokenLength);
    m_url = QString((const QChar*)finalUrl, finalUrlLength);
    emit urlChanged(m_url);
}

void WebView::notifyLoadToAnchor(const unsigned short* url, unsigned int urlLength, const unsigned short* networkToken, unsigned int networkTokenLength)
{
    // qWarning() << "notifyLoadToAnchor: URL:" << QString((const QChar*)url, urlLength) << " networkToken:" << QString((const QChar*)networkToken, networkTokenLength);
    m_url = QString((const QChar*)url, urlLength);
    emit urlChanged(m_url);
}

void WebView::notifyLoadProgress(int percentage)
{
    m_loadProgress = percentage;
    emit loadProgress(percentage);
}

void WebView::notifyLoadReadyToRender()
{
    triggerRender();
}

void WebView::notifyLoadFinished(int status)
{
    m_loadProgress = -1;
    if (!status) {
        emit loadFinished();
    }
    else {
        emit loadFailed();
    }
}

int WebView::loadProgress()
{
    return m_loadProgress;
}

bool WebView::isLoading()
{
    return m_loadProgress != -1;
}

void WebView::notifyClientRedirect(const unsigned short* originalUrl, unsigned int originalUrlLength, const unsigned short* finalUrl, unsigned int finalUrlLength)
{
    // qWarning() << "notifyClientRedirect() originalUrl: " << QString((const QChar*)originalUrl, originalUrlLength) << " finalUrl: " << QString((const QChar*)finalUrl, finalUrlLength);
    m_url = QString((const QChar*)finalUrl, finalUrlLength);
    emit urlChanged(m_url);
}

void WebView::notifyDocumentCreatedForFrame(const WebFrame frame, const bool isMainFrame, const WebDOMDocument& document, const JSContextRef context, const JSValueRef window)
{
}

void WebView::notifyFrameDetached(const WebFrame frame)
{
}


void WebView::notifyRunLayoutTestsFinished()
{
}


void WebView::addMessageToConsole(const unsigned short* message, unsigned messageLength, const unsigned short* source, unsigned sourceLength, unsigned lineNumber)
{
}

void WebView::runJavaScriptAlert(const unsigned short* message, unsigned messageLength)
{
    QMessageBox::information(this, tr("JavaScript Alert:"), QString(reinterpret_cast<const QChar*>(message), messageLength));
}

bool WebView::runJavaScriptConfirm(const unsigned short* message, unsigned messageLength)
{
    return (QMessageBox::Ok == QMessageBox::question(this, tr("JavaScript Confirm:"),
                QString(reinterpret_cast<const QChar*>(message), messageLength),
                QMessageBox::Ok | QMessageBox::Cancel,
                QMessageBox::Ok));
}

bool WebView::runJavaScriptPrompt(const unsigned short* message, unsigned messageLength, const unsigned short* defaultValue, unsigned defaultValueLength, String& result)
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("JavaScript Prompt:"),
            QString(reinterpret_cast<const QChar*>(message), messageLength),
            QLineEdit::Normal,
            QString(reinterpret_cast<const QChar*>(defaultValue), defaultValueLength),
            &ok);
    if (!ok)
        return false;

    result = String(reinterpret_cast<const short unsigned int*>(text.unicode()), text.length());
    return true;
}

bool WebView::shouldInterruptJavaScript()
{
    return true;
}


void WebView::javascriptSourceParsed(const unsigned short* url, unsigned urlLength, const unsigned short* script, unsigned scriptLength)
{
}

void WebView::javascriptParsingFailed(const unsigned short* url, unsigned urlLength, const unsigned short* error, unsigned errorLength, int lineNumber)
{
}

void WebView::javascriptPaused(const unsigned short* stack, unsigned stackLength)
{
}

void WebView::javascriptContinued()
{
}


// All of these methods use transformed coordinates
void WebView::contentsSizeChanged(const int width, const int height) const
{
    m_contentSize.setWidth(width);
    m_contentSize.setHeight(height);
}

void WebView::scrollChanged(const int scrollX, const int scrollY) const
{
}

void WebView::zoomChanged(const bool isMinZoomed, const bool isMaxZoomed, const bool isAtInitialZoom, const double newZoom) const
{
}


void WebView::setPageTitle(const unsigned short* title, unsigned titleLength)
{
    QString str(reinterpret_cast<const QChar*>(title), titleLength);

    m_title = str;
    if (!str.isEmpty())
        emit sigSetTitle(m_url, str);
        //parentWidget()->setWindowTitle(str);
}

static bool isBigEndian()
{
    // FIXME
    return false;
}

// FIXME: use dstHeight and dstWidth <--- Maybe scale?
void WebView::blitToCanvas(const int dstX, const int dstY, const int dstWidth, const int dstHeight,
                          const int srcX, const int srcY, const int srcWidth, const int srcHeight)
{
#if DEBUG_PAINTING
    DEBUG_PRINT("WebView::blitToCanvas(%d, %d, %d, %d, %d, %d, %d, %d)\n", dstX, dstY, dstWidth, dstHeight, srcX, srcY, srcWidth, srcHeight);
#endif
    if (!srcHeight || !srcWidth)
        return;

    VGImageFormat format = VG_sRGB_565;
    if(isBigEndian())
        format = VG_sBGR_565;

    unsigned char* buffer = new unsigned char[srcHeight * srcWidth * sizeof(unsigned short)];
    int stride = sizeof(unsigned short) * srcWidth;
    vgReadPixels(buffer, stride, format, srcX, srcY, srcWidth, srcHeight);

    blitToCanvas(dstX, dstY, buffer, stride, srcX, srcY, srcWidth, srcHeight);

    delete[] buffer;
}

// 5-bit red channel to 8-bit red channel with 16-bit right shift.
// Alpha channel is also filled with 0xFF.
static const unsigned int rTable[32] = {
    4278190080u, 4278714368u, 4279238656u, 4279828480u,
    4280352768u, 4280877056u, 4281401344u, 4281991168u,
    4282515456u, 4283039744u, 4283564032u, 4284088320u,
    4284678144u, 4285202432u, 4285726720u, 4286251008u,
    4286840832u, 4287365120u, 4287889408u, 4288413696u,
    4289003520u, 4289527808u, 4290052096u, 4290576384u,
    4291100672u, 4291690496u, 4292214784u, 4292739072u,
    4293263360u, 4293853184u, 4294377472u, 4294901760u,
};

// 6-bit green channel to 8-bit green channel with 8-bit right shift.
static const unsigned int gTable[64] = {
    0, 1024, 2048, 3072,
    4096, 5120, 6144, 7168,
    8192, 9216, 10240, 11520,
    12544, 13568, 14592, 15616,
    16640, 17664, 18688, 19712,
    20736, 21760, 22784, 23808,
    24832, 25856, 26880, 27904,
    28928, 29952, 30976, 32000,
    33280, 34304, 35328, 36352,
    37376, 38400, 39424, 40448,
    41472, 42496, 43520, 44544,
    45568, 46592, 47616, 48640,
    49664, 50688, 51712, 52736,
    53760, 55040, 56064, 57088,
    58112, 59136, 60160, 61184,
    62208, 63232, 64256, 65280,
};

// 5-bit blue channel to 8-bit blue channel.
static const unsigned int bTable[32] = {
    0, 8, 16, 25,
    33, 41, 49, 58,
    66, 74, 82, 90,
    99, 107, 115, 123,
    132, 140, 148, 156,
    165, 173, 181, 189,
    197, 206, 214, 222,
    230, 239, 247, 255,
};

static inline unsigned int RGB565ToRGB32(unsigned short pixel16)
{
    unsigned int b = pixel16 & 0x1F;
    unsigned int g = (pixel16 >> 5) & 0x3F;
    unsigned int r = (pixel16 >> 11) & 0x1F;
    return rTable[r] | gTable[g] | bTable[b];
}

void WebView::blitToCanvas(const int dstX,
    const int dstY,
    const unsigned char* srcImage,
    const int srcStride,
    const int srcX,
    const int srcY,
    const int srcWidth,
    const int srcHeight)
{
#ifdef DEBUG_PAINTING
    DEBUG_PRINT("WebView::blitToCanvasImageVersion"
        "(dstX=%d, dstY=%d, srcImage=%p, srcStride=%d, "
        "srcX=%d, srcY=%d, srcWidth=%d, srcHeight=%d)\n",
        dstX, dstY, srcImage, srcStride,
        srcX, srcY, srcWidth, srcHeight);
    double startTime = currentTimeMS();
#endif
    srcImage += (srcX << BACKINGSTORE_IMAGE_PIXEL_SIZE_SHIFT) + srcY * srcStride;
    for (int i = 0; i < srcHeight; ++i) {
        unsigned int* dstImage = reinterpret_cast<unsigned int*>
            (m_screenImage->scanLine(i + dstY));
        for (int j = 0; j < srcWidth; ++j)
            dstImage[dstX + j] = RGB565ToRGB32(
                reinterpret_cast<const unsigned short*>(srcImage)[j]);
        srcImage += srcStride;
    }
    m_isThumbnailNeedUpdate = true;
#ifdef DEBUG_PAINTING
    DEBUG_PRINT("blitToCanvas time (MS): %.4lf\n",
        currentTimeMS() - startTime);
#endif
}

void WebView::blitFromBufferToBuffer(unsigned char* dst,
    const int dstStride,
    const int dstX,
    const int dstY,
    const unsigned char* src,
    const int srcStride,
    const int srcX,
    const int srcY,
    const int srcWidth,
    const int srcHeight)
{
#ifdef DEBUG_PAINTING
    DEBUG_PRINT("WebView::blitFromBufferToBuffer"
        "(%p, %d, %d, %d, %p, %d, %d, %d, %d, %d)\n",
        dst, dstStride, dstX, dstY, src, srcStride,
        srcX, srcY, srcWidth, srcHeight);
#endif
    dst += (dstX << BACKINGSTORE_IMAGE_PIXEL_SIZE_SHIFT)
        + dstY * dstStride;
    src += (srcX << BACKINGSTORE_IMAGE_PIXEL_SIZE_SHIFT)
        + srcY * srcStride;
    int scanLineLength = srcWidth << BACKINGSTORE_IMAGE_PIXEL_SIZE_SHIFT;
    for (int i = 0; i < srcHeight; ++i) {
        memcpy(dst, src, scanLineLength);
        dst += dstStride;
        src += srcStride;
    }
}

inline static unsigned short alphaBlendRGB565WithA4(unsigned int bgPixel,
    unsigned int pixel,
    unsigned char alpha)
{
    bgPixel = (bgPixel | (bgPixel << 16)) & 0x7E0F81F;
    pixel = (pixel | (pixel << 16)) & 0x7E0F81F;
    register unsigned int result = ((((pixel - bgPixel) * alpha) >> 4)
        + bgPixel) & 0x7E0F81F;
    return static_cast<unsigned short>((result & 0xFFFF) | (result >> 16));
}

void WebView::blendOntoCanvas(const int dstX,
    const int dstY,
    const unsigned char* srcImage,
    const int srcStride,
    const unsigned char* srcAlphaImage,
    const int srcAlphaStride,
    const char globalAlpha,
    const int srcX,
    const int srcY,
    const int srcWidth,
    const int srcHeight)
{
    // globalAlpha is 1 byte format which range is [0, 255].
    unsigned char ga = static_cast<unsigned char>(globalAlpha);

#ifdef DEBUG_PAINTING
    DEBUG_PRINT("WebView::blendOntoCanvas"
        "(dstX=%d, dstY=%d, srcImage=%p, srcStride=%d, srcAlphaImage=%p, "
        "srcAlphaStride=%d, globalAlpha=%d, srcX=%d, srcY=%d, srcWidth=%d"
        ", srcHeight=%d)\n",
        dstX, dstY, srcImage, srcStride, srcAlphaImage,
        srcAlphaStride, ga, srcX, srcY, srcWidth,
        srcHeight);

    double startTime = currentTimeMS();
#endif

    srcImage += (srcX << BACKINGSTORE_IMAGE_PIXEL_SIZE_SHIFT)
        + srcY * srcStride;

    // srcAlphaImage is 1 byte in pixel size.
    srcAlphaImage += srcX + srcY * srcAlphaStride;
    for (int i = 0; i < srcHeight; ++i) {
        register unsigned int* dstImage = reinterpret_cast<unsigned int*>
            (m_screenImage->scanLine(i + dstY));
        for (int j = 0; j < srcWidth; ++j) {
            // Alpha format is A4.
            register unsigned int alphaIndex = j >> 1;
            register unsigned char alpha = j % 2 ?
                srcAlphaImage[alphaIndex] >> 4 :
                srcAlphaImage[alphaIndex] & 0xF;

            // This branch is to optimize numer of calls to alphaBlend.
            if (alpha) {
                register unsigned int srcPixel =
                    *reinterpret_cast<const unsigned short*>
                    (srcImage + (j << BACKINGSTORE_IMAGE_PIXEL_SIZE_SHIFT));

                // We actually multiple alpha with rgb compounds
                // of srcPixel.
                srcPixel = alphaBlendRGB565WithA4(0, srcPixel, alpha);

                // This branch is to optimize numer of calls to alphaBlend.
                if (ga == 255)
                    dstImage[dstX + j] = RGB565ToRGB32(srcPixel);
                else {
                    // globalAlpha is hardcoded in Olympia WebKit porting.
                    // If it changes in the future, implement blend with
                    // globalAlpha.
#if COMPILER(MSVC)
                    DEBUG_PRINT("Implement me: Blend with globalAlpha\n", 0);
#else
                    DEBUG_PRINT("Implement me: Blend with globalAlpha\n");
#endif

                }
            }
        }
        srcImage += srcStride;
        srcAlphaImage += srcAlphaStride;
    }
#if DEBUG_PAINTING
    DEBUG_PRINT("blendOntoCanvas time (MS): %.4lf\n",
        currentTimeMS() - startTime);
#endif
    if (!m_blendedFirstScrollbar) {
        m_blendedFirstScrollbar = true;

        // If there are 2 scrollbars we only repaint when
        // the second scrollbar is blended.
        if (hasTwoScrollbars())
            return;
    }
    m_blendedFirstScrollbar = false;
    m_needsRendering = false;
    repaint();
}

bool WebView::hasTwoScrollbars() const
{
    return m_contentSize.width() > m_pageRect.width()
        && m_contentSize.height() > m_pageRect.height();
}

void WebView::invalidateWindow(const int dstX, const int dstY, const int dstWidth, const int dstHeight)
{
    m_needsRendering = false;
    repaint(dstX, dstY, dstWidth, dstHeight);
}

void WebView::lockCanvas()
{
}

void WebView::unlockCanvas()
{
}

void WebView::clearCanvas()
{
}


void WebView::inputFocusGained(unsigned int frameId, unsigned int inputFieldId, Olympia::Platform::OlympiaInputType type, unsigned int characterCount, unsigned int selectionStart, unsigned int selectionEnd)
{
    m_editable = true;
}

void WebView::inputFocusLost(unsigned int frameId, unsigned int inputFieldId)
{
    m_editable = false;
}

void WebView::inputTextChanged(unsigned int frameId, unsigned int inputFieldId, const unsigned short* text, unsigned int textLength, unsigned int selectionStart, unsigned int selectionEnd)
{
    m_editLength = textLength;

    if (!m_inputText.isEmpty()) {
        int pasteLen = m_inputText.length();
        ReplaceArguments arg = {selectionStart, selectionEnd, 0, pasteLen, pasteLen, 0, selectionStart + pasteLen, 0};
        AttributedText attr = {pasteLen, reinterpret_cast<unsigned short*>(m_inputText.data()), 0, NULL};
        m_page->replaceText(arg, attr);
        m_inputText.clear();
    }

    if (m_clearEditContent) {
        ReplaceArguments arg1 = {0, textLength, 0, 0, 0, 0, 0, 0};
        AttributedText attr1 = {0, NULL, 0, NULL};
        m_page->replaceText(arg1, attr1);
        m_clearEditContent = false;
    }

    if (m_pasteText) {
        QString text = QApplication::clipboard()->text();
        int pasteLen = text.length();
        ReplaceArguments arg2 = {selectionStart, selectionEnd, 0, pasteLen, pasteLen, 0, selectionStart+pasteLen, 0};
        AttributedText attr2 = {pasteLen, reinterpret_cast<unsigned short*>(text.data()), 0, NULL};
        m_page->replaceText(arg2, attr2);
        m_pasteText = false;
    }
}

void WebView::inputTextForElement(unsigned int requestedFrameId, unsigned int requestedElementId, unsigned int offset, int length, int selectionStart, int selectionEnd, const unsigned short* text, unsigned int textLength)
{
}

void WebView::inputFrameCleared(unsigned int frameId)
{
}

void WebView::inputSelectionChanged(unsigned int frameId, unsigned int inputFieldId, unsigned int selectionStart, unsigned int selectionEnd)
{
}

void WebView::inputSetNavigationMode(bool)
{
}


void WebView::selectionBounds(Olympia::Platform::IntRect start, Olympia::Platform::IntRect end)
{
}


void WebView::cursorChanged(Olympia::Platform::CursorType cursorType, const char* url, const int x, const int y)
{
}


void WebView::requestGeolocationPermission(Olympia::Platform::GeoTracker*)
{
}

void WebView::cancelGeolocationPermission(Olympia::Platform::GeoTracker*)
{
}

Olympia::Platform::NetworkStreamFactory* WebView::networkStreamFactory()
{
    return Olympia::Platform::OlympiaStreamFactory::instance();
}

Olympia::Platform::HttpStreamDebugger* WebView::httpStreamDebugger()
{
    return NULL;
}

bool WebView::runMessageLoopForJavaScript()
{
    return true;
}


void WebView::handleStringPattern(const unsigned short* pattern, unsigned int length)
{
}

void WebView::handleExternalLink(const Platform::NetworkRequest&, const unsigned short* context, unsigned int contextLength)
{
}


void WebView::resetBackForwardList(unsigned int listSize, unsigned int currentIndex)
{
    m_historyLength = listSize;
    m_currentHistoryIndex = currentIndex;
}


void WebView::openPopupList(bool multiple, const int size, const ScopeArray<String>& labels, bool* enableds, const int* itemType, bool* selecteds)
{
    DEBUG_PRINT("\nWebView::openPopupList size=%d\n", size);
    // no need to delete those new pointers in function, since dialog  will be deleted when it closed, and consequently delete all its children.
    QDialog* dialog = new QDialog(this);
    QListWidget* m_list = new QListWidget(dialog);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_list);
    dialog->setLayout(layout);

    for (int i = 0; i < size; i++) {
          String str = labels[i];
          QListWidgetItem *item = new QListWidgetItem(QString(reinterpret_cast<const QChar*>(str.characters()), str.length()), m_list, itemType[i]);
          if (selecteds[i])
              item->setFlags(Qt::ItemIsSelectable);
          if (enableds[i])
              item->setFlags(Qt::ItemIsEnabled);
    }
    connect(m_list, SIGNAL(itemActivated(QListWidgetItem*)), dialog, SLOT(accept()));
    if (dialog->exec() == QDialog::Accepted) {
        DEBUG_PRINT("\nWebView::popupListClosed choose %d \n",m_list->currentRow());
        m_page->popupListClosed(m_list->currentRow());
    }
}

void WebView::popupListClosed(const int size, bool* selecteds)
{
}

void WebView::popupListClosed(const int index)
{
}

void WebView::openDateTimePopup(const int type, const String& value, const String& min, const String& max, const double step)
{
}

void WebView::setDateTimeInput(const Olympia::WebKit::String& value)
{
}


void WebView::contextChanged(const Context&)
{
}


bool WebView::chooseFilenames(bool allowMultiple, const String& acceptTypes, const SharedArray<String>& initialFiles, unsigned int initialFileSize, SharedArray<String>& chosenFiles, unsigned int& chosenFileSize)
{
    QFileDialog dlg(this, tr("Choose file(s)"),
                    QDesktopServices::storageLocation(QDesktopServices::HomeLocation),
                    QString::fromUtf16(acceptTypes.characters(), acceptTypes.length()));
    dlg.setViewMode( QFileDialog::List );
    if (allowMultiple)
        dlg.setFileMode(QFileDialog::ExistingFiles);
    else
        dlg.setFileMode(QFileDialog::ExistingFile);

    // select files in this initial file list
    QString selectedFiles = "";
    for (int i = 0; i < initialFileSize; i++) {
        selectedFiles += "\"";
        selectedFiles = QString::fromUtf16(initialFiles[i].characters(), initialFiles[i].length());
        selectedFiles += "\" ";
    }
    dlg.selectFile(selectedFiles);

    // show file dialog
    if (dlg.exec() == QDialog::Accepted) {
        const QStringList& result = dlg.selectedFiles();
        chosenFileSize = result.size();
        chosenFiles.reset(new String[chosenFileSize]);

        QStringList::const_iterator it = result.begin();
        int cnt = 0;

        // set result to return.
        while (it != result.end()) {
            chosenFiles[cnt] = String((unsigned short*)it->unicode(), it->length());
            it++;
            cnt++;
        }
        return true;
    }
    return false;
}


void WebView::loadPluginForMimetype(int, int width, int height, const char* url, bool isHtml5Video, bool hasRenderer)
{
}

void WebView::notifyPluginRectChanged(int, Platform::IntRect rectChanged)
{
}

void WebView::destroyPlugin(int)
{
}

void WebView::playMedia(int)
{
}

void WebView::pauseMedia(int)
{
}

void WebView::scheduleCloseWindow()
{
}


// Database interface.
unsigned long long WebView::databaseQuota(const unsigned short* origin, unsigned originLength,
                                         const unsigned short* databaseName, unsigned databaseNameLength,
                                         unsigned long long totalUsage, unsigned long long originUsage,
                                         unsigned long long estimatedSize)
{
    return 0;
}


void WebView::setIconForUrl(const char* originalPageUrl, const char* finalPageUrl, const char* iconUrl)
{
    QNetworkAccessManager* manager = NetworkQt::getNetworkMgrInstance();
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotIconDataFinished(QNetworkReply*)));
    m_iconUrl = QString(iconUrl);
    QNetworkRequest request;
    request.setUrl(QUrl::fromEncoded(iconUrl));
    QNetworkReply* reply = manager->get(request);
}

void WebView::slotIconDataFinished(QNetworkReply* reply)
{
    if(reply && reply->url() == m_iconUrl) {
        QVariant statsCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if(statsCode.toInt() == 200) {
            QByteArray bytes = reply->readAll();
            QPixmap pixmap;
            pixmap.loadFromData(bytes);
            m_urlIcon = QIcon(pixmap);
            disconnect(NetworkQt::getNetworkMgrInstance(), SIGNAL(finished(QNetworkReply*)), this, SLOT(slotIconDataFinished(QNetworkReply*)));
            emit sigSetIcon(m_url, m_iconUrl.toString(), m_urlIcon);
        }
    }
    else {
        emit sigNoFaviconLoaded();
    }
}

Olympia::WebKit::String WebView::getErrorPage(int errorCode, const char* errorMessage, const char* url)
{
    return "";
}


void WebView::willDeferLoading()
{
}

void WebView::didResumeLoading()
{
}


// headers is a list of alternating key and value
void WebView::setMetaHeaders(const ScopeArray<String>& headers, unsigned int headersSize)
{
    m_metaHeaders.clear();
    for(int i = 0; i < headersSize; i = i+2) {
        QString key = QString(reinterpret_cast<const QChar*>(headers[i].characters()), headers[i].length());
        QString value = QString(reinterpret_cast<const QChar*>(headers[i+1].characters()), headers[i+1].length());
        m_metaHeaders.insert(key, value);
    }
}

void WebView::changeEventHandler()
{
    Q_ASSERT(m_eventHandler);
    m_eventHandler->terminateCurrentWork();
    delete m_eventHandler;

    switch (m_naviMode) {
    case Selection:
        m_eventHandler = new SelectionEventHandler(this);
        break;
    case MultiTouch:
        m_eventHandler = new MultiTouchEventHandler(this);
        break;
    case Normal:
    default:
        m_eventHandler = new NormalEventHandler(this);
    }
}

void WebView::setSelect()
{
    setNavigationMode(WebView::Selection);
}

void WebView::unsetSelect()
{
    setNavigationMode(WebView::Normal);
}

void WebView::setNavigationMode(NavigationMode m)
{
    if (m_naviMode == m)
        return;

    m_naviMode = m;
    changeEventHandler();
}

void WebView::setMultiTouchMode()
{
    setNavigationMode(WebView::MultiTouch);
}

void WebView::unsetMultiTouchMode()
{
    setNavigationMode(WebView::Normal);
}

void WebView::copy()
{
    QString text = selectedText();
    text.remove(" ");
    text.remove("\n");

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

QString WebView::selectedText()
{
    if (!m_page)
        return QString();

    Olympia::WebKit::String str = m_page->selectedText();
    return QString(reinterpret_cast<const QChar*>(str.characters()), str.length());
    // It's dangerous to reinterpret_cast from two different types, but no other ways
    // to convert from Olympia::WebKit::String to QString, will leave it as is for now.
}

void WebView::dispatchTouchEvent(Olympia::Platform::TouchEvent te)
{
    if (!m_page)
        return;

    m_page->touchEvent(te);
}

// QWidget inferfaces:
void WebView::paintEvent(QPaintEvent* event)
{
    const QRect& dirtyRect = event->rect();
    if (!m_needsRendering && m_screenImage) {
#ifdef DEBUG_PAINTING
        double startTime = currentTimeMS();
#endif
        QPainter painter(this);
        painter.drawImage(dirtyRect, *m_screenImage, dirtyRect);
#ifdef DEBUG_PAINTING
        DEBUG_PRINT("Painting time (MS): %.4lf\n", currentTimeMS() - startTime);
#endif
    } else if (m_needsRendering && m_page) {
        m_page->backingStore()->repaint(dirtyRect.x(),
            dirtyRect.y(),
            dirtyRect.width(),
            dirtyRect.height(),
            true,
            false,
            false);
        m_needsRendering = false;
    }
    m_eventHandler->extraPaintJob();
}

void WebView::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_ASSERT(m_eventHandler);
    m_eventHandler->mouseDoubleClickEvent(event);
}

void WebView::mousePressEvent(QMouseEvent* event)
{
    Q_ASSERT(m_eventHandler);
    m_eventHandler->mousePressEvent(event);
}

void WebView::mouseReleaseEvent(QMouseEvent* event)
{
    Q_ASSERT(m_eventHandler);
    m_eventHandler->mouseReleaseEvent(event);
}

void WebView::mouseMoveEvent(QMouseEvent* event)
{
    Q_ASSERT(m_eventHandler);
    m_eventHandler->mouseMoveEvent(event);
}

void WebView::wheelEvent(QWheelEvent* event)
{
    Q_ASSERT(m_eventHandler);
    m_eventHandler->wheelEvent(event);
}

void WebView::keyPressEvent(QKeyEvent * event)
{
    Q_ASSERT(m_eventHandler);
    m_eventHandler->keyPressEvent(event);
}

void WebView::inputMethodEvent(QInputMethodEvent* ev)
{
    if (!ev->commitString().isEmpty())
        m_inputText = ev->commitString();
    ev->accept();
}

void WebView::resizeEvent(QResizeEvent* event)
{
    if(m_screenImage) {
        delete m_screenImage;
        m_screenImage = new QImage(size(), QImage::Format_RGB32);
        m_screenImage->fill(QColor(255, 255, 255).rgb());
        m_pageRect = QRect(QPoint(0, 0), size());
        m_needsRendering = false;
    }
    QWidget::resizeEvent(event);
}


QPoint WebView::validateScrollPoint(const QPoint& point)
{
    QPoint p(point);
    if (p.x() < 0)
        p.setX(0);
    if (p.y() < 0)
        p.setY(0);

    if (p.x() > m_contentSize.width() - width())
        p.setX(m_contentSize.width() - width());
    if (p.y() > m_contentSize.height() - height())
        p.setY(m_contentSize.height() - height());

    return p;
}

void WebView::animateScroll()
{
    // This value is larger, the scrolling speed drops faster:
    const static int ScrollAnimationInertia = 5;

    if (m_nextScrollStepLength.isNull()) {
        m_scrollTimer.stop();
        return;
    }

    QPoint newPoint = m_lastContentPos + m_nextScrollStepLength;
    QPoint vPoint = validateScrollPoint(newPoint);
    if (m_lastContentPos == vPoint) {
        m_scrollTimer.stop();
        return;
    }

    QPoint delta = m_lastContentPos - vPoint;
    scrollBy(delta.x(), delta.y());
    m_lastContentPos = vPoint;
    m_nextScrollStepLength = QPoint(qMax(qAbs(m_nextScrollStepLength.x()) - ScrollAnimationInertia, 0) * (m_nextScrollStepLength.x() < 0 ? -1 : 1),
        qMax(qAbs(m_nextScrollStepLength.y()) - ScrollAnimationInertia, 0) * (m_nextScrollStepLength.y() < 0 ? -1 : 1));

    if (newPoint.x() != vPoint.x() && newPoint.y() != vPoint.y())
        m_scrollTimer.stop();
}

void WebView::scrollBy(int deltaX, int deltaY)
{
    Platform::IntPoint pos = m_page->scrollPosition();
    int maxScrollH = m_contentSize.width() - width();
    int maxScrollV = m_contentSize.height() - height();

    int x = pos.x() - deltaX;
    int y = pos.y() - deltaY;
    if (x > maxScrollH)
        x = maxScrollH;
    if (x < 0)
        x = 0;

    if (y > maxScrollV)
        y = maxScrollV;
    if (y < 0)
        y = 0;
    m_page->setScrollPosition(Platform::IntPoint(x, y));
}

void WebView::triggerRender()
{
    if (isVisible() && hasIdleJobs())
        m_page->backingStore()->renderOnIdle();
}

bool WebView::hasIdleJobs()
{
    return m_page && m_page->backingStore() && m_page->backingStore()->hasIdleJobs();
}

QPixmap WebView::thumbnail(const QSize& size) const
{
    if (m_isThumbnailNeedUpdate) {
        m_thumbnailCache = QPixmap::fromImage(*m_screenImage).scaled(size);
        m_isThumbnailNeedUpdate = false;
    }
    return m_thumbnailCache;
}

void WebView::setWebPageVisible(bool isVisible)
{
    if (!m_page)
        return;
    m_page->setVisible(isVisible);
}

int WebView::historyLength()
{
    return m_historyLength;
}

int WebView::currentHistoryIndex()
{
    return m_currentHistoryIndex;
}

WebPage* WebView::createWindow(int x, int y, int width, int height, unsigned flags)
{
    WebView* tab = 0;
    emit requestCreateTab(&tab, x, y, width, height, flags);
    if (tab) {
        tab->createWebPageIfNeeded();
        return tab->webPage();
    }
    else
        return 0;
}

void WebView::slotOrientationChanged()
{
    if (m_page) {
        m_page->setScreenOrientation(90);
        m_page->setScreenRotated(Platform::IntSize(width(), height()), Platform::IntSize(width(), height()), Platform::IntRect(0, 0, width(), height()));
    }
}

void WebView::slotWindowSizeChanged()
{
    if (m_page)
        m_page->setScreenSize(Platform::IntSize(width(), height()), Platform::IntSize(width(), height()), Platform::IntRect(0, 0, width(), height()));
}

} // namespace Browser
} // namespace Olympia

