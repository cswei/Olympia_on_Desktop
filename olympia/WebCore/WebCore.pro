# WebCore - qmake build info
CONFIG += building-libs
CONFIG += depend_includepath

symbian: {
    TARGET.EPOCALLOWDLLDATA=1
    TARGET.CAPABILITY = All -Tcb
    isEmpty(QT_LIBINFIX) {
        TARGET.UID3 = 0x200267C2
    } else {
        TARGET.UID3 = 0xE00267C2
    }
    webkitlibs.sources = QtWebKit$${QT_LIBINFIX}.dll
    CONFIG(QTDIR_build): webkitlibs.sources = $$QMAKE_LIBDIR_QT/$$webkitlibs.sources
    webkitlibs.path = /sys/bin
    vendorinfo = \
        "; Localised Vendor name" \
        "%{\"Nokia, Qt\"}" \
        " " \
        "; Unique Vendor name" \
        ":\"Nokia, Qt\"" \
        " "
    webkitlibs.pkg_prerules = vendorinfo

    webkitbackup.sources = ../WebKit/qt/symbian/backup_registration.xml
    webkitbackup.path = /private/10202D56/import/packages/$$replace(TARGET.UID3, 0x,)

    DEPLOYMENT += webkitlibs webkitbackup

    # Need to guarantee that these come before system includes of /epoc32/include
    MMP_RULES += "USERINCLUDE rendering"
    MMP_RULES += "USERINCLUDE platform/text"
    symbian-abld|symbian-sbsv2 {
        # RO text (code) section in qtwebkit.dll exceeds allocated space for gcce udeb target.
        # Move RW-section base address to start from 0xE00000 instead of the toolchain default 0x400000.
        QMAKE_LFLAGS.ARMCC += --rw-base 0xE00000
        MMP_RULES += ALWAYS_BUILD_AS_ARM
    }  else {
        QMAKE_CFLAGS -= --thumb
        QMAKE_CXXFLAGS -= --thumb
    }
    CONFIG(release, debug|release): QMAKE_CXXFLAGS.ARMCC += -OTime -O3
}

isEmpty(OUTPUT_DIR): OUTPUT_DIR = ..
include($$PWD/../WebKit.pri)

olympia-*|win32-msvc-fledge {
    include($$PWD/../WebKitTools/DumpRenderTree/blackberry/DumpRenderTree.pri)
}

TEMPLATE = lib
TARGET = OlympiaWebKit

!olympia-* {
    CONFIG += staticlib
}

win32-msvc-fledge {
    CONFIG += dll
    CONFIG += embedded
    QMAKE_LFLAGS += /DEF:$$PWD/../JavaScriptCore/BlackBerryExportsJSC.def
}

contains(QT_CONFIG, embedded):CONFIG += embedded

CONFIG(standalone_package) {
    isEmpty(WC_GENERATED_SOURCES_DIR):WC_GENERATED_SOURCES_DIR = $$PWD/generated
    isEmpty(JSC_GENERATED_SOURCES_DIR):JSC_GENERATED_SOURCES_DIR = $$PWD/../JavaScriptCore/generated

    PRECOMPILED_HEADER = $$PWD/../WebKit/qt/WebKit_pch.h
} else {
    isEmpty(WC_GENERATED_SOURCES_DIR):WC_GENERATED_SOURCES_DIR = generated
    isEmpty(JSC_GENERATED_SOURCES_DIR):JSC_GENERATED_SOURCES_DIR = ../JavaScriptCore/generated

    !CONFIG(release, debug|release) {
        OBJECTS_DIR = obj/debug
    } else { # Release
        OBJECTS_DIR = obj/release
    }

}

CONFIG(QTDIR_build) {
    include($$QT_SOURCE_TREE/src/qbase.pri)
} else {
    DESTDIR = $$OUTPUT_DIR/lib
    !static: DEFINES += QT_MAKEDLL
    symbian: TARGET =$$TARGET$${QT_LIBINFIX}
}
include($$PWD/../WebKit/qt/qtwebkit_version.pri)
VERSION = $${QT_WEBKIT_MAJOR_VERSION}.$${QT_WEBKIT_MINOR_VERSION}.$${QT_WEBKIT_PATCH_VERSION}

unix {
    QMAKE_PKGCONFIG_REQUIRES = QtCore QtGui QtNetwork
}

CONFIG -= warn_on
*-g++*:QMAKE_CXXFLAGS += -Wreturn-type -fno-strict-aliasing

unix:!mac:*-g++*:QMAKE_CXXFLAGS += -ffunction-sections -fdata-sections 
#unix:!mac:*-g++*:QMAKE_LFLAGS += -Wl,--gc-sections
linux*-g++*:QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF

CONFIG(release):!CONFIG(standalone_package) {
    contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
    unix:contains(QT_CONFIG, reduce_relocations):CONFIG += bsymbolic_functions
}

DEFINES += BUILD_WEBKIT

# Remove whole program optimizations due to miscompilations
win32-msvc2005|win32-msvc2008:{
    QMAKE_CFLAGS_RELEASE -= -GL
    QMAKE_CXXFLAGS_RELEASE -= -GL
}

# Pick up 3rdparty libraries from INCLUDE/LIB just like with MSVC
win32-g++ {
    TMPPATH            = $$quote($$(INCLUDE))
    QMAKE_INCDIR_POST += $$split(TMPPATH,";")
    TMPPATH            = $$quote($$(LIB))
    QMAKE_LIBDIR_POST += $$split(TMPPATH,";")
}

# Assume that symbian OS always comes with sqlite
symbian:!CONFIG(QTDIR_build): CONFIG += system-sqlite



maemo5|symbian|embedded {
    DEFINES += ENABLE_FAST_MOBILE_SCROLLING=1
}

maemo5|symbian {
    DEFINES += WTF_USE_QT_MOBILE_THEME=1
}

contains(DEFINES, WTF_USE_QT_MOBILE_THEME=1) {
    DEFINES += ENABLE_NO_LISTBOX_RENDERING=1
}

include($$PWD/../JavaScriptCore/JavaScriptCore.pri)
addJavaScriptCoreLib(../JavaScriptCore)

# MARK: Depending on the resolution of WebKit Bug #36312 <http://www.webkit.org/b/36312> the name/use of this flag may change.
!contains(DEFINES, ENABLE_META_VIEWPORT=.): DEFINES += ENABLE_META_VIEWPORT=1


# HTML5 Media Support
# We require phonon. QtMultimedia support is disabled currently.
!contains(DEFINES, ENABLE_VIDEO=.) {
    olympia-* {
        DEFINES += ENABLE_VIDEO=1 ENABLE_PLUGIN_PROXY_FOR_VIDEO=1
    }
    else {
    DEFINES -= ENABLE_VIDEO=1
    DEFINES += ENABLE_VIDEO=0

    contains(QT_CONFIG, phonon) {
        DEFINES -= ENABLE_VIDEO=0
        DEFINES += ENABLE_VIDEO=1
    }
    }
}

# Extract sources to build from the generator definitions
defineTest(addExtraCompiler) {
    isEqual($${1}.wkAddOutputToSources, false): return(true)

    outputRule = $$eval($${1}.output)
    input = $$eval($${1}.input)
    input = $$eval($$input)

    for(file,input) {
        base = $$basename(file)
        base ~= s/\..+//
        newfile=$$replace(outputRule,\\$\\{QMAKE_FILE_BASE\\},$$base)
        SOURCES += $$newfile
    }
    SOURCES += $$eval($${1}.wkExtraSources)
    export(SOURCES)

    return(true)
}

linux-* {
    PKGCONFIG += libxml-2.0
    INCLUDEPATH += /usr/include/libxml2
    LIBS += -lxml2 -lpthread
}

include(WebCore.pri)

DEFINES += WTF_USE_JAVASCRIPTCORE_BINDINGS=1  WTF_USE_OLYMPIA_NATIVE_UNICODE=1

win32-*|olympia-* {
    INCLUDEPATH += /usr/include/freetype2
    INCLUDEPATH += /usr/include/jpeg
    INCLUDEPATH += /usr/include/libpng
    INCLUDEPATH += /usr/include/libxml2
    INCLUDEPATH += /usr/include/libxslt
    INCLUDEPATH += /usr/include/tid
    INCLUDEPATH += /usr/include/zlib
    INCLUDEPATH += /usr/include/webkitplatform
    INCLUDEPATH += /usr/include/posix
    INCLUDEPATH += /usr/include/sqlite

    contains(DEFINES, OLYMPIA_LINUX) {
        DEFINES -= WTF_USE_OLYMPIA_NATIVE_UNICODE=1
        DEFINES += USE_FREETYPE_RENDER=0
        DEFINES -= ENABLE_TILED_BACKING_STORE=1
        DEFINES += ENABLE_TILED_BACKING_STORE=0
        DEFINES -= ENABLE_INSPECTOR=0
        DEFINES += ENABLE_INSPECTOR=1
        INCLUDEPATH += ../../webkitsupport/blackberry
        INCLUDEPATH += ../../webkitsupport/blackberry/streams
        INCLUDEPATH += ../../webkitsupport/blackberry/streams/qt
        INCLUDEPATH += ../../Browser
        INCLUDEPATH += /usr/include/unicode 
        INCLUDEPATH += $$PWD/../../webkitsupport/linux/amanithvg_pre_release/
        INCLUDEPATH += $$PWD/../../webkitsupport/linux/amanithvg_pre_release/VG
        INCLUDEPATH += $$PWD/../../webkitsupport/linux/amanithvg_pre_release/EGL

        LIBS += -L$$PWD/../../webkitsupport/linux/amanithvg_pre_release/x86
        LIBS += -lAmanithVG -lAmanithEGL -lAmanithEGL_priv

        HEADERS += \ 
                  $$PWD/../../webkitsupport/blackberry/OlympiaStreamFactory.h \
                  $$PWD/../../webkitsupport/blackberry/streams/qt/OlympiaHttpStreamQt.h \
                  $$PWD/../../webkitsupport/blackberry/streams/qt/OlympiaNetworkCookieJarQt.h \
                  $$PWD/../../webkitsupport/blackberry/text/TextEngineFreeType.h \
                  $$PWD/../../webkitsupport/blackberry/text/TextFontFreeType.h \
                  $$PWD/../../webkitsupport/blackberry/text/TextFontLoaderFreeType.h \
                  $$PWD/../../webkitsupport/blackberry/text/TextGraphicsContextOpenVG.h \
                  $$PWD/../../webkitsupport/blackberry/text/TextUtilsFreeType.h

        SOURCES += \
                  $$PWD/../../webkitsupport/blackberry/LocalizeResource.cpp \
                  $$PWD/../../webkitsupport/blackberry/ObjectAllocator.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformAssert.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformTimer.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformTextBreakIterators.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformIDN.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformClient.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformCookieJar.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformFileSystem.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformGeoTracker.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformMemory.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformMisc.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformText.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformTextCodec.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaPlatformSettings.cpp \
                  $$PWD/../../webkitsupport/blackberry/OlympiaStreamFactory.cpp \
                  $$PWD/../../webkitsupport/blackberry/OutOfMemoryHandlerInterface.cpp \
                  $$PWD/../../webkitsupport/blackberry/streams/FilterStream.cpp \
                  $$PWD/../../webkitsupport/blackberry/streams/NetworkRequest.cpp \
                  $$PWD/../../webkitsupport/blackberry/streams/qt/OlympiaHttpStreamQt.cpp \
                  $$PWD/../../webkitsupport/blackberry/streams/qt/OlympiaNetworkCookieJarQt.cpp \
                  $$PWD/../../webkitsupport/blackberry/text/TextEngineFreeType.cpp \
                  $$PWD/../../webkitsupport/blackberry/text/TextFontFreeType.cpp \
                  $$PWD/../../webkitsupport/blackberry/text/TextFontLoaderFreeType.cpp \
                  $$PWD/../../webkitsupport/blackberry/text/TextGraphicsContextOpenVG.cpp
    }
    contains(DEFINES, ENABLE_OLYMPIA_DEBUG_MEMORY=1) {
        LIBS += -L$$(BBNDKROOT)/bbndk-webkit/MemoryLeakTracker/Release
        LIBS += -lMemoryLeakTracker
    }
    LIBS += -lQtCore -lQtGui -lQtNetwork -lxslt
}


INCLUDEPATH = \
    $$PWD \
    $$PWD/accessibility \
    $$PWD/bindings/cpp \
    $$PWD/bindings/js \
    $$PWD/bridge \
    $$PWD/bridge/c \
    $$PWD/bridge/jsc \
    $$PWD/css \
    $$PWD/dom \
    $$PWD/dom/default \
    $$PWD/editing \
    $$PWD/history \
    $$PWD/html \
    $$PWD/html/canvas \
    $$PWD/inspector \
    $$PWD/loader \
    $$PWD/loader/appcache \
    $$PWD/loader/archive \
    $$PWD/loader/icon \
    $$PWD/mathml \
    $$PWD/notifications \
    $$PWD/page \
    $$PWD/page/animation \
    $$PWD/platform \
    $$PWD/platform/animation \
    $$PWD/platform/graphics \
    $$PWD/platform/graphics/filters \
    $$PWD/platform/graphics/transforms \
    $$PWD/platform/image-decoders \
    $$PWD/platform/image-encoders \
    $$PWD/platform/mock \
    $$PWD/platform/image-decoders/bmp \
    $$PWD/platform/image-decoders/gif \
#    $$PWD/platform/image-decoders/ico \
    $$PWD/platform/image-decoders/jpeg \
    $$PWD/platform/image-decoders/png \
#    $$PWD/platform/image-decoders/xbm \
    $$PWD/platform/network \
    $$PWD/platform/sql \
    $$PWD/platform/text \
    $$PWD/platform/text/transcoder \
    $$PWD/plugins \
    $$PWD/rendering \
    $$PWD/rendering/style \
    $$PWD/storage \
    $$PWD/svg \
    $$PWD/svg/animation \
    $$PWD/svg/graphics \
    $$PWD/svg/graphics/filters \
    $$PWD/websockets \
    $$PWD/wml \
    $$PWD/workers \
    $$PWD/xml \
    $$WC_GENERATED_SOURCES_DIR \
    $$INCLUDEPATH

INCLUDEPATH = \
    $$PWD/bridge/blackberry \
    $$PWD/page/blackberry \
    $$PWD/platform/graphics/blackberry \
    $$PWD/platform/graphics/openvg \
    $$PWD/platform/network/blackberry \
    $$PWD/platform/text/blackberry \
    $$PWD/platform/blackberry \
    $$PWD/../WebKit/blackberry/Api \
    $$PWD/../WebKit/blackberry/WebCoreSupport \
    $$PWD/../WebKit/blackberry/WebKitSupport \
    $$PWD/svg/graphics/blackberry \
    $$INCLUDEPATH

olympia-armcc* {
    QMAKE_EXTRA_TARGETS += outputfiles outputfilesclean
    POST_TARGETDEPS += linkolympia
    outputfiles.depends = ${OBJECTS} outputfilesclean
    outputfiles.commands = $(foreach object, $(OBJECTS), $(shell echo $(object) >> linkfiles))

    outputfilesclean.depends = 
    outputfilesclean.commands = $(DEL_FILE) linkfiles

    QMAKE_EXTRA_TARGETS += linkolympia
    linkolympia.depends = outputfiles

    LIBOUTPUTDIR = ../lib

    CONFIG(release, debug|release) {
        linkolympia.commands = ($$QMAKE_CHK_DIR_EXISTS $$LIBOUTPUTDIR || $$QMAKE_MKDIR $$LIBOUTPUTDIR) && \
                           armlink --partial --output=$$LIBOUTPUTDIR/WebKit.lib $$LIBOUTPUTDIR/jscore.lib --via linkfiles
    } else {
        linkolympia.commands = ($$QMAKE_CHK_DIR_EXISTS $$LIBOUTPUTDIR || $$QMAKE_MKDIR $$LIBOUTPUTDIR) && \
                           armar --create $$LIBOUTPUTDIR/WebKit.lib $$LIBOUTPUTDIR/jscore.lib --via linkfiles
    }

}

win32-msvc-fledge {
    QMAKE_EXTRA_TARGETS += outputfiles outputfilesclean
    POST_TARGETDEPS += linkolympia

    outputfiles.depends = ${OBJECTS} outputfilesclean
    outputfiles.commands = $(foreach object, $(OBJECTS), $(shell echo $(object) >> linkfiles)) \
                           (grep \"JS\|WebDOM\" linkfiles > linkfiles.part1) && \
                           (grep -v \"JS\|WebDOM\" linkfiles > linkfiles.part2)

    outputfilesclean.depends = 
    outputfilesclean.commands = $(DEL_FILE) linkfiles linkfiles.part1 linkfiles.part2

    QMAKE_EXTRA_TARGETS += linkolympia
    linkolympia.depends = outputfiles

    LIBOUTPUTDIR = ../lib

    linkolympia.commands = ($$QMAKE_CHK_DIR_EXISTS $$LIBOUTPUTDIR || $$QMAKE_MKDIR $$LIBOUTPUTDIR) && \
                           (lib.exe -NOLOGO -OUT:$$LIBOUTPUTDIR/WebKit.part1.lib $$LIBOUTPUTDIR/jscore.lib @linkfiles.part1) && \
                           (lib.exe -NOLOGO -OUT:$$LIBOUTPUTDIR/WebKit.part2.lib @linkfiles.part2)
}

DASHBOARDSUPPORTCSSPROPERTIES = $$PWD/css/DashboardSupportCSSPropertyNames.in


contains(DEFINES, ENABLE_SVG=1) {
    EXTRACSSPROPERTIES += $$PWD/css/SVGCSSPropertyNames.in
    EXTRACSSVALUES += $$PWD/css/SVGCSSValueKeywords.in
}

contains(DEFINES, ENABLE_WCSS=1) {
    EXTRACSSPROPERTIES += $$PWD/css/WCSSPropertyNames.in
    EXTRACSSVALUES += $$PWD/css/WCSSValueKeywords.in
}

SOURCES += \
    accessibility/AccessibilityImageMapLink.cpp \
    accessibility/AccessibilityMediaControls.cpp \    
    accessibility/AccessibilityMenuList.cpp \
    accessibility/AccessibilityMenuListOption.cpp \
    accessibility/AccessibilityMenuListPopup.cpp \
    accessibility/AccessibilityObject.cpp \    
    accessibility/AccessibilityList.cpp \    
    accessibility/AccessibilityListBox.cpp \    
    accessibility/AccessibilityListBoxOption.cpp \    
    accessibility/AccessibilityProgressIndicator.cpp \    
    accessibility/AccessibilityRenderObject.cpp \    
    accessibility/AccessibilityScrollbar.cpp \
    accessibility/AccessibilitySlider.cpp \    
    accessibility/AccessibilityARIAGrid.cpp \    
    accessibility/AccessibilityARIAGridCell.cpp \    
    accessibility/AccessibilityARIAGridRow.cpp \    
    accessibility/AccessibilityTable.cpp \    
    accessibility/AccessibilityTableCell.cpp \    
    accessibility/AccessibilityTableColumn.cpp \    
    accessibility/AccessibilityTableHeaderContainer.cpp \    
    accessibility/AccessibilityTableRow.cpp \    
    accessibility/AXObjectCache.cpp \
    bindings/js/GCController.cpp \
    bindings/js/DOMObjectHashTableMap.cpp \
    bindings/js/DOMWrapperWorld.cpp \
    bindings/js/JSCallbackData.cpp \
    bindings/js/JSAttrCustom.cpp \
    bindings/js/JSCDATASectionCustom.cpp \
    bindings/js/JSCanvasRenderingContextCustom.cpp \
    bindings/js/JSCanvasRenderingContext2DCustom.cpp \
    bindings/js/JSClipboardCustom.cpp \
    bindings/js/JSConsoleCustom.cpp \
    bindings/js/JSCSSRuleCustom.cpp \
    bindings/js/JSCSSRuleListCustom.cpp \
    bindings/js/JSCSSStyleDeclarationCustom.cpp \
    bindings/js/JSCSSValueCustom.cpp \
    bindings/js/JSCoordinatesCustom.cpp \
    bindings/js/JSCustomPositionCallback.cpp \
    bindings/js/JSCustomPositionErrorCallback.cpp \
    bindings/js/JSCustomVoidCallback.cpp \
    bindings/js/JSCustomXPathNSResolver.cpp \
    bindings/js/JSDataGridColumnListCustom.cpp \
    bindings/js/JSDataGridDataSource.cpp \
    bindings/js/JSDebugWrapperSet.cpp \
    bindings/js/JSDesktopNotificationsCustom.cpp \
    bindings/js/JSDocumentCustom.cpp \
    bindings/js/JSDOMFormDataCustom.cpp \
    bindings/js/JSDOMGlobalObject.cpp \
    bindings/js/JSDOMWindowBase.cpp \
    bindings/js/JSDOMWindowCustom.cpp \
    bindings/js/JSDOMWindowShell.cpp \
    bindings/js/JSDOMWrapper.cpp \
    bindings/js/JSElementCustom.cpp \
    bindings/js/JSEventCustom.cpp \
    bindings/js/JSEventSourceConstructor.cpp \
    bindings/js/JSEventTarget.cpp \
    bindings/js/JSExceptionBase.cpp \
    bindings/js/JSGeolocationCustom.cpp \
    bindings/js/JSHistoryCustom.cpp \
    bindings/js/JSHTMLAppletElementCustom.cpp \
    bindings/js/JSHTMLCanvasElementCustom.cpp \
    bindings/js/JSHTMLAllCollectionCustom.cpp \
    bindings/js/JSHTMLCollectionCustom.cpp \
    bindings/js/JSHTMLDataGridElementCustom.cpp \
    bindings/js/JSHTMLDocumentCustom.cpp \
    bindings/js/JSHTMLElementCustom.cpp \
    bindings/js/JSHTMLEmbedElementCustom.cpp \
    bindings/js/JSHTMLFormElementCustom.cpp \
    bindings/js/JSHTMLFrameElementCustom.cpp \
    bindings/js/JSHTMLFrameSetElementCustom.cpp \
    bindings/js/JSHTMLIFrameElementCustom.cpp \
    bindings/js/JSHTMLInputElementCustom.cpp \
    bindings/js/JSHTMLObjectElementCustom.cpp \
    bindings/js/JSHTMLOptionsCollectionCustom.cpp \
    bindings/js/JSHTMLSelectElementCustom.cpp \
    bindings/js/JSImageConstructor.cpp \
    bindings/js/JSImageDataCustom.cpp \
    bindings/js/JSInjectedScriptHostCustom.cpp \
    bindings/js/JSInspectorFrontendHostCustom.cpp \
    bindings/js/JSLocationCustom.cpp \
    bindings/js/JSNamedNodeMapCustom.cpp \
    bindings/js/JSNavigatorCustom.cpp  \
    bindings/js/JSNodeCustom.cpp \
    bindings/js/JSNodeFilterCondition.cpp \
    bindings/js/JSNodeFilterCustom.cpp \
    bindings/js/JSNodeIteratorCustom.cpp \
    bindings/js/JSNodeListCustom.cpp \
    bindings/js/JSOptionConstructor.cpp \
    bindings/js/JSScriptProfileNodeCustom.cpp \
    bindings/js/JSStyleSheetCustom.cpp \
    bindings/js/JSStyleSheetListCustom.cpp \
    bindings/js/JSTextCustom.cpp \
    bindings/js/JSTreeWalkerCustom.cpp \
    bindings/js/JSWebKitCSSMatrixConstructor.cpp \
    bindings/js/JSWebKitPointConstructor.cpp \
    bindings/js/JSXMLHttpRequestConstructor.cpp \
    bindings/js/JSXMLHttpRequestCustom.cpp \
    bindings/js/JSXMLHttpRequestUploadCustom.cpp \
    bindings/js/JSPluginCustom.cpp \
    bindings/js/JSPluginArrayCustom.cpp \
    bindings/js/JSMessageChannelConstructor.cpp \
    bindings/js/JSMessageChannelCustom.cpp \
    bindings/js/JSMessageEventCustom.cpp \
    bindings/js/JSMessagePortCustom.cpp \
    bindings/js/JSMessagePortCustom.h \
    bindings/js/JSMimeTypeArrayCustom.cpp \
    bindings/js/JSDOMBinding.cpp \
    bindings/js/JSEventListener.cpp \
    bindings/js/JSLazyEventListener.cpp \
    bindings/js/JSMainThreadExecState.cpp \
    bindings/js/JSPluginElementFunctions.cpp \
    bindings/js/JSPopStateEventCustom.cpp \
    bindings/js/JSWorkerContextErrorHandler.cpp \
    bindings/js/ScriptArray.cpp \
    bindings/js/ScriptCachedFrameData.cpp \
    bindings/js/ScriptCallFrame.cpp \
    bindings/js/ScriptCallStack.cpp \
    bindings/js/ScriptController.cpp \
    bindings/js/ScriptDebugServer.cpp \
    bindings/js/ScriptEventListener.cpp \
    bindings/js/ScriptFunctionCall.cpp \
    bindings/js/ScriptGCEvent.cpp \
    bindings/js/ScriptObject.cpp \
    bindings/js/ScriptState.cpp \
    bindings/js/ScriptValue.cpp \
    bindings/js/ScheduledAction.cpp \
    bindings/js/SerializedScriptValue.cpp \
    bindings/ScriptControllerBase.cpp \
    bridge/IdentifierRep.cpp \
    bridge/NP_jsobject.cpp \
    bridge/npruntime.cpp \
    bridge/runtime_array.cpp \
    bridge/runtime_method.cpp \
    bridge/runtime_object.cpp \
    bridge/runtime_root.cpp \
    bridge/c/CRuntimeObject.cpp \
    bridge/c/c_class.cpp \
    bridge/c/c_instance.cpp \
    bridge/c/c_runtime.cpp \
    bridge/c/c_utility.cpp \
    bridge/jsc/BridgeJSC.cpp \
    css/CSSBorderImageValue.cpp \
    css/CSSCanvasValue.cpp \
    css/CSSCharsetRule.cpp \
    css/CSSComputedStyleDeclaration.cpp \
    css/CSSCursorImageValue.cpp \
    css/CSSFontFace.cpp \
    css/CSSFontFaceRule.cpp \
    css/CSSFontFaceSrcValue.cpp \
    css/CSSFontSelector.cpp \
    css/CSSFontFaceSource.cpp \
    css/CSSFunctionValue.cpp \
    css/CSSGradientValue.cpp \
    css/CSSHelper.cpp \
    css/CSSImageValue.cpp \
    css/CSSImageGeneratorValue.cpp \
    css/CSSImportRule.cpp \
    css/CSSInheritedValue.cpp \
    css/CSSInitialValue.cpp \
    css/CSSMediaRule.cpp \
    css/CSSMutableStyleDeclaration.cpp \
    css/CSSPageRule.cpp \
    css/CSSParser.cpp \
    css/CSSParserValues.cpp \
    css/CSSPrimitiveValue.cpp \
    css/CSSProperty.cpp \
    css/CSSPropertyLonghand.cpp \
    css/CSSReflectValue.cpp \
    css/CSSRule.cpp \
    css/CSSRuleList.cpp \
    css/CSSSelector.cpp \
    css/CSSSelectorList.cpp \
    css/CSSSegmentedFontFace.cpp \
    css/CSSStyleDeclaration.cpp \
    css/CSSStyleRule.cpp \
    css/CSSStyleSelector.cpp \
    css/CSSStyleSheet.cpp \
    css/CSSTimingFunctionValue.cpp \
    css/CSSUnicodeRangeValue.cpp \
    css/CSSValueList.cpp \
    css/CSSVariableDependentValue.cpp \
    css/CSSVariablesDeclaration.cpp \
    css/CSSVariablesRule.cpp \
    css/FontFamilyValue.cpp \
    css/FontValue.cpp \
    css/MediaFeatureNames.cpp \
    css/MediaList.cpp \
    css/MediaQuery.cpp \
    css/MediaQueryEvaluator.cpp \
    css/MediaQueryExp.cpp \
    css/RGBColor.cpp \
    css/ShadowValue.cpp \
    css/StyleBase.cpp \
    css/StyleList.cpp \
    css/StyleMedia.cpp \
    css/StyleSheet.cpp \
    css/StyleSheetList.cpp \
    css/WebKitCSSKeyframeRule.cpp \
    css/WebKitCSSKeyframesRule.cpp \
    css/WebKitCSSMatrix.cpp \
    css/WebKitCSSTransformValue.cpp \
    dom/ActiveDOMObject.cpp \
    dom/Attr.cpp \
    dom/Attribute.cpp \
    dom/BeforeTextInsertedEvent.cpp \
    dom/BeforeUnloadEvent.cpp \
    dom/CDATASection.cpp \
    dom/CanvasSurface.cpp \
    dom/CharacterData.cpp \
    dom/CheckedRadioButtons.cpp \
    dom/ChildNodeList.cpp \
    dom/ClassNodeList.cpp \
    dom/ClientRect.cpp \
    dom/ClientRectList.cpp \
    dom/Clipboard.cpp \
    dom/ClipboardEvent.cpp \
    dom/Comment.cpp \
    dom/CompositionEvent.cpp \
    dom/ContainerNode.cpp \
    dom/CSSMappedAttributeDeclaration.cpp \
    dom/CustomEvent.cpp \
    dom/DeviceOrientation.cpp \
    dom/DeviceOrientationEvent.cpp \
    dom/Document.cpp \
    dom/DocumentFragment.cpp \
    dom/DocumentType.cpp \
    dom/DOMImplementation.cpp \
    dom/DOMStringList.cpp \
    dom/DynamicNodeList.cpp \
    dom/EditingText.cpp \
    dom/Element.cpp \
    dom/Entity.cpp \
    dom/EntityReference.cpp \
    dom/ErrorEvent.cpp \
    dom/Event.cpp \
    dom/EventNames.cpp \
    dom/EventTarget.cpp \
    dom/ExceptionBase.cpp \
    dom/ExceptionCode.cpp \
    dom/InputElement.cpp \
    dom/KeyboardEvent.cpp \
    dom/MessageChannel.cpp \
    dom/MessageEvent.cpp \
    dom/MessagePort.cpp \
    dom/MessagePortChannel.cpp \
    dom/MouseEvent.cpp \
    dom/MouseRelatedEvent.cpp \
    dom/MutationEvent.cpp \
    dom/NamedNodeMap.cpp \
    dom/NameNodeList.cpp \
    dom/Node.cpp \
    dom/NodeFilterCondition.cpp \
    dom/NodeFilter.cpp \
    dom/NodeIterator.cpp \
    dom/Notation.cpp \
    dom/OptionGroupElement.cpp \
    dom/OptionElement.cpp \
    dom/OverflowEvent.cpp \
    dom/PageTransitionEvent.cpp \
    dom/PopStateEvent.cpp \
    dom/Position.cpp \
    dom/PositionIterator.cpp \
    dom/ProcessingInstruction.cpp \
    dom/ProgressEvent.cpp \
    dom/QualifiedName.cpp \
    dom/Range.cpp \
    dom/RegisteredEventListener.cpp \
    dom/ScriptElement.cpp \
    dom/ScriptExecutionContext.cpp \
    dom/SelectElement.cpp \
    dom/SelectorNodeList.cpp \
    dom/SpaceSplitString.cpp \
    dom/StaticNodeList.cpp \
    dom/StyledElement.cpp \
    dom/StyleElement.cpp \
    dom/TagNodeList.cpp \
    dom/Text.cpp \
    dom/TextEvent.cpp \
    dom/Touch.cpp \
    dom/TouchEvent.cpp \
    dom/TouchList.cpp \
    dom/Traversal.cpp \
    dom/TreeWalker.cpp \
    dom/UIEvent.cpp \
    dom/UIEventWithKeyState.cpp \
    dom/UserGestureIndicator.cpp \
    dom/WebKitAnimationEvent.cpp \
    dom/WebKitTransitionEvent.cpp \
    dom/WheelEvent.cpp \
    dom/XMLTokenizer.cpp \
    dom/XMLTokenizerScope.cpp \
    dom/default/PlatformMessagePortChannel.cpp \
    dom/XMLTokenizerLibxml2.cpp \
    editing/AppendNodeCommand.cpp \
    editing/ApplyStyleCommand.cpp \
    editing/BreakBlockquoteCommand.cpp \
    editing/CompositeEditCommand.cpp \
    editing/CreateLinkCommand.cpp \
    editing/DeleteButtonController.cpp \
    editing/DeleteButton.cpp \
    editing/DeleteFromTextNodeCommand.cpp \
    editing/DeleteSelectionCommand.cpp \
    editing/EditCommand.cpp \
    editing/Editor.cpp \
    editing/EditorCommand.cpp \
    editing/FormatBlockCommand.cpp \
    editing/htmlediting.cpp \
    editing/HTMLInterchange.cpp \
    editing/IndentOutdentCommand.cpp \
    editing/InsertIntoTextNodeCommand.cpp \
    editing/InsertLineBreakCommand.cpp \
    editing/InsertListCommand.cpp \
    editing/InsertNodeBeforeCommand.cpp \
    editing/InsertParagraphSeparatorCommand.cpp \
    editing/InsertTextCommand.cpp \
    editing/JoinTextNodesCommand.cpp \
    editing/markup.cpp \
    editing/MergeIdenticalElementsCommand.cpp \
    editing/ModifySelectionListLevel.cpp \
    editing/MoveSelectionCommand.cpp \
    editing/RemoveCSSPropertyCommand.cpp \
    editing/RemoveFormatCommand.cpp \
    editing/RemoveNodeCommand.cpp \
    editing/RemoveNodePreservingChildrenCommand.cpp \
    editing/ReplaceNodeWithSpanCommand.cpp \
    editing/ReplaceSelectionCommand.cpp \
    editing/SelectionController.cpp \
    editing/SetNodeAttributeCommand.cpp \
    editing/SmartReplaceICU.cpp \
    editing/SplitElementCommand.cpp \
    editing/SplitTextNodeCommand.cpp \
    editing/SplitTextNodeContainingElementCommand.cpp \
    editing/TextIterator.cpp \
    editing/TypingCommand.cpp \
    editing/UnlinkCommand.cpp \
    editing/VisiblePosition.cpp \
    editing/VisibleSelection.cpp \
    editing/visible_units.cpp \
    editing/WrapContentsInDummySpanCommand.cpp \
    history/BackForwardList.cpp \
    history/CachedFrame.cpp \
    history/CachedPage.cpp \
    history/HistoryItem.cpp \
    history/PageCache.cpp \
    html/Blob.cpp \
    html/canvas/CanvasGradient.cpp \
    html/canvas/CanvasPattern.cpp \
    html/canvas/CanvasPixelArray.cpp \
    html/canvas/CanvasRenderingContext.cpp \
    html/canvas/CanvasRenderingContext2D.cpp \
    html/canvas/CanvasStyle.cpp \
    html/CollectionCache.cpp \
    html/DataGridColumn.cpp \
    html/DataGridColumnList.cpp \
    html/DateComponents.cpp \
    html/DOMDataGridDataSource.cpp \
    html/DOMFormData.cpp \
    html/File.cpp \
    html/FileList.cpp \
    html/FileReader.cpp \
    html/FileStream.cpp \
    html/FileStreamProxy.cpp \
    html/FileThread.cpp \
    html/FormDataList.cpp \
    html/HTML5Lexer.cpp \
    html/HTML5Tokenizer.cpp \
    html/HTML5TreeBuilder.cpp \
    html/HTMLAllCollection.cpp \
    html/HTMLAnchorElement.cpp \
    html/HTMLAppletElement.cpp \
    html/HTMLAreaElement.cpp \
    html/HTMLBaseElement.cpp \
    html/HTMLBaseFontElement.cpp \
    html/HTMLBlockquoteElement.cpp \
    html/HTMLBodyElement.cpp \
    html/HTMLBRElement.cpp \
    html/HTMLButtonElement.cpp \
    html/HTMLCanvasElement.cpp \
    html/HTMLCollection.cpp \
    html/HTMLDataGridElement.cpp \
    html/HTMLDataGridCellElement.cpp \
    html/HTMLDataGridColElement.cpp \
    html/HTMLDataGridRowElement.cpp \
    html/HTMLDataListElement.cpp \
    html/HTMLDirectoryElement.cpp \
    html/HTMLDivElement.cpp \
    html/HTMLDListElement.cpp \
    html/HTMLDocument.cpp \
    html/HTMLElement.cpp \
    html/HTMLEmbedElement.cpp \
    html/HTMLFieldSetElement.cpp \
    html/HTMLFontElement.cpp \
    html/HTMLFormCollection.cpp \
    html/HTMLFormElement.cpp \
    html/HTMLFrameElementBase.cpp \
    html/HTMLFrameElement.cpp \
    html/HTMLFrameOwnerElement.cpp \
    html/HTMLFrameSetElement.cpp \
    html/HTMLFormControlElement.cpp \
    html/HTMLHeadElement.cpp \
    html/HTMLHeadingElement.cpp \
    html/HTMLHRElement.cpp \
    html/HTMLHtmlElement.cpp \
    html/HTMLIFrameElement.cpp \
    html/HTMLImageElement.cpp \
    html/HTMLImageLoader.cpp \
    html/HTMLInputElement.cpp \
    html/HTMLIsIndexElement.cpp \
    html/HTMLKeygenElement.cpp \
    html/HTMLLabelElement.cpp \
    html/HTMLLegendElement.cpp \
    html/HTMLLIElement.cpp \
    html/HTMLLinkElement.cpp \
    html/HTMLMapElement.cpp \
    html/HTMLMarqueeElement.cpp \
    html/HTMLMenuElement.cpp \
    html/HTMLMetaElement.cpp \
    html/HTMLMeterElement.cpp \
    html/HTMLModElement.cpp \
    html/HTMLNameCollection.cpp \
    html/HTMLObjectElement.cpp \
    html/HTMLOListElement.cpp \
    html/HTMLOptGroupElement.cpp \
    html/HTMLOptionElement.cpp \
    html/HTMLOptionsCollection.cpp \
    html/HTMLParagraphElement.cpp \
    html/HTMLParamElement.cpp \
    html/HTMLParser.cpp \
    html/HTMLParserErrorCodes.cpp \
    html/HTMLPlugInElement.cpp \
    html/HTMLPlugInImageElement.cpp \
    html/HTMLPreElement.cpp \
    html/HTMLProgressElement.cpp \
    html/HTMLQuoteElement.cpp \
    html/HTMLScriptElement.cpp \
    html/HTMLSelectElement.cpp \
    html/HTMLStyleElement.cpp \
    html/HTMLTableCaptionElement.cpp \
    html/HTMLTableCellElement.cpp \
    html/HTMLTableColElement.cpp \
    html/HTMLTableElement.cpp \
    html/HTMLTablePartElement.cpp \
    html/HTMLTableRowElement.cpp \
    html/HTMLTableRowsCollection.cpp \
    html/HTMLTableSectionElement.cpp \
    html/HTMLTextAreaElement.cpp \
    html/HTMLTitleElement.cpp \
    html/HTMLTokenizer.cpp \
    html/HTMLUListElement.cpp \
    html/HTMLViewSourceDocument.cpp \
    html/ImageData.cpp \
    html/LabelsNodeList.cpp \
    html/PreloadScanner.cpp \
    html/StepRange.cpp \
    html/ValidityState.cpp \
    inspector/ConsoleMessage.cpp \
    inspector/InjectedScript.cpp \
    inspector/InjectedScriptHost.cpp \
    inspector/InspectorBackend.cpp \
    inspector/InspectorCSSStore.cpp \
    inspector/InspectorController.cpp \
    inspector/InspectorDatabaseResource.cpp \
    inspector/InspectorDOMAgent.cpp \
    inspector/InspectorDOMStorageResource.cpp \
    inspector/InspectorFrontend.cpp \
    inspector/InspectorFrontendClientLocal.cpp \
    inspector/InspectorFrontendHost.cpp \
    inspector/InspectorResource.cpp \
    inspector/InspectorTimelineAgent.cpp \
    inspector/TimelineRecordFactory.cpp \
    loader/archive/ArchiveFactory.cpp \
    loader/archive/ArchiveResource.cpp \
    loader/archive/ArchiveResourceCollection.cpp \
#    loader/UserStyleSheetLoader.cpp \
    loader/Cache.cpp \
    loader/CachedCSSStyleSheet.cpp \
    loader/CachedFont.cpp \
    loader/CachedImage.cpp \
    loader/CachedResourceClientWalker.cpp \
    loader/CachedResourceHandle.cpp \
    loader/CachedResource.cpp \
    loader/CachedScript.cpp \
    loader/CachedXSLStyleSheet.cpp \
    loader/CrossOriginAccessControl.cpp \
    loader/CrossOriginPreflightResultCache.cpp \
    loader/DocLoader.cpp \
    loader/DocumentLoader.cpp \
    loader/DocumentThreadableLoader.cpp \
    loader/DocumentWriter.cpp \
    loader/FormState.cpp \
    loader/FrameLoader.cpp \
    loader/HistoryController.cpp \
    loader/FTPDirectoryDocument.cpp \
    loader/FTPDirectoryParser.cpp \
    loader/icon/IconLoader.cpp \
    loader/ImageDocument.cpp \
    loader/ImageLoader.cpp \
    loader/loader.cpp \
    loader/MainResourceLoader.cpp \
    loader/MediaDocument.cpp \
    loader/NavigationAction.cpp \
    loader/NetscapePlugInStreamLoader.cpp \
    loader/PlaceholderDocument.cpp \
    loader/PluginDocument.cpp \
    loader/PolicyCallback.cpp \
    loader/PolicyChecker.cpp \
    loader/ProgressTracker.cpp \
    loader/RedirectScheduler.cpp \
    loader/Request.cpp \
    loader/ResourceLoader.cpp \
    loader/ResourceLoadNotifier.cpp \
    loader/SinkDocument.cpp \
    loader/SubresourceLoader.cpp \
    loader/TextDocument.cpp \
    loader/TextResourceDecoder.cpp \
    loader/ThreadableLoader.cpp \
    notifications/Notification.cpp \
    notifications/NotificationCenter.cpp \
    page/animation/AnimationBase.cpp \
    page/animation/AnimationController.cpp \
    page/animation/CompositeAnimation.cpp \
    page/animation/ImplicitAnimation.cpp \
    page/animation/KeyframeAnimation.cpp \
    page/BarInfo.cpp \
    page/Chrome.cpp \
    page/Console.cpp \
    page/ContextMenuController.cpp \
    page/DOMSelection.cpp \
    page/DOMTimer.cpp \
    page/DOMWindow.cpp \
    page/Navigator.cpp \
    page/NavigatorBase.cpp \
    page/DragController.cpp \
    page/EventHandler.cpp \
    page/EventSource.cpp \
    page/FocusController.cpp \
    page/Frame.cpp \
    page/FrameTree.cpp \
    page/FrameView.cpp \
    page/Geolocation.cpp \
    page/GeolocationController.cpp \
    page/GeolocationPositionCache.cpp \
    page/History.cpp \
    page/Location.cpp \
    page/MouseEventWithHitTestResults.cpp \
    page/OriginAccessEntry.cpp \
    page/Page.cpp \
    page/PageGroup.cpp \
    page/PageGroupLoadDeferrer.cpp \
    page/PluginHalter.cpp \
    page/PrintContext.cpp \
    page/SecurityOrigin.cpp \
    page/Screen.cpp \
    page/Settings.cpp \
    page/SpatialNavigation.cpp \
    page/SuspendableTimer.cpp \
    page/UserContentURLPattern.cpp \
    page/WindowFeatures.cpp \
    page/XSSAuditor.cpp \
    plugins/PluginData.cpp \
    plugins/PluginArray.cpp \
    plugins/Plugin.cpp \
    plugins/PluginMainThreadScheduler.cpp \
    plugins/MimeType.cpp \
    plugins/MimeTypeArray.cpp \
    platform/animation/Animation.cpp \
    platform/animation/AnimationList.cpp \
    platform/Arena.cpp \
    platform/text/Base64.cpp \
    platform/text/BidiContext.cpp \
    platform/ContentType.cpp \
    platform/ContextMenu.cpp \
    platform/CrossThreadCopier.cpp \
    platform/DeprecatedPtrListImpl.cpp \
    platform/DragData.cpp \
    platform/DragImage.cpp \
    platform/FileChooser.cpp \
    platform/GeolocationService.cpp \
    platform/graphics/filters/FEGaussianBlur.cpp \
    platform/graphics/FontDescription.cpp \
    platform/graphics/FontFallbackList.cpp \
    platform/graphics/FontFamily.cpp \
    platform/graphics/BitmapImage.cpp \
    platform/graphics/Color.cpp \
    platform/graphics/FloatPoint3D.cpp \
    platform/graphics/FloatPoint.cpp \
    platform/graphics/FloatQuad.cpp \
    platform/graphics/FloatRect.cpp \
    platform/graphics/FloatSize.cpp \
    platform/graphics/FontData.cpp \
    platform/graphics/Font.cpp \
    platform/graphics/FontCache.cpp \
    platform/graphics/GeneratedImage.cpp \
    platform/graphics/Gradient.cpp \
    platform/graphics/GraphicsContext.cpp \
    platform/graphics/GraphicsTypes.cpp \
    platform/graphics/Image.cpp \
    platform/graphics/ImageBuffer.cpp \
    platform/graphics/ImageSource.cpp \
    platform/graphics/IntRect.cpp \
    platform/graphics/IntRectRegion.cpp \
    platform/graphics/Path.cpp \
    platform/graphics/PathTraversalState.cpp \
    platform/graphics/Pattern.cpp \
    platform/graphics/Pen.cpp \
    platform/graphics/SegmentedFontData.cpp \
    platform/graphics/SimpleFontData.cpp \
    platform/graphics/TiledBackingStore.cpp \
    platform/graphics/StringTruncator.cpp \
    platform/graphics/transforms/AffineTransform.cpp \
    platform/graphics/transforms/TransformationMatrix.cpp \
    platform/graphics/transforms/MatrixTransformOperation.cpp \
    platform/graphics/transforms/Matrix3DTransformOperation.cpp \
    platform/graphics/transforms/PerspectiveTransformOperation.cpp \
    platform/graphics/transforms/RotateTransformOperation.cpp \
    platform/graphics/transforms/ScaleTransformOperation.cpp \
    platform/graphics/transforms/SkewTransformOperation.cpp \
    platform/graphics/transforms/TransformOperations.cpp \
    platform/graphics/transforms/TranslateTransformOperation.cpp \
    platform/image-decoders/ImageDecoder.cpp \
    platform/image-decoders/bmp/BMPImageDecoder.cpp \
    platform/image-decoders/bmp/BMPImageReader.cpp \
    platform/image-decoders/gif/GIFImageDecoder.cpp \
    platform/image-decoders/gif/GIFImageReader.cpp \
#    platform/image-decoders/ico/ICOImageDecoder.cpp \
    platform/image-decoders/jpeg/JPEGImageDecoder.cpp \
    platform/image-decoders/png/PNGImageDecoder.cpp \
#    platform/image-decoders/xbm/XBMImageDecoder.cpp \
    platform/image-encoders/JPEGImageEncoder.cpp \
    platform/image-encoders/PNGImageEncoder.cpp \
    platform/KURL.cpp \
    platform/Length.cpp \
    platform/LinkHash.cpp \
    platform/Logging.cpp \
    platform/MIMETypeRegistry.cpp \
    platform/mock/GeolocationServiceMock.cpp \
    platform/network/AuthenticationChallengeBase.cpp \
    platform/network/Credential.cpp \
    platform/network/FormData.cpp \
    platform/network/FormDataBuilder.cpp \
    platform/network/HTTPHeaderMap.cpp \
    platform/network/HTTPParsers.cpp \
    platform/network/NetworkStateNotifier.cpp \
    platform/network/ProtectionSpace.cpp \
    platform/network/ResourceErrorBase.cpp \
    platform/network/ResourceHandle.cpp \
    platform/network/ResourceRequestBase.cpp \
    platform/network/ResourceResponseBase.cpp \
    platform/text/RegularExpression.cpp \
    platform/Scrollbar.cpp \
    platform/ScrollbarThemeComposite.cpp \
    platform/ScrollView.cpp \
    platform/text/SegmentedString.cpp \
    platform/SharedBuffer.cpp \
    platform/StringPattern.cpp \
    platform/text/String.cpp \
    platform/text/StringBuilder.cpp \
    platform/text/TextBoundaries.cpp \
    platform/text/TextCodec.cpp \
    platform/text/TextCodecLatin1.cpp \
    platform/text/TextCodecUserDefined.cpp \
    platform/text/TextCodecUTF16.cpp \
    platform/text/TextEncoding.cpp \
    platform/text/TextEncodingDetectorNone.cpp \
    platform/text/TextEncodingRegistry.cpp \
    platform/text/TextStream.cpp \
    platform/ThreadGlobalData.cpp \
    platform/ThreadTimers.cpp \
    platform/Timer.cpp \
    platform/text/UnicodeRange.cpp \
    platform/text/transcoder/FontTranscoder.cpp \
    platform/UUID.cpp \
    platform/Widget.cpp \
    plugins/PluginDatabase.cpp \
    plugins/PluginDebug.cpp \
    plugins/PluginPackage.cpp \
    plugins/PluginStream.cpp \
    plugins/PluginView.cpp \
    rendering/AutoTableLayout.cpp \
    rendering/break_lines.cpp \
    rendering/BidiRun.cpp \
    rendering/CounterNode.cpp \
    rendering/EllipsisBox.cpp \
    rendering/FixedTableLayout.cpp \
    rendering/HitTestResult.cpp \
    rendering/InlineBox.cpp \
    rendering/InlineFlowBox.cpp \
    rendering/InlineTextBox.cpp \
    rendering/LayoutState.cpp \
    rendering/RenderApplet.cpp \
    rendering/RenderArena.cpp \
    rendering/RenderBlock.cpp \
    rendering/RenderBlockLineLayout.cpp \
    rendering/RenderBox.cpp \
    rendering/RenderBoxModelObject.cpp \
    rendering/RenderBR.cpp \
    rendering/RenderButton.cpp \
    rendering/RenderCounter.cpp \
    rendering/RenderDataGrid.cpp \
    rendering/RenderEmbeddedObject.cpp \
    rendering/RenderFieldset.cpp \
    rendering/RenderFileUploadControl.cpp \
    rendering/RenderFlexibleBox.cpp \
    rendering/RenderFrame.cpp \
    rendering/RenderFrameBase.cpp \
    rendering/RenderFrameSet.cpp \
    rendering/RenderHTMLCanvas.cpp \
    rendering/RenderIFrame.cpp \
    rendering/RenderImage.cpp \
    rendering/RenderImageGeneratedContent.cpp \
    rendering/RenderInline.cpp \
    rendering/RenderLayer.cpp \
    rendering/RenderLineBoxList.cpp \
    rendering/RenderListBox.cpp \
    rendering/RenderListItem.cpp \
    rendering/RenderListMarker.cpp \
    rendering/RenderMarquee.cpp \
    rendering/RenderMenuList.cpp \
    rendering/RenderMeter.cpp \
    rendering/RenderObject.cpp \
    rendering/RenderObjectChildList.cpp \
    rendering/RenderPart.cpp \
    rendering/RenderProgress.cpp \
    rendering/RenderReplaced.cpp \
    rendering/RenderReplica.cpp \
    rendering/RenderRuby.cpp \
    rendering/RenderRubyBase.cpp \
    rendering/RenderRubyRun.cpp \
    rendering/RenderRubyText.cpp \
    rendering/RenderScrollbar.cpp \
    rendering/RenderScrollbarPart.cpp \
    rendering/RenderScrollbarTheme.cpp \
    rendering/RenderSlider.cpp \
    rendering/RenderTable.cpp \
    rendering/RenderTableCell.cpp \
    rendering/RenderTableCol.cpp \
    rendering/RenderTableRow.cpp \
    rendering/RenderTableSection.cpp \
    rendering/RenderText.cpp \
    rendering/RenderTextControl.cpp \
    rendering/RenderTextControlMultiLine.cpp \
    rendering/RenderTextControlSingleLine.cpp \
    rendering/RenderTextFragment.cpp \
    rendering/RenderTheme.cpp \
    rendering/RenderTreeAsText.cpp \
    rendering/RenderView.cpp \
    rendering/RenderWidget.cpp \
    rendering/RenderWordBreak.cpp \
    rendering/RootInlineBox.cpp \
    rendering/SVGRenderTreeAsText.cpp \
    rendering/ScrollBehavior.cpp \
    rendering/TextControlInnerElements.cpp \
    rendering/TransformState.cpp \
    rendering/style/BindingURI.cpp \
    rendering/style/ContentData.cpp \
    rendering/style/CounterDirectives.cpp \
    rendering/style/FillLayer.cpp \
    rendering/style/KeyframeList.cpp \
    rendering/style/NinePieceImage.cpp \
    rendering/style/RenderStyle.cpp \
    rendering/style/ShadowData.cpp \
    rendering/style/StyleBackgroundData.cpp \
    rendering/style/StyleBoxData.cpp \
    rendering/style/StyleCachedImage.cpp \
    rendering/style/StyleFlexibleBoxData.cpp \
    rendering/style/StyleGeneratedImage.cpp \
    rendering/style/StyleInheritedData.cpp \
    rendering/style/StyleMarqueeData.cpp \
    rendering/style/StyleMultiColData.cpp \
    rendering/style/StyleRareInheritedData.cpp \
    rendering/style/StyleRareNonInheritedData.cpp \
    rendering/style/StyleSurroundData.cpp \
    rendering/style/StyleTransformData.cpp \
    rendering/style/StyleVisualData.cpp \
    xml/DOMParser.cpp \
    xml/XMLHttpRequest.cpp \
    xml/XMLHttpRequestProgressEventThrottle.cpp \
    xml/XMLHttpRequestUpload.cpp \
    xml/XMLSerializer.cpp 

HEADERS += \
    accessibility/AccessibilityARIAGridCell.h \
    accessibility/AccessibilityARIAGrid.h \
    accessibility/AccessibilityARIAGridRow.h \
    accessibility/AccessibilityImageMapLink.h \
    accessibility/AccessibilityListBox.h \
    accessibility/AccessibilityListBoxOption.h \
    accessibility/AccessibilityList.h \
    accessibility/AccessibilityMediaControls.h \
    accessibility/AccessibilityObject.h \
    accessibility/AccessibilityProgressIndicator.h \
    accessibility/AccessibilityRenderObject.h \
    accessibility/AccessibilityScrollbar.h \
    accessibility/AccessibilitySlider.h \
    accessibility/AccessibilityTableCell.h \
    accessibility/AccessibilityTableColumn.h \
    accessibility/AccessibilityTable.h \
    accessibility/AccessibilityTableHeaderContainer.h \
    accessibility/AccessibilityTableRow.h \
    accessibility/AXObjectCache.h \
    bindings/js/CachedScriptSourceProvider.h \
    bindings/js/GCController.h \
    bindings/js/DOMObjectHashTableMap.h \
    bindings/js/DOMWrapperWorld.h \
    bindings/js/JSCallbackData.h \
    bindings/js/JSAudioConstructor.h \
    bindings/js/JSCSSStyleDeclarationCustom.h \
    bindings/js/JSCustomPositionCallback.h \
    bindings/js/JSCustomPositionErrorCallback.h \
    bindings/js/JSCustomVoidCallback.h \
    bindings/js/JSCustomXPathNSResolver.h \
    bindings/js/JSDataGridDataSource.h \
    bindings/js/JSDebugWrapperSet.h \
    bindings/js/JSDOMBinding.h \
    bindings/js/JSDOMGlobalObject.h \
    bindings/js/JSDOMWindowBase.h \
    bindings/js/JSDOMWindowBase.h \
    bindings/js/JSDOMWindowCustom.h \
    bindings/js/JSDOMWindowShell.h \
    bindings/js/JSDOMWrapper.h \
    bindings/js/JSEventListener.h \
    bindings/js/JSEventSourceConstructor.h \
    bindings/js/JSEventTarget.h \
    bindings/js/JSHistoryCustom.h \
    bindings/js/JSHTMLAppletElementCustom.h \
    bindings/js/JSHTMLEmbedElementCustom.h \
    bindings/js/JSHTMLInputElementCustom.h \
    bindings/js/JSHTMLObjectElementCustom.h \
    bindings/js/JSHTMLSelectElementCustom.h \
    bindings/js/JSImageConstructor.h \
    bindings/js/JSLazyEventListener.h \
    bindings/js/JSLocationCustom.h \
    bindings/js/JSMessageChannelConstructor.h \
    bindings/js/JSNodeCustom.h \
    bindings/js/JSNodeFilterCondition.h \
    bindings/js/JSOptionConstructor.h \
    bindings/js/JSPluginElementFunctions.h \
    bindings/js/JSSharedWorkerConstructor.h \
    bindings/js/JSStorageCustom.h \
    bindings/js/JSWebKitCSSMatrixConstructor.h \
    bindings/js/JSWebKitPointConstructor.h \
    bindings/js/JSWorkerConstructor.h \
    bindings/js/JSWorkerContextBase.h \
    bindings/js/JSWorkerContextErrorHandler.h \
    bindings/js/JSXMLHttpRequestConstructor.h \
    bindings/js/JSXSLTProcessorConstructor.h \
    bindings/js/JavaScriptCallFrame.h \
    bindings/js/ScheduledAction.h \
    bindings/js/ScriptArray.h \
    bindings/js/ScriptCachedFrameData.h \
    bindings/js/ScriptCallFrame.h \
    bindings/js/ScriptCallStack.h \
    bindings/js/ScriptController.h \
    bindings/js/ScriptEventListener.h \
    bindings/js/ScriptFunctionCall.h \
    bindings/js/ScriptGCEvent.h \
    bindings/js/ScriptObject.h \
    bindings/js/ScriptProfile.h \
    bindings/js/ScriptProfileNode.h \
    bindings/js/ScriptProfiler.h \
    bindings/js/ScriptSourceCode.h \
    bindings/js/ScriptSourceProvider.h \
    bindings/js/ScriptState.h \
    bindings/js/ScriptValue.h \
    bindings/js/ScriptWrappable.h \
    bindings/js/SerializedScriptValue.h \
    bindings/js/StringSourceProvider.h \
    bindings/js/WebCoreJSClientData.h \
    bindings/js/WorkerScriptController.h \
    bridge/Bridge.h \
    bridge/c/CRuntimeObject.h \
    bridge/c/c_class.h \
    bridge/c/c_instance.h \
    bridge/c/c_runtime.h \
    bridge/c/c_utility.h \
    bridge/jsc/BridgeJSC.h \
    bridge/IdentifierRep.h \
    bridge/NP_jsobject.h \
    bridge/npruntime.h \
    bridge/runtime_array.h \
    bridge/runtime_method.h \
    bridge/runtime_object.h \
    bridge/runtime_root.h \
    css/CSSBorderImageValue.h \
    css/CSSCanvasValue.h \
    css/CSSCharsetRule.h \
    css/CSSComputedStyleDeclaration.h \
    css/CSSCursorImageValue.h \
    css/CSSFontFace.h \
    css/CSSFontFaceRule.h \
    css/CSSFontFaceSource.h \
    css/CSSFontFaceSrcValue.h \
    css/CSSFontSelector.h \
    css/CSSFunctionValue.h \
    css/CSSGradientValue.h \
    css/CSSHelper.h \
    css/CSSImageGeneratorValue.h \
    css/CSSImageValue.h \
    css/CSSImportRule.h \
    css/CSSInheritedValue.h \
    css/CSSInitialValue.h \
    css/CSSMediaRule.h \
    css/CSSMutableStyleDeclaration.h \
    css/CSSPageRule.h \
    css/CSSParser.h \
    css/CSSParserValues.h \
    css/CSSPrimitiveValue.h \
    css/CSSProperty.h \
    css/CSSPropertyLonghand.h \
    css/CSSReflectValue.h \
    css/CSSRule.h \
    css/CSSRuleList.h \
    css/CSSSegmentedFontFace.h \
    css/CSSSelector.h \
    css/CSSSelectorList.h \
    css/CSSStyleDeclaration.h \
    css/CSSStyleRule.h \
    css/CSSStyleSelector.h \
    css/CSSStyleSheet.h \
    css/CSSTimingFunctionValue.h \
    css/CSSUnicodeRangeValue.h \
    css/CSSValueList.h \
    css/CSSVariableDependentValue.h \
    css/CSSVariablesDeclaration.h \
    css/CSSVariablesRule.h \
    css/FontFamilyValue.h \
    css/FontValue.h \
    css/MediaFeatureNames.h \
    css/MediaList.h \
    css/MediaQueryEvaluator.h \
    css/MediaQueryExp.h \
    css/MediaQuery.h \
    css/RGBColor.h \
    css/ShadowValue.h \
    css/StyleBase.h \
    css/StyleList.h \
    css/StyleMedia.h \
    css/StyleSheet.h \
    css/StyleSheetList.h \
    css/WebKitCSSKeyframeRule.h \
    css/WebKitCSSKeyframesRule.h \
    css/WebKitCSSMatrix.h \
    css/WebKitCSSTransformValue.h \
    dom/ActiveDOMObject.h \
    dom/Attr.h \
    dom/Attribute.h \
    dom/BeforeTextInsertedEvent.h \
    dom/BeforeUnloadEvent.h \
    dom/CDATASection.h \
    dom/CharacterData.h \
    dom/CheckedRadioButtons.h \
    dom/ChildNodeList.h \
    dom/ClassNodeList.h \
    dom/ClientRect.h \
    dom/ClientRectList.h \
    dom/ClipboardEvent.h \
    dom/Clipboard.h \
    dom/Comment.h \
    dom/ContainerNode.h \
    dom/CSSMappedAttributeDeclaration.h \
    dom/CustomEvent.h \
    dom/default/PlatformMessagePortChannel.h \
    dom/DeviceOrientation.h \
    dom/DeviceOrientationClient.h \
    dom/DeviceOrientationEvent.h \
    dom/DocumentFragment.h \
    dom/Document.h \
    dom/DocumentType.h \
    dom/DOMImplementation.h \
    dom/DOMStringList.h \
    dom/DynamicNodeList.h \
    dom/EditingText.h \
    dom/Element.h \
    dom/Entity.h \
    dom/EntityReference.h \
    dom/Event.h \
    dom/EventNames.h \
    dom/EventTarget.h \
    dom/ExceptionBase.h \
    dom/ExceptionCode.h \
    dom/InputElement.h \
    dom/KeyboardEvent.h \
    dom/MessageChannel.h \
    dom/MessageEvent.h \
    dom/MessagePortChannel.h \
    dom/MessagePort.h \
    dom/MouseEvent.h \
    dom/MouseRelatedEvent.h \
    dom/MutationEvent.h \
    dom/NamedNodeMap.h \
    dom/NameNodeList.h \
    dom/NodeFilterCondition.h \
    dom/NodeFilter.h \
    dom/Node.h \
    dom/NodeIterator.h \
    dom/Notation.h \
    dom/OptionElement.h \
    dom/OptionGroupElement.h \
    dom/OverflowEvent.h \
    dom/PageTransitionEvent.h \
    dom/Position.h \
    dom/PositionIterator.h \
    dom/ProcessingInstruction.h \
    dom/ProgressEvent.h \
    dom/QualifiedName.h \
    dom/Range.h \
    dom/RegisteredEventListener.h \
    dom/ScriptElement.h \
    dom/ScriptExecutionContext.h \
    dom/SelectElement.h \
    dom/SelectorNodeList.h \
    dom/SpaceSplitString.h \
    dom/StaticNodeList.h \
    dom/StyledElement.h \
    dom/StyleElement.h \
    dom/TagNodeList.h \
    dom/TextEvent.h \
    dom/Text.h \
    dom/Touch.h \
    dom/TouchEvent.h \
    dom/TouchList.h \
    dom/TransformSource.h \
    dom/Traversal.h \
    dom/TreeDepthLimit.h \
    dom/TreeWalker.h \
    dom/UIEvent.h \
    dom/UIEventWithKeyState.h \
    dom/UserGestureIndicator.h \
    dom/WebKitAnimationEvent.h \
    dom/WebKitTransitionEvent.h \
    dom/WheelEvent.h \
    dom/XMLTokenizer.h \
    editing/AppendNodeCommand.h \
    editing/ApplyStyleCommand.h \
    editing/BreakBlockquoteCommand.h \
    editing/CompositeEditCommand.h \
    editing/CreateLinkCommand.h \
    editing/DeleteButtonController.h \
    editing/DeleteButton.h \
    editing/DeleteFromTextNodeCommand.h \
    editing/DeleteSelectionCommand.h \
    editing/EditCommand.h \
    editing/Editor.h \
    editing/FormatBlockCommand.h \
    editing/htmlediting.h \
    editing/HTMLInterchange.h \
    editing/IndentOutdentCommand.h \
    editing/InsertIntoTextNodeCommand.h \
    editing/InsertLineBreakCommand.h \
    editing/InsertListCommand.h \
    editing/InsertNodeBeforeCommand.h \
    editing/InsertParagraphSeparatorCommand.h \
    editing/InsertTextCommand.h \
    editing/JoinTextNodesCommand.h \
    editing/markup.h \
    editing/MergeIdenticalElementsCommand.h \
    editing/ModifySelectionListLevel.h \
    editing/MoveSelectionCommand.h \
    editing/RemoveCSSPropertyCommand.h \
    editing/RemoveFormatCommand.h \
    editing/RemoveNodeCommand.h \
    editing/RemoveNodePreservingChildrenCommand.h \
    editing/ReplaceNodeWithSpanCommand.h \
    editing/ReplaceSelectionCommand.h \
    editing/SelectionController.h \
    editing/SetNodeAttributeCommand.h \
    editing/SmartReplace.h \
    editing/SplitElementCommand.h \
    editing/SplitTextNodeCommand.h \
    editing/SplitTextNodeContainingElementCommand.h \
    editing/TextIterator.h \
    editing/TypingCommand.h \
    editing/UnlinkCommand.h \
    editing/VisiblePosition.h \
    editing/VisibleSelection.h \
    editing/visible_units.h \
    editing/WrapContentsInDummySpanCommand.h \
    history/BackForwardList.h \
    history/CachedFrame.h \
    history/CachedPage.h \
    history/HistoryItem.h \
    history/PageCache.h \
    html/Blob.h \
    html/canvas/CanvasGradient.h \
    html/canvas/CanvasPattern.h \
    html/canvas/CanvasPixelArray.h \
    html/canvas/CanvasRenderingContext.h \
    html/canvas/CanvasRenderingContext2D.h \
    html/canvas/CanvasStyle.h \
    html/CollectionCache.h \
    html/DataGridColumn.h \
    html/DataGridColumnList.h \
    html/DateComponents.h \
    html/DOMDataGridDataSource.h \
    html/DOMFormData.h \
    html/File.h \
    html/FileError.h \
    html/FileList.h \
    html/FileReader.h \
    html/FileStream.h \
    html/FileStreamClient.h \
    html/FileStreamProxy.h \
    html/FileThread.h \
    html/FileThreadTask.h \
    html/FormDataList.h \
    html/HTMLAllCollection.h \
    html/HTMLAnchorElement.h \
    html/HTMLAppletElement.h \
    html/HTMLAreaElement.h \
    html/HTMLAudioElement.h \
    html/HTMLBaseElement.h \
    html/HTMLBaseFontElement.h \
    html/HTMLBlockquoteElement.h \
    html/HTMLBodyElement.h \
    html/HTMLBRElement.h \
    html/HTMLButtonElement.h \
    html/HTMLCanvasElement.h \
    html/HTMLCollection.h \
    html/HTMLDataGridCellElement.h \
    html/HTMLDataGridColElement.h \
    html/HTMLDataGridElement.h \
    html/HTMLDataGridRowElement.h \
    html/HTMLDirectoryElement.h \
    html/HTMLDivElement.h \
    html/HTMLDListElement.h \
    html/HTMLDocument.h \
    html/HTMLElement.h \
    html/HTMLEmbedElement.h \
    html/HTMLFieldSetElement.h \
    html/HTMLFontElement.h \
    html/HTMLFormCollection.h \
    html/HTMLFormControlElement.h \
    html/HTMLFormElement.h \
    html/HTMLFrameElementBase.h \
    html/HTMLFrameElement.h \
    html/HTMLFrameOwnerElement.h \
    html/HTMLFrameSetElement.h \
    html/HTMLHeadElement.h \
    html/HTMLHeadingElement.h \
    html/HTMLHRElement.h \
    html/HTMLHtmlElement.h \
    html/HTMLIFrameElement.h \
    html/HTMLImageElement.h \
    html/HTMLImageLoader.h \
    html/HTMLInputElement.h \
    html/HTMLIsIndexElement.h \
    html/HTMLKeygenElement.h \
    html/HTMLLabelElement.h \
    html/HTMLLegendElement.h \
    html/HTMLLIElement.h \
    html/HTMLLinkElement.h \
    html/HTMLMapElement.h \
    html/HTMLMarqueeElement.h \
    html/HTMLMediaElement.h \
    html/HTMLMenuElement.h \
    html/HTMLMetaElement.h \
    html/HTMLMeterElement.h \
    html/HTMLModElement.h \
    html/HTMLNameCollection.h \
    html/HTMLNoScriptElement.h \
    html/HTMLObjectElement.h \
    html/HTMLOListElement.h \
    html/HTMLOptGroupElement.h \
    html/HTMLOptionElement.h \
    html/HTMLOptionsCollection.h \
    html/HTMLParagraphElement.h \
    html/HTMLParamElement.h \
    html/HTMLParserErrorCodes.h \
    html/HTMLParser.h \
    html/HTMLPlugInElement.h \
    html/HTMLPlugInImageElement.h \
    html/HTMLPreElement.h \
    html/HTMLProgressElement.h \
    html/HTMLQuoteElement.h \
    html/HTMLScriptElement.h \
    html/HTMLSelectElement.h \
    html/HTMLSourceElement.h \
    html/HTMLStyleElement.h \
    html/HTMLTableCaptionElement.h \
    html/HTMLTableCellElement.h \
    html/HTMLTableColElement.h \
    html/HTMLTableElement.h \
    html/HTMLTablePartElement.h \
    html/HTMLTableRowElement.h \
    html/HTMLTableRowsCollection.h \
    html/HTMLTableSectionElement.h \
    html/HTMLTextAreaElement.h \
    html/HTMLTitleElement.h \
    html/HTMLTokenizer.h \
    html/HTMLUListElement.h \
    html/HTMLVideoElement.h \
    html/HTMLViewSourceDocument.h \
    html/ImageData.h \
    html/LabelsNodeList.h \
    html/PreloadScanner.h \
    html/StepRange.h \
    html/TimeRanges.h \
    html/ValidityState.h \
    inspector/ConsoleMessage.h \
    inspector/InjectedScript.h \
    inspector/InjectedScriptHost.h \
    inspector/InspectorBackend.h \
    inspector/InspectorController.h \
    inspector/InspectorDatabaseResource.h \
    inspector/InspectorDOMStorageResource.h \
    inspector/InspectorFrontend.h \
    inspector/InspectorFrontendClient.h \
    inspector/InspectorFrontendClientLocal.h \
    inspector/InspectorFrontendHost.h \
    inspector/InspectorResource.h \
    inspector/InspectorTimelineAgent.h \
    inspector/ScriptGCEventListener.h \
    inspector/TimelineRecordFactory.h \
    loader/appcache/ApplicationCacheGroup.h \
    loader/appcache/ApplicationCacheHost.h \
    loader/appcache/ApplicationCache.h \
    loader/appcache/ApplicationCacheResource.h \
    loader/appcache/ApplicationCacheStorage.h \
    loader/appcache/ApplicationCacheStorageManager.h \
    loader/appcache/DOMApplicationCache.h \
    loader/appcache/ManifestParser.h \
    loader/archive/ArchiveFactory.h \
    loader/archive/ArchiveResourceCollection.h \
    loader/archive/ArchiveResource.h \
    loader/CachedCSSStyleSheet.h \
    loader/CachedFont.h \
    loader/CachedImage.h \
    loader/CachedResourceClientWalker.h \
    loader/CachedResource.h \
    loader/CachedResourceHandle.h \
    loader/CachedScript.h \
    loader/CachedXSLStyleSheet.h \
    loader/Cache.h \
    loader/CrossOriginAccessControl.h \
    loader/CrossOriginPreflightResultCache.h \
    loader/DocLoader.h \
    loader/DocumentLoader.h \
    loader/DocumentThreadableLoader.h \
    loader/FormState.h \
    loader/FrameLoader.h \
    loader/FTPDirectoryDocument.h \
    loader/FTPDirectoryParser.h \
    loader/icon/IconDatabase.h \
    loader/icon/IconLoader.h \
    loader/icon/IconRecord.h \
    loader/icon/PageURLRecord.h \
    loader/ImageDocument.h \
    loader/ImageLoader.h \
    loader/loader.h \
    loader/MainResourceLoader.h \
    loader/MediaDocument.h \
    loader/NavigationAction.h \
    loader/NetscapePlugInStreamLoader.h \
    loader/PlaceholderDocument.h \
    loader/PluginDocument.h \
    loader/ProgressTracker.h \
    loader/Request.h \
    loader/ResourceLoader.h \
    loader/SubresourceLoader.h \
    loader/TextDocument.h \
    loader/TextResourceDecoder.h \
    loader/ThreadableLoader.h \
    loader/WorkerThreadableLoader.h \
    mathml/MathMLElement.h \
    mathml/MathMLInlineContainerElement.h \
    mathml/MathMLMathElement.h \
    mathml/MathMLTextElement.h \
    mathml/RenderMathMLBlock.h \
    mathml/RenderMathMLFraction.h \
    mathml/RenderMathMLMath.h \
    mathml/RenderMathMLOperator.h \
    mathml/RenderMathMLRoot.h \
    mathml/RenderMathMLRow.h \
    mathml/RenderMathMLSquareRoot.h \
    mathml/RenderMathMLSubSup.h \
    mathml/RenderMathMLUnderOver.h \
    notifications/Notification.h \
    notifications/NotificationCenter.h \
    notifications/NotificationPresenter.h \
    notifications/NotificationContents.h \
    page/animation/AnimationBase.h \
    page/animation/AnimationController.h \
    page/animation/CompositeAnimation.h \
    page/animation/ImplicitAnimation.h \
    page/animation/KeyframeAnimation.h \
    page/BarInfo.h \
    page/Chrome.h \
    page/Console.h \
    page/ContextMenuController.h \
    page/ContextMenuProvider.h \
    page/Coordinates.h \
    page/DOMSelection.h \
    page/DOMTimer.h \
    page/DOMWindow.h \
    page/DragController.h \
    page/EventHandler.h \
    page/EventSource.h \
    page/FocusController.h \
    page/Frame.h \
    page/FrameTree.h \
    page/FrameView.h \
    page/Geolocation.h \
    page/GeolocationPositionCache.h \
    page/Geoposition.h \
    page/HaltablePlugin.h \
    page/History.h \
    page/Location.h \
    page/MouseEventWithHitTestResults.h \
    page/NavigatorBase.h \
    page/Navigator.h \
    page/PageGroup.h \
    page/PageGroupLoadDeferrer.h \
    page/Page.h \
    page/PluginHalter.h \
    page/PluginHalterClient.h \
    page/PrintContext.h \
    page/Screen.h \
    page/SecurityOrigin.h \
    page/Settings.h \
    page/SpatialNavigation.h \
    page/WindowFeatures.h \
    page/WorkerNavigator.h \
    page/XSSAuditor.h \
    page/ZoomMode.h \
    platform/animation/Animation.h \
    platform/animation/AnimationList.h \
    platform/Arena.h \
    platform/ContentType.h \
    platform/ContextMenu.h \
    platform/CrossThreadCopier.h \
    platform/DeprecatedPtrListImpl.h \
    platform/DragData.h \
    platform/DragImage.h \
    platform/FileChooser.h \
    platform/GeolocationService.h \
    platform/image-decoders/ImageDecoder.h \
    platform/mock/GeolocationServiceMock.h \
    platform/graphics/BitmapImage.h \
    platform/graphics/Color.h \
    platform/graphics/filters/FEBlend.h \
    platform/graphics/filters/FEColorMatrix.h \
    platform/graphics/filters/FEComponentTransfer.h \
    platform/graphics/filters/FEComposite.h \
    platform/graphics/filters/FEGaussianBlur.h \
    platform/graphics/filters/FilterEffect.h \
    platform/graphics/filters/SourceAlpha.h \
    platform/graphics/filters/SourceGraphic.h \
    platform/graphics/FloatPoint3D.h \
    platform/graphics/FloatPoint.h \
    platform/graphics/FloatQuad.h \
    platform/graphics/FloatRect.h \
    platform/graphics/FloatSize.h \
    platform/graphics/FontData.h \
    platform/graphics/FontDescription.h \
    platform/graphics/FontFamily.h \
    platform/graphics/Font.h \
    platform/graphics/GeneratedImage.h \
    platform/graphics/Gradient.h \
    platform/graphics/GraphicsContext.h \
    platform/graphics/GraphicsTypes.h \
    platform/graphics/Image.h \
    platform/graphics/ImageSource.h \
    platform/graphics/IntPoint.h \
    platform/graphics/IntPointHash.h \
    platform/graphics/IntRect.h \
    platform/graphics/IntRectRegion.h \
    platform/graphics/MediaPlayer.h \
    platform/graphics/Path.h \
    platform/graphics/PathTraversalState.h \
    platform/graphics/Pattern.h \
    platform/graphics/Pen.h \
    platform/graphics/SegmentedFontData.h \
    platform/graphics/SimpleFontData.h \
    platform/graphics/Tile.h \
    platform/graphics/TiledBackingStore.h \    
    platform/graphics/TiledBackingStoreClient.h \
    platform/graphics/transforms/Matrix3DTransformOperation.h \
    platform/graphics/transforms/MatrixTransformOperation.h \
    platform/graphics/transforms/PerspectiveTransformOperation.h \
    platform/graphics/transforms/RotateTransformOperation.h \
    platform/graphics/transforms/ScaleTransformOperation.h \
    platform/graphics/transforms/SkewTransformOperation.h \
    platform/graphics/transforms/TransformationMatrix.h \
    platform/graphics/transforms/TransformOperations.h \
    platform/graphics/transforms/TranslateTransformOperation.h \
    platform/KURL.h \
    platform/Length.h \
    platform/LinkHash.h \
    platform/Logging.h \
    platform/MIMETypeRegistry.h \
    platform/network/AuthenticationChallengeBase.h \
    platform/network/AuthenticationClient.h \
    platform/network/Credential.h \
    platform/network/FormDataBuilder.h \
    platform/network/FormData.h \
    platform/network/HTTPHeaderMap.h \
    platform/network/HTTPParsers.h \
    platform/network/NetworkStateNotifier.h \
    platform/network/ProtectionSpace.h \
    platform/network/ResourceErrorBase.h \
    platform/network/ResourceHandle.h \
    platform/network/ResourceRequestBase.h \
    platform/network/ResourceResponseBase.h \
    platform/PlatformTouchEvent.h \
    platform/PlatformTouchPoint.h \
    platform/Scrollbar.h \
    platform/ScrollbarThemeComposite.h \
    platform/ScrollView.h \
    platform/SharedBuffer.h \
    platform/sql/SQLiteDatabase.h \
    platform/sql/SQLiteFileSystem.h \
    platform/sql/SQLiteStatement.h \
    platform/sql/SQLiteTransaction.h \
    platform/sql/SQLValue.h \
    platform/StringPattern.h \
    platform/text/AtomicString.h \
    platform/text/Base64.h \
    platform/text/BidiContext.h \
    platform/text/RegularExpression.h \
    platform/text/SegmentedString.h \
    platform/text/StringBuilder.h \
    platform/text/StringImpl.h \
    platform/text/TextCodec.h \
    platform/text/TextCodecLatin1.h \
    platform/text/TextCodecUserDefined.h \
    platform/text/TextCodecUTF16.h \
    platform/text/TextEncoding.h \
    platform/text/TextEncodingRegistry.h \
    platform/text/TextStream.h \
    platform/text/UnicodeRange.h \
    platform/text/transcoder/FontTranscoder.h \
    platform/ThreadGlobalData.h \
    platform/ThreadTimers.h \
    platform/Timer.h \
    platform/Widget.h \
    plugins/MimeTypeArray.h \
    plugins/MimeType.h \
    plugins/PluginArray.h \
    plugins/PluginDatabase.h \
    plugins/PluginData.h \
    plugins/PluginDebug.h \
    plugins/Plugin.h \
    plugins/PluginMainThreadScheduler.h \
    plugins/PluginPackage.h \
    plugins/PluginStream.h \
    plugins/PluginView.h \
    plugins/win/PluginMessageThrottlerWin.h \
    rendering/AutoTableLayout.h \
    rendering/break_lines.h \
    rendering/CounterNode.h \
    rendering/EllipsisBox.h \
    rendering/FixedTableLayout.h \
    rendering/HitTestResult.h \
    rendering/InlineBox.h \
    rendering/InlineFlowBox.h \
    rendering/InlineTextBox.h \
    rendering/LayoutState.h \
    rendering/MediaControlElements.h \
    rendering/PointerEventsHitRules.h \
    rendering/RenderApplet.h \
    rendering/RenderArena.h \
    rendering/RenderBlock.h \
    rendering/RenderBox.h \
    rendering/RenderBoxModelObject.h \
    rendering/RenderBR.h \
    rendering/RenderButton.h \
    rendering/RenderCounter.h \
    rendering/RenderDataGrid.h \
    rendering/RenderEmbeddedObject.h \
    rendering/RenderFieldset.h \
    rendering/RenderFileUploadControl.h \
    rendering/RenderFlexibleBox.h \
    rendering/RenderForeignObject.h \
    rendering/RenderFrame.h \
    rendering/RenderFrameBase.h \
    rendering/RenderFrameSet.h \
    rendering/RenderHTMLCanvas.h \
    rendering/RenderIFrame.h \
    rendering/RenderImageGeneratedContent.h \
    rendering/RenderImage.h \
    rendering/RenderInline.h \
    rendering/RenderLayer.h \
    rendering/RenderLineBoxList.h \
    rendering/RenderListBox.h \
    rendering/RenderListItem.h \
    rendering/RenderListMarker.h \
    rendering/RenderMarquee.h \
    rendering/RenderMedia.h \
    rendering/RenderMenuList.h \
    rendering/RenderMeter.h \
    rendering/RenderObjectChildList.h \
    rendering/RenderObject.h \
    rendering/RenderPart.h \
    rendering/RenderPath.h \
    rendering/RenderProgress.h \
    rendering/RenderReplaced.h \
    rendering/RenderReplica.h \
    rendering/RenderRuby.h \
    rendering/RenderRubyBase.h \
    rendering/RenderRubyRun.h \
    rendering/RenderRubyText.h \
    rendering/RenderScrollbar.h \
    rendering/RenderScrollbarPart.h \
    rendering/RenderScrollbarTheme.h \
    rendering/RenderSlider.h \
    rendering/RenderSVGBlock.h \
    rendering/RenderSVGContainer.h \
    rendering/RenderSVGGradientStop.h \
    rendering/RenderSVGHiddenContainer.h \
    rendering/RenderSVGImage.h \
    rendering/RenderSVGInline.h \
    rendering/RenderSVGInlineText.h \
    rendering/RenderSVGModelObject.h \
    rendering/RenderSVGResource.h \
    rendering/RenderSVGResourceClipper.h \
    rendering/RenderSVGResourceFilter.h \ 
    rendering/RenderSVGResourceGradient.h \
    rendering/RenderSVGResourceLinearGradient.h \
    rendering/RenderSVGResourceMarker.h \
    rendering/RenderSVGResourceMasker.h \
    rendering/RenderSVGResourcePattern.h \
    rendering/RenderSVGResourceRadialGradient.h \
    rendering/RenderSVGResourceSolidColor.h \
    rendering/RenderSVGRoot.h \
    rendering/RenderSVGShadowTreeRootContainer.h \
    rendering/RenderSVGText.h \
    rendering/RenderSVGTextPath.h \
    rendering/RenderSVGTransformableContainer.h \
    rendering/RenderSVGTSpan.h \
    rendering/RenderSVGViewportContainer.h \
    rendering/RenderTableCell.h \
    rendering/RenderTableCol.h \
    rendering/RenderTable.h \
    rendering/RenderTableRow.h \
    rendering/RenderTableSection.h \
    rendering/RenderTextControl.h \
    rendering/RenderTextControlMultiLine.h \
    rendering/RenderTextControlSingleLine.h \
    rendering/RenderTextFragment.h \
    rendering/RenderText.h \
    rendering/RenderTheme.h \
    rendering/RenderTreeAsText.h \
    rendering/RenderVideo.h \
    rendering/RenderView.h \
    rendering/RenderWidget.h \
    rendering/RenderWordBreak.h \
    rendering/RootInlineBox.h \
    rendering/ScrollBehavior.h \
    rendering/style/BindingURI.h \
    rendering/style/ContentData.h \
    rendering/style/CounterDirectives.h \
    rendering/style/CursorData.h \
    rendering/style/CursorList.h \
    rendering/style/FillLayer.h \
    rendering/style/KeyframeList.h \
    rendering/style/NinePieceImage.h \
    rendering/style/RenderStyle.h \
    rendering/style/ShadowData.h \
    rendering/style/StyleBackgroundData.h \
    rendering/style/StyleBoxData.h \
    rendering/style/StyleCachedImage.h \
    rendering/style/StyleFlexibleBoxData.h \
    rendering/style/StyleGeneratedImage.h \
    rendering/style/StyleInheritedData.h \
    rendering/style/StyleMarqueeData.h \
    rendering/style/StyleMultiColData.h \
    rendering/style/StyleRareInheritedData.h \
    rendering/style/StyleRareNonInheritedData.h \
    rendering/style/StyleReflection.h \
    rendering/style/StyleSurroundData.h \
    rendering/style/StyleTransformData.h \
    rendering/style/StyleVisualData.h \
    rendering/style/SVGRenderStyleDefs.h \
    rendering/style/SVGRenderStyle.h \
    rendering/SVGCharacterData.h \
    rendering/SVGCharacterLayoutInfo.h \
    rendering/SVGInlineFlowBox.h \
    rendering/SVGInlineTextBox.h \
    rendering/SVGMarkerData.h \
    rendering/SVGMarkerLayoutInfo.h \
    rendering/SVGRenderSupport.h \
    rendering/SVGRenderTreeAsText.h \
    rendering/SVGRootInlineBox.h \
    rendering/SVGShadowTreeElements.h \
    rendering/SVGTextChunkLayoutInfo.h \
    rendering/SVGTextLayoutUtilities.h \
    rendering/TextControlInnerElements.h \
    rendering/TransformState.h \
    svg/animation/SMILTimeContainer.h \
    svg/animation/SMILTime.h \
    svg/animation/SVGSMILElement.h \
    svg/ColorDistance.h \
    svg/graphics/filters/SVGFEConvolveMatrix.h \
    svg/graphics/filters/SVGFEDiffuseLighting.h \
    svg/graphics/filters/SVGFEDisplacementMap.h \
    svg/graphics/filters/SVGFEFlood.h \
    svg/graphics/filters/SVGFEImage.h \
    svg/graphics/filters/SVGFELighting.h \
    svg/graphics/filters/SVGFEMerge.h \
    svg/graphics/filters/SVGFEMorphology.h \
    svg/graphics/filters/SVGFEOffset.h \
    svg/graphics/filters/SVGFESpecularLighting.h \
    svg/graphics/filters/SVGFETile.h \
    svg/graphics/filters/SVGFETurbulence.h \
    svg/graphics/filters/SVGFilterBuilder.h \
    svg/graphics/filters/SVGFilter.h \
    svg/graphics/filters/SVGLightSource.h \
    svg/graphics/SVGImage.h \
    svg/SVGAElement.h \
    svg/SVGAltGlyphElement.h \
    svg/SVGAngle.h \
    svg/SVGAnimateColorElement.h \
    svg/SVGAnimatedPathData.h \
    svg/SVGAnimatedPoints.h \
    svg/SVGAnimatedProperty.h \
    svg/SVGAnimatedPropertySynchronizer.h \
    svg/SVGAnimatedPropertyTraits.h \
    svg/SVGAnimatedTemplate.h \
    svg/SVGAnimateElement.h \
    svg/SVGAnimateMotionElement.h \
    svg/SVGAnimateTransformElement.h \
    svg/SVGAnimationElement.h \
    svg/SVGCircleElement.h \
    svg/SVGClipPathElement.h \
    svg/SVGColor.h \
    svg/SVGComponentTransferFunctionElement.h \
    svg/SVGCursorElement.h \
    svg/SVGDefsElement.h \
    svg/SVGDescElement.h \
    svg/SVGDocumentExtensions.h \
    svg/SVGDocument.h \
    svg/SVGElement.h \
    svg/SVGElementInstance.h \
    svg/SVGElementInstanceList.h \
    svg/SVGElementRareData.h \
    svg/SVGEllipseElement.h \
    svg/SVGExternalResourcesRequired.h \
    svg/SVGFEBlendElement.h \
    svg/SVGFEColorMatrixElement.h \
    svg/SVGFEComponentTransferElement.h \
    svg/SVGFECompositeElement.h \
    svg/SVGFEDiffuseLightingElement.h \
    svg/SVGFEDisplacementMapElement.h \
    svg/SVGFEDistantLightElement.h \
    svg/SVGFEFloodElement.h \
    svg/SVGFEFuncAElement.h \
    svg/SVGFEFuncBElement.h \
    svg/SVGFEFuncGElement.h \
    svg/SVGFEFuncRElement.h \
    svg/SVGFEGaussianBlurElement.h \
    svg/SVGFEImageElement.h \
    svg/SVGFELightElement.h \
    svg/SVGFEMergeElement.h \
    svg/SVGFEMergeNodeElement.h \
    svg/SVGFEMorphologyElement.h \
    svg/SVGFEOffsetElement.h \
    svg/SVGFEPointLightElement.h \
    svg/SVGFESpecularLightingElement.h \
    svg/SVGFESpotLightElement.h \
    svg/SVGFETileElement.h \
    svg/SVGFETurbulenceElement.h \
    svg/SVGFilterElement.h \
    svg/SVGFilterPrimitiveStandardAttributes.h \
    svg/SVGFitToViewBox.h \
    svg/SVGFontData.h \
    svg/SVGFontElement.h \
    svg/SVGFontFaceElement.h \
    svg/SVGFontFaceFormatElement.h \
    svg/SVGFontFaceNameElement.h \
    svg/SVGFontFaceSrcElement.h \
    svg/SVGFontFaceUriElement.h \
    svg/SVGForeignObjectElement.h \
    svg/SVGGElement.h \
    svg/SVGGlyphElement.h \
    svg/SVGGradientElement.h \
    svg/SVGHKernElement.h \
    svg/SVGImageElement.h \
    svg/SVGImageLoader.h \
    svg/SVGLangSpace.h \
    svg/SVGLength.h \
    svg/SVGLengthList.h \
    svg/SVGLinearGradientElement.h \
    svg/SVGLineElement.h \
    svg/SVGLocatable.h \
    svg/SVGMarkerElement.h \
    svg/SVGMaskElement.h \
    svg/SVGMetadataElement.h \
    svg/SVGMissingGlyphElement.h \
    svg/SVGMPathElement.h \
    svg/SVGNumberList.h \
    svg/SVGPaint.h \
    svg/SVGParserUtilities.h \
    svg/SVGPathElement.h \
    svg/SVGPathSegArc.h \
    svg/SVGPathSegClosePath.h \
    svg/SVGPathSegCurvetoCubic.h \
    svg/SVGPathSegCurvetoCubicSmooth.h \
    svg/SVGPathSegCurvetoQuadratic.h \
    svg/SVGPathSegCurvetoQuadraticSmooth.h \
    svg/SVGPathSegLineto.h \
    svg/SVGPathSegLinetoHorizontal.h \
    svg/SVGPathSegLinetoVertical.h \
    svg/SVGPathSegList.h \
    svg/SVGPathSegMoveto.h \
    svg/SVGPatternElement.h \
    svg/SVGPointList.h \
    svg/SVGPolyElement.h \
    svg/SVGPolygonElement.h \
    svg/SVGPolylineElement.h \
    svg/SVGPreserveAspectRatio.h \
    svg/SVGRadialGradientElement.h \
    svg/SVGRectElement.h \
    svg/SVGScriptElement.h \
    svg/SVGSetElement.h \
    svg/SVGStopElement.h \
    svg/SVGStringList.h \
    svg/SVGStylable.h \
    svg/SVGStyledElement.h \
    svg/SVGStyledLocatableElement.h \
    svg/SVGStyledTransformableElement.h \
    svg/SVGStyleElement.h \
    svg/SVGSVGElement.h \
    svg/SVGSwitchElement.h \
    svg/SVGSymbolElement.h \
    svg/SVGTests.h \
    svg/SVGTextContentElement.h \
    svg/SVGTextElement.h \
    svg/SVGTextPathElement.h \
    svg/SVGTextPositioningElement.h \
    svg/SVGTitleElement.h \
    svg/SVGTransformable.h \
    svg/SVGTransformDistance.h \
    svg/SVGTransform.h \
    svg/SVGTransformList.h \
    svg/SVGTRefElement.h \
    svg/SVGTSpanElement.h \
    svg/SVGURIReference.h \
    svg/SVGUseElement.h \
    svg/SVGViewElement.h \
    svg/SVGViewSpec.h \
    svg/SVGVKernElement.h \
    svg/SVGZoomAndPan.h \
    svg/SVGZoomEvent.h \
    wml/WMLAccessElement.h \
    wml/WMLAElement.h \
    wml/WMLAnchorElement.h \
    wml/WMLBRElement.h \
    wml/WMLCardElement.h \
    wml/WMLDocument.h \
    wml/WMLDoElement.h \
    wml/WMLElement.h \
    wml/WMLErrorHandling.h \
    wml/WMLEventHandlingElement.h \
    wml/WMLFieldSetElement.h \
    wml/WMLFormControlElement.h \
    wml/WMLGoElement.h \
    wml/WMLImageElement.h \
    wml/WMLImageLoader.h \
    wml/WMLInputElement.h \
    wml/WMLInsertedLegendElement.h \
    wml/WMLIntrinsicEvent.h \
    wml/WMLIntrinsicEventHandler.h \
    wml/WMLMetaElement.h \
    wml/WMLNoopElement.h \
    wml/WMLOnEventElement.h \
    wml/WMLOptGroupElement.h \
    wml/WMLOptionElement.h \
    wml/WMLPageState.h \
    wml/WMLPElement.h \
    wml/WMLPostfieldElement.h \
    wml/WMLPrevElement.h \
    wml/WMLRefreshElement.h \
    wml/WMLSelectElement.h \
    wml/WMLSetvarElement.h \
    wml/WMLTableElement.h \
    wml/WMLTaskElement.h \
    wml/WMLTemplateElement.h \
    wml/WMLTimerElement.h \
    wml/WMLVariables.h \
    workers/AbstractWorker.h \
    workers/DedicatedWorkerContext.h \
    workers/DedicatedWorkerThread.h \
    workers/SharedWorker.h \
    workers/WorkerContext.h \
    workers/Worker.h \
    workers/WorkerLocation.h \
    workers/WorkerMessagingProxy.h \
    workers/WorkerRunLoop.h \
    workers/WorkerScriptLoader.h \
    workers/WorkerThread.h \
    xml/DOMParser.h \
    xml/NativeXPathNSResolver.h \
    xml/XMLHttpRequest.h \
    xml/XMLHttpRequestUpload.h \
    xml/XMLSerializer.h \
    xml/XPathEvaluator.h \
    xml/XPathExpression.h \
    xml/XPathExpressionNode.h \
    xml/XPathFunctions.h \
    xml/XPathNamespace.h \
    xml/XPathNodeSet.h \
    xml/XPathNSResolver.h \
    xml/XPathParser.h \
    xml/XPathPath.h \
    xml/XPathPredicate.h \
    xml/XPathResult.h \
    xml/XPathStep.h \
    xml/XPathUtil.h \
    xml/XPathValue.h \
    xml/XPathVariableReference.h \
    xml/XSLImportRule.h \
    xml/XSLStyleSheet.h \
    xml/XSLTExtensions.h \
    xml/XSLTProcessor.h \
    xml/XSLTUnicodeSort.h

# Generic OpenVG support (not depending on Olympia)
HEADERS += \
    platform/graphics/openvg/EGLDisplayOpenVG.h \
    platform/graphics/openvg/ImageBufferData.h \
    platform/graphics/openvg/GradientOpenVG.h \
    platform/graphics/openvg/PainterOpenVG.h \
    platform/graphics/openvg/PaintOpenVG.h \
    platform/graphics/openvg/PatternOpenVG.h \
    platform/graphics/openvg/PlatformPathOpenVG.h \
    platform/graphics/openvg/SharedResourceOpenVG.h \
    platform/graphics/openvg/SurfaceOpenVG.h \
    platform/graphics/openvg/TiledImageOpenVG.h

SOURCES += \
    platform/image-decoders/openvg/ImageDecoderOpenVG.cpp \
    platform/graphics/openvg/EGLDisplayOpenVG.cpp \
    platform/graphics/openvg/GradientOpenVG.cpp \
    platform/graphics/openvg/ImageBufferOpenVG.cpp \
    platform/graphics/openvg/ImageOpenVG.cpp \
    platform/graphics/openvg/GraphicsContextOpenVG.cpp \
    platform/graphics/openvg/PathOpenVG.cpp \
    platform/graphics/openvg/PainterOpenVG.cpp \
    platform/graphics/openvg/PaintOpenVG.cpp \
    platform/graphics/openvg/PatternOpenVG.cpp \
    platform/graphics/openvg/SharedResourceOpenVG.cpp \
    platform/graphics/openvg/SurfaceOpenVG.cpp \
    platform/graphics/openvg/TiledImageOpenVG.cpp \
    platform/graphics/openvg/VGUtils.cpp \


# Olympia specific code
HEADERS += \
    bindings/cpp/WebDOMCString.h \
    bindings/cpp/WebDOMEventTarget.h \
    bindings/cpp/WebDOMObject.h \
    bindings/cpp/WebDOMString.h \
    bindings/cpp/WebExceptionHandler.h \
    bindings/cpp/WebNativeEventListener.h \
    bindings/cpp/WebNativeNodeFilterCondition.h \
    $$PWD/platform/blackberry/MenuEventProxy.h \
    $$PWD/platform/blackberry/GeolocationServiceBlackBerry.h \
    $$PWD/../WebKit/blackberry/WebCoreSupport/FrameLoaderClientBlackBerry.h \
    $$PWD/platform/blackberry/BlackBerryCookieCache.h \


SOURCES += \
    bindings/cpp/WebDOMCString.cpp \
    bindings/cpp/WebDOMDOMWindowCustom.cpp \
    bindings/cpp/WebDOMEventListenerCustom.cpp \
    bindings/cpp/WebDOMEventTarget.cpp \
    bindings/cpp/WebDOMHTMLCollectionCustom.cpp \
    bindings/cpp/WebDOMHTMLDocumentCustom.cpp \
    bindings/cpp/WebDOMHTMLOptionsCollectionCustom.cpp \
    bindings/cpp/WebDOMNodeCustom.cpp \
    bindings/cpp/WebDOMNodeFilterCustom.cpp \
    bindings/cpp/WebDOMString.cpp \
    bindings/cpp/WebExceptionHandler.cpp \
    bindings/cpp/WebNativeEventListener.cpp \
    bindings/cpp/WebNativeNodeFilterCondition.cpp \
    bindings/js/ScriptControllerBlackBerry.cpp \
    page/blackberry/AccessibilityObjectBlackBerry.cpp \
    page/blackberry/DragControllerBlackBerry.cpp \
    page/blackberry/EventHandlerBlackBerry.cpp \
    page/blackberry/FrameBlackBerry.cpp \
    platform/graphics/blackberry/FontBlackBerry.cpp \
    platform/graphics/blackberry/FontCacheBlackBerry.cpp \
    platform/graphics/blackberry/FontCustomPlatformDataBlackBerry.cpp \
    platform/graphics/blackberry/FontPlatformDataBlackBerry.cpp \
    platform/graphics/GlyphPageTreeNode.cpp \
    platform/graphics/blackberry/GlyphPageTreeNodeBlackBerry.cpp \
    platform/graphics/blackberry/SimpleFontDataBlackBerry.cpp \
    platform/graphics/blackberry/IconBlackBerry.cpp \
    platform/graphics/blackberry/IntPointBlackBerry.cpp \
    platform/graphics/blackberry/IntRectBlackBerry.cpp \
    platform/graphics/blackberry/IntSizeBlackBerry.cpp \
    platform/graphics/blackberry/PatternBlackBerry.cpp \
    platform/network/blackberry/AboutData.cpp \
    platform/network/blackberry/NetworkManager.cpp \
    platform/network/blackberry/NetworkStateNotifierBlackBerry.cpp \
    platform/network/blackberry/ResourceErrorBlackBerry.cpp \
    platform/network/blackberry/ResourceHandleBlackBerry.cpp \
    platform/network/blackberry/ResourceRequestBlackBerry.cpp \
    editing/blackberry/EditorBlackBerry.cpp \
    editing/blackberry/SmartReplaceBlackBerry.cpp \
    platform/blackberry/ClipboardBlackBerry.cpp \
    platform/blackberry/ContextMenuItemBlackBerry.cpp \
    platform/blackberry/ContextMenuBlackBerry.cpp \
    platform/blackberry/CookieJarBlackBerry.cpp \
    platform/blackberry/BlackBerryCookieCache.cpp \
    platform/blackberry/CursorBlackBerry.cpp \
    platform/blackberry/DragDataBlackBerry.cpp \
    platform/blackberry/DragImageBlackBerry.cpp \
    platform/blackberry/EventLoopBlackBerry.cpp \
    platform/blackberry/FileChooserBlackBerry.cpp \
    platform/blackberry/FileSystemBlackBerry.cpp \
    platform/blackberry/GeolocationServiceBlackBerry.cpp \
    platform/blackberry/SharedBufferBlackBerry.cpp \
    platform/blackberry/SSLKeyGeneratorBlackBerry.cpp \
    platform/blackberry/SystemTimeBlackBerry.cpp \
    platform/blackberry/KURLBlackBerry.cpp \
    platform/blackberry/Localizations.cpp \
    platform/blackberry/MIMETypeRegistryBlackBerry.cpp \
    platform/blackberry/PasteboardBlackBerry.cpp \
    platform/blackberry/PlatformKeyboardEventBlackBerry.cpp \
    platform/blackberry/PlatformMouseEventBlackBerry.cpp \
    platform/blackberry/PlatformScreenBlackBerry.cpp \
    platform/blackberry/PlatformTouchEventBlackBerry.cpp \
    platform/blackberry/PlatformTouchPointBlackBerry.cpp \
    platform/blackberry/PopupMenuBlackBerry.cpp \
    platform/blackberry/RenderThemeBlackBerry.cpp \
    platform/blackberry/ScrollbarBlackBerry.cpp \
    platform/blackberry/ScrollbarThemeBlackBerry.cpp \
    platform/blackberry/ScrollViewBlackBerry.cpp \
    platform/blackberry/SearchPopupMenuBlackBerry.cpp \
    platform/blackberry/SharedTimerBlackBerry.cpp \
    platform/blackberry/SoundBlackBerry.cpp \
    platform/blackberry/LoggingBlackBerry.cpp \
    platform/text/blackberry/StringBlackBerry.cpp \
    platform/blackberry/TemporaryLinkStubs.cpp \
    platform/text/blackberry/TextBreakIteratorBlackBerry.cpp \
    platform/text/blackberry/TextCodecBlackBerry.cpp \
    platform/blackberry/WheelEventBlackBerry.cpp \
    platform/blackberry/WidgetBlackBerry.cpp \
    plugins/blackberry/PluginDataBlackBerry.cpp \
#    plugins/blackberry/PluginPackageBlackBerry.cpp \
#    plugins/blackberry/PluginViewBlackBerry.cpp \
    ../WebKit/blackberry/WebCoreSupport/CacheClientBlackBerry.cpp \
    ../WebKit/blackberry/WebCoreSupport/ChromeClientBlackBerry.cpp \
    ../WebKit/blackberry/WebCoreSupport/CollectableObjects.cpp \
    ../WebKit/blackberry/WebCoreSupport/ContextMenuClientBlackBerry.cpp \
    ../WebKit/blackberry/WebCoreSupport/DragClientBlackBerry.cpp \
    ../WebKit/blackberry/WebCoreSupport/EditorClientBlackBerry.cpp \
    ../WebKit/blackberry/WebCoreSupport/EditCommandBlackBerry.cpp \
    ../WebKit/blackberry/WebCoreSupport/FrameLoaderClientBlackBerry.cpp \
    ../WebKit/blackberry/WebCoreSupport/InspectorClientBlackBerry.cpp \
    ../WebKit/blackberry/WebCoreSupport/JavaScriptDebuggerBlackBerry.cpp \
    ../WebKit/blackberry/WebCoreSupport/MainThreadBlackBerry.cpp \
    ../WebKit/blackberry/WebCoreSupport/PluginWidget.cpp \
    ../WebKit/blackberry/WebKitSupport/BackingStoreScrollbar.cpp \
    ../WebKit/blackberry/WebKitSupport/BackingStoreTile.cpp \
    ../WebKit/blackberry/WebKitSupport/InputHandler.cpp \
    ../WebKit/blackberry/WebKitSupport/BlackBerryAnimation.cpp \
    ../WebKit/blackberry/WebKitSupport/OutOfMemoryHandler.cpp \
    ../WebKit/blackberry/WebKitSupport/RenderQueue.cpp \
    ../WebKit/blackberry/WebKitSupport/SelectionHandler.cpp \
    ../WebKit/blackberry/WebKitSupport/SurfacePool.cpp \
    ../WebKit/blackberry/WebKitSupport/TouchEventHandler.cpp \
    ../WebKit/blackberry/WebKitSupport/WebPlugin.cpp \
    ../WebKit/blackberry/Api/BackingStore.cpp \
    ../WebKit/blackberry/Api/BlackBerryGlobal.cpp \
    ../WebKit/blackberry/Api/OlympiaString.cpp \
    ../WebKit/blackberry/Api/ResourceHolderImpl.cpp \
    ../WebKit/blackberry/Api/Version.cpp \
    ../WebKit/blackberry/Api/WebPage.cpp \
    ../WebKit/blackberry/Api/WebPageGroupLoadDeferrer.cpp \
    ../WebKit/blackberry/Api/WebSettings.cpp

contains(DEFINES, WTF_USE_QT_MOBILE_THEME=1) {
    HEADERS += platform/qt/Maemo5Webstyle.h
    SOURCES += platform/qt/Maemo5Webstyle.cpp
}

maemo5 {
    HEADERS += ../WebKit/qt/WebCoreSupport/QtMaemoWebPopup.h
    SOURCES += ../WebKit/qt/WebCoreSupport/QtMaemoWebPopup.cpp
}


    mac {
        SOURCES += \
            platform/text/cf/StringCF.cpp \
            platform/text/cf/StringImplCF.cpp \
            platform/cf/SharedBufferCF.cpp
        LIBS_PRIVATE += -framework Carbon -framework AppKit
    }

    win32-* {
        LIBS += -lgdi32
        LIBS += -lOle32
        LIBS += -luser32
    }
    wince* {
        LIBS += -lmmtimer
        LIBS += -lOle32
    }

contains(DEFINES, ENABLE_NETSCAPE_PLUGIN_API=1) {

    SOURCES += plugins/npapi.cpp

    symbian {
        SOURCES += \
        plugins/symbian/PluginPackageSymbian.cpp \
        plugins/symbian/PluginDatabaseSymbian.cpp \
        plugins/symbian/PluginViewSymbian.cpp \
        plugins/symbian/PluginContainerSymbian.cpp

        HEADERS += \
        plugins/symbian/PluginContainerSymbian.h \
        plugins/symbian/npinterface.h

        LIBS += -lefsrv

    } else {

        unix {
    
            mac {
                SOURCES += \
                    plugins/mac/PluginPackageMac.cpp \
                    plugins/mac/PluginViewMac.cpp
                OBJECTIVE_SOURCES += \
                    platform/text/mac/StringImplMac.mm \
                    platform/mac/WebCoreNSStringExtras.mm
                INCLUDEPATH += platform/mac
                # Note: XP_MACOSX is defined in npapi.h
            } else {
                !embedded {
                    CONFIG += x11
                    LIBS += -lXrender
                }
                SOURCES += \
                    plugins/qt/PluginContainerQt.cpp \
                    plugins/qt/PluginPackageQt.cpp \
                    plugins/qt/PluginViewQt.cpp
                HEADERS += \
                    plugins/qt/PluginContainerQt.h
                DEFINES += XP_UNIX
            }
        }
    
        win32-* {
            INCLUDEPATH += $$PWD/plugins/win \
                           $$PWD/platform/win
    
            SOURCES += plugins/win/PluginDatabaseWin.cpp \
                       plugins/win/PluginPackageWin.cpp \
                       plugins/win/PluginMessageThrottlerWin.cpp \
                       plugins/win/PluginViewWin.cpp \
                       platform/win/BitmapInfo.cpp \
                       platform/win/WebCoreInstanceHandle.cpp
    
            LIBS += \
                -ladvapi32 \
                -lgdi32 \
                -lshell32 \
                -lshlwapi \
                -luser32 \
                -lversion
        }
    }

} else {
    SOURCES += \
        plugins/PluginPackageNone.cpp \
        plugins/PluginViewNone.cpp
}

contains(DEFINES, ENABLE_SQLITE=1) {
    # somewhat copied from src/plugins/sqldrivers/sqlite/sqlite.pro
    !linux-* {
        exists( $${SQLITE3SRCDIR}/sqlite3.c )  {
            # we have source - use it
            CONFIG(release, debug|release):DEFINES *= NDEBUG
            DEFINES += SQLITE_CORE SQLITE_OMIT_LOAD_EXTENSION SQLITE_OMIT_COMPLETE
            INCLUDEPATH += $${SQLITE3SRCDIR}
            SOURCES += $${SQLITE3SRCDIR}/sqlite3.c
        } else {
            # Use sqlite3 from the underlying OS
            CONFIG(QTDIR_build) {
                QMAKE_CXXFLAGS *= $$QT_CFLAGS_SQLITE
                LIBS *= $$QT_LFLAGS_SQLITE
            } else {
                INCLUDEPATH += $${SQLITE3SRCDIR}
                LIBS += -lsqlite3 -ljpeg -lpng -lxml2 -licuuc 
            }
        }
    }
    SOURCES += \
        platform/sql/SQLiteAuthorizer.cpp \
        platform/sql/SQLiteDatabase.cpp \
        platform/sql/SQLiteFileSystem.cpp \
        platform/sql/SQLiteStatement.cpp \
        platform/sql/SQLiteTransaction.cpp \
        platform/sql/SQLValue.cpp \
        storage/AbstractDatabase.cpp \
        storage/Database.cpp \
        storage/DatabaseAuthorizer.cpp \
        storage/DatabaseSync.cpp
}


contains(DEFINES, ENABLE_DATABASE=1) {
    SOURCES += \
        storage/ChangeVersionWrapper.cpp \
        storage/DatabaseTask.cpp \
        storage/DatabaseTracker.cpp \
        storage/DatabaseTrackerManager.cpp \
        storage/OriginQuotaManager.cpp \
        storage/OriginUsageRecord.cpp \
        storage/SQLResultSet.cpp \
        storage/SQLResultSetRowList.cpp \
        storage/SQLStatement.cpp \
        storage/SQLTransaction.cpp \
        storage/SQLTransactionClient.cpp \
        storage/SQLTransactionCoordinator.cpp \
        storage/SQLTransactionSync.cpp \
        bindings/js/JSCustomSQLStatementErrorCallback.cpp \
        bindings/js/JSDatabaseCustom.cpp \
        bindings/js/JSDatabaseSyncCustom.cpp \
        bindings/js/JSSQLResultSetRowListCustom.cpp \
        bindings/js/JSSQLTransactionCustom.cpp \
        bindings/js/JSSQLTransactionSyncCustom.cpp

    contains(DEFINES, ENABLE_SINGLE_THREADED=1) {
        SOURCES += storage/single-threaded/DatabaseThreadPseudo.cpp
    } else {
        SOURCES += storage/DatabaseThread.cpp
    }
}

contains(DEFINES, ENABLE_INDEXED_DATABASE=1) {
    HEADERS += \
        storage/IDBAny.h \
        storage/IDBCallbacks.h \
        storage/IDBDatabase.h \
        storage/IDBDatabaseImpl.h \
        storage/IDBDatabaseError.h \
        storage/IDBDatabaseException.h \
        storage/IDBDatabaseRequest.h \
        storage/IDBErrorEvent.h \
        storage/IDBEvent.h \
        storage/IDBObjectStore.h \
        storage/IDBObjectStoreRequest.h \
        storage/IDBRequest.h \
        storage/IDBSuccessEvent.h \
        storage/IndexedDatabase.h \
        storage/IndexedDatabaseImpl.h \
        storage/IndexedDatabaseRequest.h

    SOURCES += \
        bindings/js/JSIDBAnyCustom.cpp \
        storage/IDBAny.cpp \
        storage/IDBDatabaseImpl.cpp \
        storage/IDBDatabaseRequest.cpp \
        storage/IDBErrorEvent.cpp \
        storage/IDBEvent.cpp \
        storage/IDBObjectStore.cpp \
        storage/IDBObjectStoreRequest.cpp \
        storage/IDBRequest.cpp \
        storage/IDBSuccessEvent.cpp \
        storage/IndexedDatabase.cpp \
        storage/IndexedDatabaseImpl.cpp \
        storage/IndexedDatabaseRequest.cpp
}

contains(DEFINES, ENABLE_DOM_STORAGE=1) {
    HEADERS += \
        storage/AbstractDatabase.h \
        storage/ChangeVersionWrapper.h \
        storage/DatabaseAuthorizer.h \
        storage/Database.h \
        storage/DatabaseCallback.h \
        storage/DatabaseSync.h \
        storage/DatabaseTask.h \
        storage/DatabaseThread.h \
        storage/DatabaseTracker.h \
        storage/DatabaseTrackerManager.h \
        storage/LocalStorageTask.h \
        storage/LocalStorageThread.h \
        storage/OriginQuotaManager.h \
        storage/OriginUsageRecord.h \
        storage/SQLResultSet.h \
        storage/SQLResultSetRowList.h \
        storage/SQLStatement.h \
        storage/SQLTransaction.h \
        storage/SQLTransactionClient.h \
        storage/SQLTransactionCoordinator.h \
        storage/SQLTransactionSync.h \
        storage/SQLTransactionSyncCallback.h \
        storage/StorageArea.h \
        storage/StorageAreaImpl.h \
        storage/StorageAreaSync.h \
        storage/StorageEvent.h \
        storage/StorageEventDispatcher.h \
        storage/Storage.h \
        storage/StorageMap.h \
        storage/StorageNamespace.h \
        storage/StorageNamespaceImpl.h \
        storage/StorageSyncManager.h

    SOURCES += \
        bindings/js/JSStorageCustom.cpp \
        storage/LocalStorageTask.cpp \
        storage/Storage.cpp \
        storage/StorageAreaImpl.cpp \
        storage/StorageAreaSync.cpp \
        storage/StorageEvent.cpp \
        storage/StorageEventDispatcher.cpp \
        storage/StorageMap.cpp \
        storage/StorageNamespace.cpp \
        storage/StorageNamespaceImpl.cpp \
        storage/StorageSyncManager.cpp

    contains(DEFINES, ENABLE_SINGLE_THREADED=1) {
        SOURCES += \
            storage/single-threaded/LocalStorageThreadPseudo.cpp \
            storage/single-threaded/StorageAreaSyncSingleThreaded.cpp
    } else {
        SOURCES += storage/LocalStorageThread.cpp
    }
}

contains(DEFINES, ENABLE_ICONDATABASE=1) {
    SOURCES += \
        loader/icon/IconDatabase.cpp \
        loader/icon/IconRecord.cpp \
        loader/icon/PageURLRecord.cpp
} else {
    SOURCES += \
        loader/icon/IconDatabaseNone.cpp
}

contains(DEFINES, ENABLE_WORKERS=1) {
    SOURCES += \
        bindings/js/JSDedicatedWorkerContextCustom.cpp \
        bindings/js/JSWorkerConstructor.cpp \
        bindings/js/JSWorkerContextBase.cpp \
        bindings/js/JSWorkerContextCustom.cpp \
        bindings/js/JSWorkerCustom.cpp \
        bindings/js/WorkerScriptController.cpp \
        loader/WorkerThreadableLoader.cpp \
        page/WorkerNavigator.cpp \
        workers/AbstractWorker.cpp \
        workers/DedicatedWorkerContext.cpp \
        workers/DedicatedWorkerThread.cpp \
        workers/Worker.cpp \
        workers/WorkerContext.cpp \
        workers/WorkerLocation.cpp \
        workers/WorkerMessagingProxy.cpp \
        workers/WorkerRunLoop.cpp \
        workers/WorkerThread.cpp \
        workers/WorkerScriptLoader.cpp
}

contains(DEFINES, ENABLE_SHARED_WORKERS=1) {
    SOURCES += \
        bindings/js/JSSharedWorkerConstructor.cpp \
        bindings/js/JSSharedWorkerCustom.cpp \
        workers/DefaultSharedWorkerRepository.cpp \
        workers/SharedWorker.cpp \
        workers/SharedWorkerContext.cpp \
        workers/SharedWorkerThread.cpp
}

contains(DEFINES, ENABLE_VIDEO=1) {
    SOURCES += \
        html/HTMLAudioElement.cpp \
        html/HTMLMediaElement.cpp \
        html/HTMLSourceElement.cpp \
        html/HTMLVideoElement.cpp \
        html/TimeRanges.cpp \
        platform/graphics/MediaPlayer.cpp \
        platform/graphics/blackberry/MediaPlayerPrivateBlackBerry.cpp \
        platform/graphics/blackberry/MediaPlayerProxy.cpp \
        rendering/MediaControlElements.cpp \
        rendering/RenderVideo.cpp \
        rendering/RenderMedia.cpp \
        bindings/js/JSAudioConstructor.cpp
}

contains(DEFINES, ENABLE_XPATH=1) {
    SOURCES += \
        xml/NativeXPathNSResolver.cpp \
        xml/XPathEvaluator.cpp \
        xml/XPathExpression.cpp \
        xml/XPathExpressionNode.cpp \
        xml/XPathFunctions.cpp \
        xml/XPathNamespace.cpp \
        xml/XPathNodeSet.cpp \
        xml/XPathNSResolver.cpp \
        xml/XPathParser.cpp \
        xml/XPathPath.cpp \
        xml/XPathPredicate.cpp \
        xml/XPathResult.cpp \
        xml/XPathStep.cpp \
        xml/XPathUtil.cpp \
        xml/XPathValue.cpp \
        xml/XPathVariableReference.cpp
}

unix:!mac:CONFIG += link_pkgconfig

contains(DEFINES, ENABLE_XSLT=1) {
    olympia-*|win32-msvc-fledge {
        SOURCES += \
            bindings/js/JSXSLTProcessorConstructor.cpp \
            bindings/js/JSXSLTProcessorCustom.cpp \
            dom/TransformSourceLibxslt.cpp \
            xml/XSLStyleSheetLibxslt.cpp \
            xml/XSLTProcessor.cpp \
            xml/XSLTProcessorLibxslt.cpp \
            xml/XSLImportRule.cpp \
            xml/XSLTExtensions.cpp \
            xml/XSLTUnicodeSort.cpp
    } else {
    tobe|!tobe: QT += xmlpatterns

    SOURCES += \
        bindings/js/JSXSLTProcessorConstructor.cpp \
        bindings/js/JSXSLTProcessorCustom.cpp \
        dom/TransformSourceQt.cpp \
        xml/XSLStyleSheetQt.cpp \
        xml/XSLTProcessor.cpp \
        xml/XSLTProcessorQt.cpp
    }
}

contains(DEFINES, ENABLE_FILTERS=1) {
    SOURCES += \
        platform/graphics/filters/FEBlend.cpp \
        platform/graphics/filters/FEColorMatrix.cpp \
        platform/graphics/filters/FEComponentTransfer.cpp \
        platform/graphics/filters/FEComposite.cpp \
        platform/graphics/filters/FEGaussianBlur.cpp \
        platform/graphics/filters/FilterEffect.cpp \
        platform/graphics/filters/SourceAlpha.cpp \
        platform/graphics/filters/SourceGraphic.cpp
}

contains(DEFINES, ENABLE_MATHML=1) {
    SOURCES += \
        mathml/MathMLElement.cpp \
        mathml/MathMLInlineContainerElement.cpp \
        mathml/MathMLMathElement.cpp \
        mathml/MathMLTextElement.cpp \
        mathml/RenderMathMLBlock.cpp \
        mathml/RenderMathMLFraction.cpp \
        mathml/RenderMathMLMath.cpp \
        mathml/RenderMathMLOperator.cpp \
        mathml/RenderMathMLRoot.cpp \
        mathml/RenderMathMLRow.cpp \
        mathml/RenderMathMLSquareRoot.cpp \
        mathml/RenderMathMLSubSup.cpp \
        mathml/RenderMathMLUnderOver.cpp
}

contains(DEFINES, ENABLE_WML=1) {
    SOURCES += \
        wml/WMLAElement.cpp \
        wml/WMLAccessElement.cpp \
        wml/WMLAnchorElement.cpp \
        wml/WMLBRElement.cpp \
        wml/WMLCardElement.cpp \
        wml/WMLDoElement.cpp \
        wml/WMLDocument.cpp \
        wml/WMLElement.cpp \
        wml/WMLErrorHandling.cpp \
        wml/WMLEventHandlingElement.cpp \
        wml/WMLFormControlElement.cpp \
        wml/WMLFieldSetElement.cpp \
        wml/WMLGoElement.cpp \
        wml/WMLImageElement.cpp \
        wml/WMLImageLoader.cpp \
        wml/WMLInputElement.cpp \
        wml/WMLInsertedLegendElement.cpp \
        wml/WMLIntrinsicEvent.cpp \
        wml/WMLIntrinsicEventHandler.cpp \
        wml/WMLMetaElement.cpp \
        wml/WMLNoopElement.cpp \
        wml/WMLOnEventElement.cpp \
        wml/WMLPElement.cpp \
        wml/WMLOptGroupElement.cpp \
        wml/WMLOptionElement.cpp \
        wml/WMLPageState.cpp \
        wml/WMLPostfieldElement.cpp \
        wml/WMLPrevElement.cpp \
        wml/WMLRefreshElement.cpp \
        wml/WMLSelectElement.cpp \
        wml/WMLSetvarElement.cpp \
        wml/WMLTableElement.cpp \
        wml/WMLTaskElement.cpp \
        wml/WMLTemplateElement.cpp \
        wml/WMLTimerElement.cpp \
        wml/WMLVariables.cpp
}

contains(DEFINES, ENABLE_META_VIEWPORT=1) {
   HEADERS += \
       dom/ViewportArguments.h

   SOURCES += \
       dom/ViewportArguments.cpp

    FEATURE_DEFINES_BINDINGS += ENABLE_META_VIEWPORT=1
}


contains(DEFINES, ENABLE_XHTMLMP=1) {
    SOURCES += \
        html/HTMLNoScriptElement.cpp
}

contains(DEFINES, ENABLE_QT_BEARER=1) {
    HEADERS += \
        platform/network/qt/NetworkStateNotifierPrivate.h

    SOURCES += \
        platform/network/qt/NetworkStateNotifierQt.cpp

    CONFIG += mobility
    MOBILITY += bearer
}

contains(DEFINES, ENABLE_SVG=1) {
    SOURCES += \
# TODO: this-one-is-not-auto-added! FIXME! tmp/SVGElementFactory.cpp \
        bindings/js/JSSVGElementInstanceCustom.cpp \
        bindings/js/JSSVGLengthCustom.cpp \
        bindings/js/JSSVGMatrixCustom.cpp \
        bindings/js/JSSVGPathSegCustom.cpp \
        bindings/js/JSSVGPathSegListCustom.cpp \
        css/SVGCSSComputedStyleDeclaration.cpp \
        css/SVGCSSParser.cpp \
        css/SVGCSSStyleSelector.cpp \
        rendering/style/SVGRenderStyle.cpp \
        rendering/style/SVGRenderStyleDefs.cpp \
        svg/SVGZoomEvent.cpp \
        rendering/PointerEventsHitRules.cpp \
        svg/SVGDocumentExtensions.cpp \
        svg/SVGImageLoader.cpp \
        svg/ColorDistance.cpp \
        svg/SVGAElement.cpp \
        svg/SVGAltGlyphElement.cpp \
        svg/SVGAngle.cpp \
        svg/SVGAnimateColorElement.cpp \
        svg/SVGAnimatedPathData.cpp \
        svg/SVGAnimatedPoints.cpp \
        svg/SVGAnimateElement.cpp \
        svg/SVGAnimateMotionElement.cpp \
        svg/SVGAnimateTransformElement.cpp \
        svg/SVGAnimationElement.cpp \
        svg/SVGCircleElement.cpp \
        svg/SVGClipPathElement.cpp \
        svg/SVGColor.cpp \
        svg/SVGComponentTransferFunctionElement.cpp \
        svg/SVGCursorElement.cpp \
        svg/SVGDefsElement.cpp \
        svg/SVGDescElement.cpp \
        svg/SVGDocument.cpp \
        svg/SVGElement.cpp \
        svg/SVGElementInstance.cpp \
        svg/SVGElementInstanceList.cpp \
        svg/SVGEllipseElement.cpp \
        svg/SVGExternalResourcesRequired.cpp \
        svg/SVGFEBlendElement.cpp \
        svg/SVGFEColorMatrixElement.cpp \
        svg/SVGFEComponentTransferElement.cpp \
        svg/SVGFECompositeElement.cpp \
        svg/SVGFEDiffuseLightingElement.cpp \
        svg/SVGFEDisplacementMapElement.cpp \
        svg/SVGFEDistantLightElement.cpp \
        svg/SVGFEFloodElement.cpp \
        svg/SVGFEFuncAElement.cpp \
        svg/SVGFEFuncBElement.cpp \
        svg/SVGFEFuncGElement.cpp \
        svg/SVGFEFuncRElement.cpp \
        svg/SVGFEGaussianBlurElement.cpp \
        svg/SVGFEImageElement.cpp \
        svg/SVGFELightElement.cpp \
        svg/SVGFEMergeElement.cpp \
        svg/SVGFEMergeNodeElement.cpp \
        svg/SVGFEMorphologyElement.cpp \
        svg/SVGFEOffsetElement.cpp \
        svg/SVGFEPointLightElement.cpp \
        svg/SVGFESpecularLightingElement.cpp \
        svg/SVGFESpotLightElement.cpp \
        svg/SVGFETileElement.cpp \
        svg/SVGFETurbulenceElement.cpp \
        svg/SVGFilterElement.cpp \
        svg/SVGFilterPrimitiveStandardAttributes.cpp \
        svg/SVGFitToViewBox.cpp \
        svg/SVGFont.cpp \
        svg/SVGFontData.cpp \
        svg/SVGFontElement.cpp \
        svg/SVGFontFaceElement.cpp \
        svg/SVGFontFaceFormatElement.cpp \
        svg/SVGFontFaceNameElement.cpp \
        svg/SVGFontFaceSrcElement.cpp \
        svg/SVGFontFaceUriElement.cpp \
        svg/SVGForeignObjectElement.cpp \
        svg/SVGGElement.cpp \
        svg/SVGGlyphElement.cpp \
        svg/SVGGradientElement.cpp \
        svg/SVGHKernElement.cpp \
        svg/SVGImageElement.cpp \
        svg/SVGLangSpace.cpp \
        svg/SVGLength.cpp \
        svg/SVGLengthList.cpp \
        svg/SVGLinearGradientElement.cpp \
        svg/SVGLineElement.cpp \
        svg/SVGLocatable.cpp \
        svg/SVGMarkerElement.cpp \
        svg/SVGMaskElement.cpp \
        svg/SVGMetadataElement.cpp \
        svg/SVGMissingGlyphElement.cpp \
        svg/SVGMPathElement.cpp \
        svg/SVGNumberList.cpp \
        svg/SVGPaint.cpp \
        svg/SVGParserUtilities.cpp \
        svg/SVGPathElement.cpp \
        svg/SVGPathSegArc.cpp \
        svg/SVGPathSegClosePath.cpp \
        svg/SVGPathSegCurvetoCubic.cpp \
        svg/SVGPathSegCurvetoCubicSmooth.cpp \
        svg/SVGPathSegCurvetoQuadratic.cpp \
        svg/SVGPathSegCurvetoQuadraticSmooth.cpp \
        svg/SVGPathSegLineto.cpp \
        svg/SVGPathSegLinetoHorizontal.cpp \
        svg/SVGPathSegLinetoVertical.cpp \
        svg/SVGPathSegList.cpp \
        svg/SVGPathSegMoveto.cpp \
        svg/SVGPatternElement.cpp \
        svg/SVGPointList.cpp \
        svg/SVGPolyElement.cpp \
        svg/SVGPolygonElement.cpp \
        svg/SVGPolylineElement.cpp \
        svg/SVGPreserveAspectRatio.cpp \
        svg/SVGRadialGradientElement.cpp \
        svg/SVGRectElement.cpp \
        svg/SVGScriptElement.cpp \
        svg/SVGSetElement.cpp \
        svg/SVGStopElement.cpp \
        svg/SVGStringList.cpp \
        svg/SVGStylable.cpp \
        svg/SVGStyledElement.cpp \
        svg/SVGStyledLocatableElement.cpp \
        svg/SVGStyledTransformableElement.cpp \
        svg/SVGStyleElement.cpp \
        svg/SVGSVGElement.cpp \
        svg/SVGSwitchElement.cpp \
        svg/SVGSymbolElement.cpp \
        svg/SVGTests.cpp \
        svg/SVGTextContentElement.cpp \
        svg/SVGTextElement.cpp \
        svg/SVGTextPathElement.cpp \
        svg/SVGTextPositioningElement.cpp \
        svg/SVGTitleElement.cpp \
        svg/SVGTransformable.cpp \
        svg/SVGTransform.cpp \
        svg/SVGTransformDistance.cpp \
        svg/SVGTransformList.cpp \
        svg/SVGTRefElement.cpp \
        svg/SVGTSpanElement.cpp \
        svg/SVGURIReference.cpp \
        svg/SVGUseElement.cpp \
        svg/SVGViewElement.cpp \
        svg/SVGViewSpec.cpp \
        svg/SVGVKernElement.cpp \
        svg/SVGZoomAndPan.cpp \
        svg/animation/SMILTime.cpp \
        svg/animation/SMILTimeContainer.cpp \
        svg/animation/SVGSMILElement.cpp \
        svg/graphics/filters/SVGFEConvolveMatrix.cpp \
        svg/graphics/filters/SVGFEDiffuseLighting.cpp \
        svg/graphics/filters/SVGFEDisplacementMap.cpp \
        svg/graphics/filters/SVGFEFlood.cpp \
        svg/graphics/filters/SVGFEImage.cpp \
        svg/graphics/filters/SVGFELighting.cpp \
        svg/graphics/filters/SVGFEMerge.cpp \
        svg/graphics/filters/SVGFEMorphology.cpp \
        svg/graphics/filters/SVGFEOffset.cpp \
        svg/graphics/filters/SVGFESpecularLighting.cpp \
        svg/graphics/filters/SVGFETile.cpp \
        svg/graphics/filters/SVGFETurbulence.cpp \
        svg/graphics/filters/SVGFilter.cpp \
        svg/graphics/filters/SVGFilterBuilder.cpp \
        svg/graphics/filters/SVGLightSource.cpp \
        svg/graphics/SVGImage.cpp \
        rendering/RenderForeignObject.cpp \
        rendering/RenderPath.cpp \
        rendering/RenderSVGBlock.cpp \
        rendering/RenderSVGContainer.cpp \
        rendering/RenderSVGGradientStop.cpp \
        rendering/RenderSVGHiddenContainer.cpp \
        rendering/RenderSVGImage.cpp \
        rendering/RenderSVGInline.cpp \
        rendering/RenderSVGInlineText.cpp \
        rendering/RenderSVGModelObject.cpp \
        rendering/RenderSVGResource.cpp \
        rendering/RenderSVGResourceClipper.cpp \
        rendering/RenderSVGResourceFilter.cpp \
        rendering/RenderSVGResourceGradient.cpp \
        rendering/RenderSVGResourceLinearGradient.cpp \
        rendering/RenderSVGResourceMarker.cpp \
        rendering/RenderSVGResourceMasker.cpp \
        rendering/RenderSVGResourcePattern.cpp \
        rendering/RenderSVGResourceRadialGradient.cpp \
        rendering/RenderSVGResourceSolidColor.cpp \
        rendering/RenderSVGRoot.cpp \
        rendering/RenderSVGShadowTreeRootContainer.cpp \
        rendering/RenderSVGText.cpp \
        rendering/RenderSVGTextPath.cpp \
        rendering/RenderSVGTransformableContainer.cpp \
        rendering/RenderSVGTSpan.cpp \
        rendering/RenderSVGViewportContainer.cpp \
        rendering/SVGCharacterData.cpp \
        rendering/SVGCharacterLayoutInfo.cpp \
        rendering/SVGInlineFlowBox.cpp \
        rendering/SVGInlineTextBox.cpp \
        rendering/SVGMarkerLayoutInfo.cpp \
        rendering/SVGRenderSupport.cpp \
        rendering/SVGRootInlineBox.cpp \
        rendering/SVGShadowTreeElements.cpp \
        rendering/SVGTextLayoutUtilities.cpp
}

contains(DEFINES, ENABLE_JAVASCRIPT_DEBUGGER=1) {
    SOURCES += \
        bindings/js/JSJavaScriptCallFrameCustom.cpp \
        bindings/js/ScriptProfiler.cpp \
        bindings/js/JavaScriptCallFrame.cpp \
}

contains(DEFINES, ENABLE_OFFLINE_WEB_APPLICATIONS=1) {
SOURCES += \
    loader/appcache/ApplicationCache.cpp \
    loader/appcache/ApplicationCacheGroup.cpp \
    loader/appcache/ApplicationCacheHost.cpp \
    loader/appcache/ApplicationCacheResource.cpp \
    loader/appcache/ApplicationCacheStorage.cpp \
    loader/appcache/ApplicationCacheStorageManager.cpp \
    loader/appcache/DOMApplicationCache.cpp \
    loader/appcache/ManifestParser.cpp \
    bindings/js/JSDOMApplicationCacheCustom.cpp
}

contains(DEFINES, ENABLE_WEB_SOCKETS=1) {
    HEADERS += \
        websockets/ThreadableWebSocketChannel.h \
        websockets/ThreadableWebSocketChannelClientWrapper.h \
        websockets/WebSocket.h \
        websockets/WebSocketChannel.h \
        websockets/WebSocketChannelClient.h \
        websockets/WebSocketHandshake.h \
        websockets/WebSocketHandshakeRequest.h

    SOURCES += \
        websockets/WebSocket.cpp \
        websockets/WebSocketChannel.cpp \
        websockets/WebSocketHandshake.cpp \
        websockets/WebSocketHandshakeRequest.cpp \
        websockets/ThreadableWebSocketChannel.cpp \
        platform/network/SocketStreamErrorBase.cpp \
        platform/network/SocketStreamHandleBase.cpp \
        platform/network/blackberry/SocketStreamHandleBlackBerry.cpp \
        bindings/js/JSWebSocketCustom.cpp \
        bindings/js/JSWebSocketConstructor.cpp

    contains(DEFINES, ENABLE_WORKERS=1) {
        HEADERS += \
            websockets/WorkerThreadableWebSocketChannel.h

        SOURCES += \
            websockets/WorkerThreadableWebSocketChannel.cpp
    }
}

contains(DEFINES, ENABLE_3D_CANVAS=1) {
HEADERS += \
	bindings/js/JSArrayBufferConstructor.h \
	bindings/js/JSArrayBufferViewHelper.h \
	bindings/js/JSInt8ArrayConstructor.h \
	bindings/js/JSFloatArrayConstructor.h \
	bindings/js/JSInt32ArrayConstructor.h \
	bindings/js/JSInt16ArrayConstructor.h \
	bindings/js/JSUint8ArrayConstructor.h \
	bindings/js/JSUint32ArrayConstructor.h \
	bindings/js/JSUint16ArrayConstructor.h \
	html/canvas/CanvasContextAttributes.h \
	html/canvas/CanvasObject.h \
	html/canvas/WebGLActiveInfo.h \
	html/canvas/ArrayBuffer.h \
	html/canvas/ArrayBufferView.h \
	html/canvas/WebGLBuffer.h \
	html/canvas/Int8Array.h \
	html/canvas/WebGLContextAttributes.h \
	html/canvas/FloatArray.h \
	html/canvas/WebGLFramebuffer.h \
	html/canvas/WebGLGetInfo.h \
	html/canvas/Int32Array.h \
	html/canvas/WebGLProgram.h \
	html/canvas/WebGLRenderbuffer.h \
	html/canvas/WebGLRenderingContext.h \
	html/canvas/WebGLShader.h \
	html/canvas/Int16Array.h \
	html/canvas/WebGLTexture.h \
	html/canvas/WebGLUniformLocation.h \
	html/canvas/Uint8Array.h \
	html/canvas/Uint32Array.h \
	html/canvas/Uint16Array.h \
    platform/graphics/GraphicsContext3D.h 

SOURCES += \
	bindings/js/JSArrayBufferConstructor.cpp \
	bindings/js/JSArrayBufferViewCustom.cpp \
	bindings/js/JSInt8ArrayConstructor.cpp \
	bindings/js/JSInt8ArrayCustom.cpp \
	bindings/js/JSFloatArrayConstructor.cpp \
	bindings/js/JSFloatArrayCustom.cpp \
	bindings/js/JSInt32ArrayConstructor.cpp \
	bindings/js/JSInt32ArrayCustom.cpp \
	bindings/js/JSWebGLRenderingContextCustom.cpp \
	bindings/js/JSInt16ArrayConstructor.cpp \
	bindings/js/JSInt16ArrayCustom.cpp \
	bindings/js/JSUint8ArrayConstructor.cpp \
	bindings/js/JSUint8ArrayCustom.cpp \
	bindings/js/JSUint32ArrayConstructor.cpp \
	bindings/js/JSUint32ArrayCustom.cpp \
	bindings/js/JSUint16ArrayConstructor.cpp \
	bindings/js/JSUint16ArrayCustom.cpp \
	html/canvas/CanvasContextAttributes.cpp \
    html/canvas/CanvasObject.cpp \
	html/canvas/ArrayBuffer.cpp \
	html/canvas/ArrayBufferView.cpp \
	html/canvas/WebGLBuffer.cpp \
	html/canvas/Int8Array.cpp \
	html/canvas/WebGLContextAttributes.cpp \
	html/canvas/FloatArray.cpp \
	html/canvas/WebGLFramebuffer.cpp \
	html/canvas/WebGLGetInfo.cpp \
	html/canvas/Int32Array.cpp \
	html/canvas/WebGLProgram.cpp \
	html/canvas/WebGLRenderbuffer.cpp \
	html/canvas/WebGLRenderingContext.cpp \
	html/canvas/WebGLShader.cpp \
	html/canvas/Int16Array.cpp \
	html/canvas/WebGLTexture.cpp \
	html/canvas/WebGLUniformLocation.cpp \
	html/canvas/Uint8Array.cpp \
	html/canvas/Uint32Array.cpp \
	html/canvas/Uint16Array.cpp \
    platform/graphics/GraphicsContext3D.cpp

}

contains(DEFINES, ENABLE_SYMBIAN_DIALOG_PROVIDERS) {
    # this feature requires the S60 platform private BrowserDialogsProvider.h header file
    # and is therefore not enabled by default but only meant for platform builds.
    symbian {
        LIBS += -lbrowserdialogsprovider
    }
}

include($$PWD/../WebKit/blackberry/Api/headers.pri)
HEADERS += $$WEBKIT_API_HEADERS

!CONFIG(QTDIR_build) {
    exists($$OUTPUT_DIR/include/QtWebKit/classheaders.pri): include($$OUTPUT_DIR/include/QtWebKit/classheaders.pri)
    WEBKIT_INSTALL_HEADERS = $$WEBKIT_API_HEADERS $$WEBKIT_CLASS_HEADERS

    !symbian {
        headers.files = $$WEBKIT_INSTALL_HEADERS

        headers.path = $$INSTALL_HEADERS/QtWebKit
        target.path = $$INSTALL_LIBS

        isEmpty(INSTALL_HEADERS): headers.path = $$[QT_INSTALL_HEADERS]/QtWebKit
        isEmpty(INSTALL_LIBS): target.path = $$[QT_INSTALL_LIBS]

        INSTALLS += target headers
    } else {
        # INSTALLS is not implemented in qmake's s60 generators, copy headers manually
        inst_headers.commands = $$QMAKE_COPY ${QMAKE_FILE_NAME} ${QMAKE_FILE_OUT}
        inst_headers.input = WEBKIT_INSTALL_HEADERS

        isEmpty(INSTALL_HEADERS): inst_headers.output = $$[QT_INSTALL_HEADERS]/QtWebKit/${QMAKE_FILE_BASE}${QMAKE_FILE_EXT}
        inst_headers.output = $$INSTALL_HEADERS/QtWebKit/${QMAKE_FILE_BASE}${QMAKE_FILE_EXT}

        QMAKE_EXTRA_COMPILERS += inst_headers

        install.depends += compiler_inst_headers_make_all
        QMAKE_EXTRA_TARGETS += install
    }

    prf.files = $$PWD/../WebKit/blackberry/Api/qtwebkit.prf
    prf.path = $$[QT_INSTALL_PREFIX]/mkspecs/features
    INSTALLS += prf

    win32-msvc-fledge|olympia-* {
        VERSION=
    }

    win32-*|wince* {
        DLLDESTDIR = $$OUTPUT_DIR/bin
        TARGET = $$qtLibraryTarget($$TARGET)

        dlltarget.commands = $(COPY_FILE) $(DESTDIR_TARGET) $$[QT_INSTALL_BINS]
        dlltarget.CONFIG = no_path
        INSTALLS += dlltarget
    }

    unix {
        CONFIG += create_pc create_prl
        QMAKE_PKGCONFIG_LIBDIR = $$target.path
        QMAKE_PKGCONFIG_INCDIR = $$headers.path
        QMAKE_PKGCONFIG_DESTDIR = pkgconfig
        lib_replace.match = $$DESTDIR
        lib_replace.replace = $$[QT_INSTALL_LIBS]
        QMAKE_PKGCONFIG_INSTALL_REPLACE += lib_replace
    }

    mac {
        !static:contains(QT_CONFIG, qt_framework):!CONFIG(webkit_no_framework) {
            !build_pass {
                message("Building QtWebKit as a framework, as that's how Qt was built. You can")
                message("override this by passing CONFIG+=webkit_no_framework to build-webkit.")

                CONFIG += build_all
            } else {
                debug_and_release:TARGET = $$qtLibraryTarget($$TARGET)
            }

            CONFIG += lib_bundle qt_no_framework_direct_includes qt_framework
            FRAMEWORK_HEADERS.version = Versions
            FRAMEWORK_HEADERS.files = $${headers.files}
            FRAMEWORK_HEADERS.path = Headers
            QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
        }

        QMAKE_LFLAGS_SONAME = "$${QMAKE_LFLAGS_SONAME}$${DESTDIR}$${QMAKE_DIR_SEP}"
        LIBS += -framework Carbon -framework AppKit
    }
}

CONFIG(QTDIR_build) {
    # Remove the following 2 lines if you want debug information in WebCore
    CONFIG -= separate_debug_info
    CONFIG += no_debug_info
}

!win32-g++:win32:contains(QMAKE_HOST.arch, x86_64):{
    asm_compiler.commands = ml64 /c
    asm_compiler.commands +=  /Fo ${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}
    asm_compiler.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_OBJ)}
    asm_compiler.input = ASM_SOURCES
    asm_compiler.variable_out = OBJECTS
    asm_compiler.name = compiling[asm] ${QMAKE_FILE_IN}
    silent:asm_compiler.commands = @echo compiling[asm] ${QMAKE_FILE_IN} && $$asm_compiler.commands
    QMAKE_EXTRA_COMPILERS += asm_compiler

    ASM_SOURCES += \
        plugins/win/PaintHooks.asm
   if(win32-msvc2005|win32-msvc2008):equals(TEMPLATE_PREFIX, "vc") {
        SOURCES += \
            plugins/win/PaintHooks.asm
    }
}

olympia-* {
    DEFINES -= WTF_USE_ACCELERATED_COMPOSITING
}

contains(DEFINES, WTF_USE_ACCELERATED_COMPOSITING) {
HEADERS += \
    rendering/RenderLayerBacking.h \
    rendering/RenderLayerCompositor.h \
    platform/graphics/GraphicsLayer.h \
    platform/graphics/GraphicsLayerClient.h \
    platform/graphics/qt/GraphicsLayerQt.h
SOURCES += \
    platform/graphics/GraphicsLayer.cpp \
    platform/graphics/qt/GraphicsLayerQt.cpp \
    rendering/RenderLayerBacking.cpp \
    rendering/RenderLayerCompositor.cpp
}

symbian {
    shared {
        contains(CONFIG, def_files) {
            DEF_FILE=../WebKit/qt/symbian
            # defFilePath is for Qt4.6 compatibility
            defFilePath=../WebKit/qt/symbian
        } else {
            MMP_RULES += EXPORTUNFROZEN
        }
    }
}

# Disable C++0x mode in WebCore for those who enabled it in their Qt's mkspec
*-g++*:QMAKE_CXXFLAGS -= -std=c++0x -std=gnu++0x

INCLUDEPATH += $$(SYSTEMINCDIR)

# Set WEBKIT_EGL_SURFACE_DEFAULT_COLORSPACE for xscale, pj4 and fledge. msm7k doesn't support this.
olympia-armcc-xscale|olympia-armcc-pj4|win32-msvc-fledge {
    DEFINES += WEBKIT_EGL_SURFACE_DEFAULT_COLORSPACE=EGL_COLORSPACE_sRGB
}

