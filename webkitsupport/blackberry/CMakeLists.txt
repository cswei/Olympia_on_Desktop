find_package(Qt4 COMPONENTS QtCore QtGui QtXml QtNetwork REQUIRED)
set(QT_USE_NETWORK TRUE)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDE_DIR})

find_package(ICU REQUIRED)
set(optionalLibs ${optionalLibs} ${ICU_LIBRARIES})

find_package(Freetype REQUIRED)
include_directories(${Freetype_INCLUDE_DIR})
set(optionalLibs ${optionalLibs} ${Freetype_LIBRARIES})
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
include_directories(/usr/X11/include)
endif ()

#find_package(Threads REQUIRED)
#include_directories(${THREAD_INCLUDE_DIR})
#set(optionalLibs ${optionalLibs} ${THREAD_LIBRARIES})

ADD_DEFINITIONS(-DBUILD_WEBKIT)
SET(DERIVED_SOURCE_DIR ${CMAKE_BINARY_DIR}/DerivedSources)
set(WebKitSupport_MOC_HDRS
    streams/qt/OlympiaNetworkCookieJarQt.h
    streams/qt/OlympiaHttpStreamQt.h
)
QT4_WRAP_CPP(WebKitSupport_MOC_SRCS ${WebKitSupport_MOC_HDRS})
QT4_GENERATE_MOC(OlympiaPlatformTimer.cpp ${DERIVED_SOURCE_DIR}/OlympiaPlatformTimer.moc)

SET(WebKit_INCLUDE_DIRECTORIES
    "${WEBKIT_DIR}"
    "${WEBKIT_DIR}/blackberry/Api"
    "${WEBCORE_DIR}"
    "${WEBCORE_DIR}/bindings/cpp"
    "${WEBCORE_DIR}/bindings/js"
    "${WEBCORE_DIR}/bridge"
    "${WEBCORE_DIR}/bridge/jsc"
    "${WEBCORE_DIR}/css"
    "${WEBCORE_DIR}/dom"
    "${WEBCORE_DIR}/editing"
    "${WEBCORE_DIR}/history"
    "${WEBCORE_DIR}/html"
    "${WEBCORE_DIR}/inspector"
    "${WEBCORE_DIR}/loader"
    "${WEBCORE_DIR}/loader/icon"
    "${WEBCORE_DIR}/page"
    "${WEBCORE_DIR}/page/animation"
    "${WEBCORE_DIR}/platform"
    "${WEBCORE_DIR}/platform/animation"
    "${WEBCORE_DIR}/platform/graphics"
    "${WEBCORE_DIR}/platform/graphics/openvg"
    "${WEBCORE_DIR}/platform/graphics/transforms"
    "${WEBCORE_DIR}/platform/network"
    "${WEBCORE_DIR}/platform/sql"
    "${WEBCORE_DIR}/platform/text"
    "${WEBCORE_DIR}/plugins"
    "${WEBCORE_DIR}/rendering"
    "${WEBCORE_DIR}/rendering/style"
    "${WEBCORE_DIR}/storage"
    "${WEBCORE_DIR}/svg"
    "${JAVASCRIPTCORE_DIR}"
    "${JAVASCRIPTCORE_DIR}/ForwardingHeaders"
    "${JAVASCRIPTCORE_DIR}/API"
    "${JAVASCRIPTCORE_DIR}/assembler"
    "${JAVASCRIPTCORE_DIR}/bytecode"
    "${JAVASCRIPTCORE_DIR}/bytecompiler"
    "${JAVASCRIPTCORE_DIR}/debugger"
    "${JAVASCRIPTCORE_DIR}/interpreter"
    "${JAVASCRIPTCORE_DIR}/jit"
    "${JAVASCRIPTCORE_DIR}/parser"
    "${JAVASCRIPTCORE_DIR}/profiler"
    "${JAVASCRIPTCORE_DIR}/runtime"
    "${JAVASCRIPTCORE_DIR}/wtf"
    "${JAVASCRIPTCORE_DIR}/wtf/unicode"
    "${DERIVED_SOURCES_DIR}"
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/streams"
    "${CMAKE_CURRENT_SOURCE_DIR}/streams/qt"
    "${WEBCORE_DIR}/platform/network/blackberry"
)

SET(WEBKIT_SUPPORT_DIR ${WEBCORE_DIR}/../../webkitsupport/blackberry)

LIST(APPEND WebKitSupport_SOURCES
    LocalizeResource.cpp
    OlympiaPlatformFileSystem.cpp
    ObjectAllocator.cpp
    streams/NetworkRequest.cpp
    streams/FilterStream.cpp
    streams/qt/OlympiaNetworkCookieJarQt.cpp
    streams/qt/OlympiaHttpStreamQt.cpp
    OlympiaPlatformTextCodec.cpp
    OlympiaPlatformMisc.cpp
    OutOfMemoryHandlerInterface.cpp
    OlympiaPlatformGeoTracker.cpp
    OlympiaPlatformCookieJar.cpp
    OlympiaPlatformText.cpp
    OlympiaPlatformTimer.cpp
    OlympiaStreamFactory.cpp
    OlympiaPlatformMemory.cpp
    OlympiaPlatformTextBreakIterators.cpp
    OlympiaPlatformIDN.cpp
    OlympiaPlatformAssert.cpp
    OlympiaPlatformClient.cpp
    OlympiaPlatformSettings.cpp
    text/TextGraphicsContextOpenVG.cpp
    text/TextFontFreeType.cpp
    text/TextFontLoaderFreeType.cpp
    text/TextEngineFreeType.cpp
    ${WebKitSupport_MOC_SRCS}
    ${DERIVED_SOURCE_DIR}/OlympiaPlatformTimer.moc
)

source_group(streams "streams/")
source_group(text "text/")


INCLUDE_DIRECTORIES(${WebKit_INCLUDE_DIRECTORIES})
include_directories(${ICU_INCLUDE_DIR})
include_directories(${ICU_INCLUDE_DIR}/unicode)
include_directories(${VG_DIR}/include ${VG_DIR}/include/VG)
include_directories(${EGL_DIR}/include ${EGL_DIR}/include/EGL)

ADD_LIBRARY(${WebKitSupport_LIBRARY_NAME} STATIC ${WebKitSupport_SOURCES})
TARGET_LINK_LIBRARIES(${WebKitSupport_LIBRARY_NAME} ${QT_LIBRARIES} ${optionalLibs})

