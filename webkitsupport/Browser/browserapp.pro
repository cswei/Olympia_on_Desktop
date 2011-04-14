TEMPLATE = app
TARGET = browser
DESTDIR = $$PWD
includes.path = $$[QT_INSTALL_HEADERS]

DEFINES += JS_NO_EXPORT=1\
           WTF_PLATFORM_OLYMPIA=1 \
           BUILDING_OLYMPIA__ \
           WTF_USE_PTHREADS=1 \
           SYSTEMINCLUDE=/include \
           ENABLE_AS_IMAGE=1 \
           ENABLE_BLOB_SLICE=0 \ 
           ENABLE_CHANNEL_MESSAGING=1 \ 
           ENABLE_DATABASE=1 \
           ENABLE_DATAGRID=0 \
           ENABLE_DATALIST=1 \
           ENABLE_DOM_STORAGE=1 \ 
           ENABLE_EVENTSOURCE=1 \
           ENABLE_FAST_MOBILE_SCROLLING=1 \ 
           ENABLE_FILTERS=1 \
           ENABLE_GEOLOCATION=1 \ 
           ENABLE_GLIB_SUPPORT=1 \ 
           ENABLE_ICONDATABASE=1 \
           ENABLE_IMAGE_DECODER_DOWN_SAMPLING=1 \ 
           ENABLE_INSPECTOR=0 \
           ENABLE_JAVASCRIPT_DEBUGGER=1 \ 
           ENABLE_JIT=0 \
           ENABLE_MATHML=0 \ 
           ENABLE_NETSCAPE_PLUGIN_API=0 \
           ENABLE_NOTIFICATIONS=0 \
           ENABLE_OFFLINE_WEB_APPLICATIONS=1 \ 
           ENABLE_ORIENTATION_EVENTS=0 \
           ENABLE_PROGRESS_TAG=0 \
           ENABLE_RUBY=0 \ 
           ENABLE_SANDBOX=0 \
           ENABLE_SHARED_WORKERS=1 \ 
           ENABLE_SVG=0 \
           ENABLE_SVG_ANIMATION=0 \ 
           ENABLE_SVG_FONTS=0 \
           ENABLE_SVG_FOREIGN_OBJECT=0 \ 
           ENABLE_SVG_USE=0 \
           ENABLE_VIDEO=0 \
           ENABLE_WEB_SOCKETS=0 \ 
           ENABLE_WML=0 \
           ENABLE_WORKERS=1 \ 
           ENABLE_XHTMLMP=0 \
           ENABLE_XPATH=1 \
           ENABLE_XSLT=0 \
           ENABLE_VIEWPORT_REFLOW=1 \ 
           JS_BLOCK_TOTAL_SIZE=2097152 \ 
           ENABLE_SINGLE_THREADED \



INCLUDEPATH += ../olympia/WebKit/blackberry/Api \
               ../olympia/WebCore \
               ../olympia/WebCore/platform/graphics \
               ../webkitsupport/blackberry \
               ../olympia/JavaScriptCore \
               ../olympia/JavaScriptCore/API \
               ../webkitsupport/egl/include/ \
               ../webkitsupport/windows/openvg/include \
               ../webkitsupport/windows/openvg/include/VG \
               ../olympia/WebCore/bindings/cpp \
               ../olympia/JavaScriptCore/ForwardingHeaders \
               ../olympia \
               ../olympia/WebCore/icu/unicode \
               ./widgets/AddressBar \
               ./widgets/StatusBar \
               ./widgets/MainWindow \
               ./widgets/Tabs \
               ./ \
               $$(QTDIR)/include/QtNetwork\
               ../olympia/build/DerivedSources \
               ../olympia/WebCore/platform \
               ../olympia/WebCore/platform/text\
               ../olympia/WebCore/platform/network/blackberry\
               ../olympia/JavaScriptCore/runtime\
               ../olympia/JavaScriptCore/wtf \
               ../olympia/JavaScriptCore/wtf/text \
               ../webkitsupport/blackberry/streams \
               ../webkitsupport/blackberry/streams/qt \
               ../olympia/WebCore/platform/graphics/openvg \               
              
contains(DEFINES, WIN32) {
INCLUDEPATH += ../webkitsupport/windows/icu/include\
               ../webkitsupport/windows \
               ../webkitsupport/windows/posix \ 
               ../webkitsupport/windows/pthread/include \
               ../webkitsupport/windows/freetype/include\
               ../olympia/JavaScriptCore/os-win32\
               ../webkitsupport/egl/include \
               ../webkitsupport/egl/include/EGL\

DEFINES += OLYMPIA_WINDOWS=1
} else {
INCLUDEPATH += /usr/include/freetype2/ \
              ../webkitsupport/linux/amanithvg_pre_release/ \
              ../webkitsupport/linux/amanithvg_pre_release/VG\
              ../webkitsupport/linux/amanithvg_pre_release/EGL
DEFINES += OLYMPIA_LINUX=1
}

FORMS += \
          ConfigWidget.ui \

SOURCES += \
          BrowserWidgetsFactory.cpp \
          ConfigWidget.cpp \
          Launcher.cpp \
          main.cpp \
          NetworkQt.cpp \
          WebViewQt.cpp \
          TabbedView.cpp \
          TabListView.cpp \
          TabThumbButton.cpp \
          History.cpp \
          ./widgets/MainWindow/MainWindowAbstract.cpp \
          ./widgets/MainWindow/OlympiaMobileMainWindow.cpp \
          ./widgets/MainWindow/OlympiaDesktopMainWindow.cpp \
          ./widgets/AddressBar/AddressBar.cpp \
          ./widgets/AddressBar/MatchedHistView.cpp \
          ./widgets/AddressBar/OlympiaMobileAddressBar.cpp \
          ./widgets/StatusBar/StatusBar.cpp \
          ./widgets/StatusBar/OlympiaMobileStatusBar.cpp \
          ./widgets/Tabs/Tabs.cpp \
          ./widgets/Tabs/OlympiaDesktopTabs.cpp \
          ./widgets/Tabs/OlympiaMobileTabs.cpp \
          ../webkitsupport/blackberry/LocalizeResource.cpp \
          ../webkitsupport/blackberry/OlympiaPlatformFileSystem.cpp\
          ../webkitsupport/blackberry/ObjectAllocator.cpp\
          ../webkitsupport/blackberry/streams/NetworkRequest.cpp\
          ../webkitsupport/blackberry/streams/FilterStream.cpp\
          ../webkitsupport/blackberry/streams/qt/OlympiaNetworkCookieJarQt.cpp\
          ../webkitsupport/blackberry/streams/qt/OlympiaHttpStreamQt.cpp\
          ../webkitsupport/blackberry/OlympiaPlatformTextCodec.cpp\
          ../webkitsupport/blackberry/OlympiaPlatformMisc.cpp \
          ../webkitsupport/blackberry/OutOfMemoryHandlerInterface.cpp\
          ../webkitsupport/blackberry/OlympiaPlatformGeoTracker.cpp\
          ../webkitsupport/blackberry/OlympiaPlatformCookieJar.cpp\
          ../webkitsupport/blackberry/OlympiaPlatformText.cpp\
          ../webkitsupport/blackberry/OlympiaPlatformTimer.cpp\
          ../webkitsupport/blackberry/OlympiaStreamFactory.cpp\
          ../webkitsupport/blackberry/OlympiaPlatformMemory.cpp\
          ../webkitsupport/blackberry/OlympiaPlatformTextBreakIterators.cpp\
          ../webkitsupport/blackberry/OlympiaPlatformIDN.cpp\
          ../webkitsupport/blackberry/OlympiaPlatformAssert.cpp\
          ../webkitsupport/blackberry/OlympiaPlatformClient.cpp\
          ../webkitsupport/blackberry/OlympiaPlatformSettings.cpp\
          ../webkitsupport/blackberry/text/TextGraphicsContextOpenVG.cpp\
          ../webkitsupport/blackberry/text/TextFontFreeType.cpp\
          ../webkitsupport/blackberry/text/TextFontLoaderFreeType.cpp\
          ../webkitsupport/blackberry/text/TextEngineFreeType.cpp \
          ../olympia/build/DerivedSources/BuildInformation.cpp 


HEADERS += \
           BrowserWidgetsFactory.h \
           ConfigWidget.h \
           Launcher.h \
           Constant.h \
           NetworkQt.h \
           WebViewQt.h \
           TabbedView.h \
           TabListView.h \
           TabThumbButton.h \
           History.h \
           ./widgets/MainWindow/MainWindowAbstract.h \
           ./widgets/MainWindow/OlympiaMobileMainWindow.h \
           ./widgets/MainWindow/OlympiaDesktopMainWindow.h \
           ./widgets/AddressBar/AddressBar.h \
           ./widgets/AddressBar/MatchedHistView.h \
           ./widgets/AddressBar/OlympiaMobileAddressBar.h \
           ./widgets/StatusBar/StatusBar.h \
           ./widgets/StatusBar/OlympiaMobileStatusBar.h \
           ./widgets/Tabs/Tabs.h \
           ./widgets/Tabs/OlympiaDesktopTabs.h \
           ./widgets/Tabs/OlympiaMobileTabs.h\
           ../webkitsupport/blackberry/streams/qt/OlympiaNetworkCookieJarQt.h \
           ../webkitsupport/blackberry/streams/qt/OlympiaHttpStreamQt.h \


LIBS += -L$$(QTDIR)/lib \
        -L../olympia/build/JavaScriptCore/wtf/debug \
        -L../olympia/build/JavaScriptCore/wtf/\
        -L../olympia/build/JavaScriptCore/debug \
        -L../olympia/build/JavaScriptCore/\
        -L../olympia/build/WebKit/debug \
        -L../olympia/build/WebKit/\
        -L../olympia/build/WebCore/debug \
        -L../olympia/build/WebCore/\

contains(DEFINES, WIN32) {
LIBS += -L../webkitsupport/windows/png/lib -llibpng \
        -L../webkitsupport/windows/jpeg/lib -ljpeg \
        -L../webkitsupport/windows/sqlite3/lib -lsqlite3 \
        -L../webkitsupport/windows/libxml2/lib -llibxml2 \
        -L../webkitsupport/windows/iconv/lib -liconv \
        -L../webkitsupport/windows/freetype/lib -lfreetype244 \
        -L../webkitsupport/windows/pthread/lib -lpthreadvc2 \
        -L../webkitsupport/windows/openvg/lib -llibAmanithVG_SRE \
        -L../webkitsupport/egl/lib -llibEGL_AMWin32 \
        -L../webkitsupport/windows/icu/lib -licuuc \
        -L../webkitsupport/ \
        -lwinmm \
        -lQtCore4 -lQtGui4 -lQtNetwork4 -lQtXML4

} else {
LIBS += -lQtCore -lQtGui -lQtNetwork -lQtXml -ljpeg -lpng -licuuc -lxml2 -lsqlite3 \
        -L../webkitsupport/linux/amanithvg_pre_release/x86 -lAmanithEGL -lAmanithVG -lAmanithEGL_priv 
}

QT += sql

LIBS +=  -lwtf -ljavascriptcore -lwebcore -lwebkit 
