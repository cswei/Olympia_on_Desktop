find_package(Qt4 COMPONENTS QtCore QtGui QtXml QtSql QtNetwork REQUIRED)
include(${QT_USE_FILE})

SET(DERIVED_SOURCE_DIR ${CMAKE_BINARY_DIR}/DerivedSources)

SET(BROWSER_INCLUDE_DIRECTORIES
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
    "${JAVASCRIPTCORE_DIR}/wtf/text"
    "${DERIVED_SOURCES_DIR}"
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/AddressBar"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/StatusBar"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/MainWindow"
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/Tabs"
    "${WEBCORE_DIR}/platform/network/blackberry"
    "${VG_DIR}/include"
    "${VG_DIR}/include/VG"
    "${EGL_DIR}/include"
    "${EGL_DIR}/include/EGL"
)



LIST(APPEND BROWSER_SOURCES
          BrowserWidgetsFactory.cpp
          ConfigWidget.cpp
          Launcher.cpp
          main.cpp
          NetworkQt.cpp
          WebViewQt.cpp
          WebViewEventHandler.cpp
          TabbedView.cpp
          TabListView.cpp
          TabThumbButton.cpp
          History.cpp
          ./widgets/MainWindow/MainWindowAbstract.cpp
          ./widgets/MainWindow/OlympiaMobileMainWindow.cpp
          ./widgets/MainWindow/OlympiaDesktopMainWindow.cpp
          ./widgets/AddressBar/AddressBar.cpp
          ./widgets/AddressBar/OlympiaMobileAddressBar.cpp
          ./widgets/AddressBar/MatchedHistView.cpp
          ./widgets/StatusBar/StatusBar.cpp
          ./widgets/StatusBar/OlympiaMobileStatusBar.cpp
          ./widgets/Tabs/Tabs.cpp
          ./widgets/Tabs/OlympiaDesktopTabs.cpp
          ./widgets/Tabs/OlympiaMobileTabs.cpp
)

set(BROWSER_MOC_HEADERS
    BrowserWidgetsFactory.h
    ConfigWidget.h
    Launcher.h
    NetworkQt.h
    WebViewQt.h
    WebViewEventHandler.h
    TabbedView.h
    TabListView.h
    TabThumbButton.h
    History.h
    ./widgets/MainWindow/MainWindowAbstract.h
    ./widgets/MainWindow/OlympiaMobileMainWindow.h
    ./widgets/MainWindow/OlympiaDesktopMainWindow.h
    ./widgets/AddressBar/AddressBar.h
    ./widgets/AddressBar/MatchedHistView.h
    ./widgets/AddressBar/OlympiaMobileAddressBar.h
    ./widgets/StatusBar/StatusBar.h
    ./widgets/StatusBar/OlympiaMobileStatusBar.h
    ./widgets/Tabs/Tabs.h
    ./widgets/Tabs/OlympiaDesktopTabs.h
    ./widgets/Tabs/OlympiaMobileTabs.h
)

source_group(widgets\\MainWindow "MainWindow/")
source_group(widgets\\AddressBar "AddressBar/")
source_group(widgets\\StatusBar "StatusBar/")
source_group(widgets\\Tabs "Tabs/")

QT4_WRAP_CPP(BROWSER_MOC_SRCS ${BROWSER_MOC_HEADERS})
QT4_WRAP_UI(BROWSER_UIC_SRCS ConfigWidget.ui)

list(APPEND BROWSER_SOURCES ${BROWSER_MOC_SRCS} ${BROWSER_UIC_SRCS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${BROWSER_INCLUDE_DIRECTORIES})
include_directories(${QT_INCLUDE_DIR})

set(BROWSER_LIBRARIES
        ${WebKit_LIBRARY_NAME}
        ${VG_LIB_NAME}
        EGL_AM
)

link_directories(${VG_DIR}/lib)

if (APPLE OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
    #add_custom_target(browser_PostBuild_for_Mac ALL
    #    install_name_tool -change ${CMAKE_BINARY_DIR}/WebKit/libwebkit.dylib @executable_path/libwebkit.dylib ${CMAKE_BINARY_DIR}/$<
    #    COMMAND install_name_tool -change libz.1.dylib /usr/local/lib/libz.dylib ${CMAKE_BINARY_DIR}/$<
    #    COMMAND install_name_tool -change libicuuc.dylib.36 /usr/local/lib/libicuuc.dylib ${CMAKE_BINARY_DIR}/$<
    #    DEPENDS browser
    #)
else()

endif()


ADD_EXECUTABLE(browser ${BROWSER_SOURCES})
TARGET_LINK_LIBRARIES(browser ${BROWSER_LIBRARIES} ${QT_LIBRARIES} ${optionalLibs})

INSTALL(TARGETS browser DESTINATION ${CMAKE_BINARY_DIR}/bin)
install(DIRECTORY config DESTINATION ${CMAKE_BINARY_DIR}/bin)
install(DIRECTORY images DESTINATION ${CMAKE_BINARY_DIR}/bin)
install(FILES ${VG_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${VG_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
    DESTINATION ${CMAKE_BINARY_DIR}/bin)

if (APPLE OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
#install(FILES ${CMAKE_SOURCE_DIR}/../amanith_egl/qmake/bin/libAmanithVG_SRE.1.0.0.dylib DESTINATION ${CMAKE_BINARY_DIR}/bin RENAME libAmanithVG_SRE.1.dylib)
#install(FILES ${CMAKE_SOURCE_DIR}/../webkitsupport/egl/lib/libEGL_AM.1.0.0.dylib DESTINATION ${CMAKE_BINARY_DIR}/bin RENAME libEGL_AM.1.dylib)
endif ()

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
INSTALL(FILES ${CMAKE_SOURCE_DIR}/../webkitsupport/windows/libxml2/bin/libxml2.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/../webkitsupport/windows/jpeg/bin/jpeg62.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/../webkitsupport/windows/png/bin/libpng12.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/../webkitsupport/windows/pthread/lib/pthreadvc2.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/../webkitsupport/windows/iconv/bin/iconv.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/../webkitsupport/windows/zlib/bin/zlib1.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/../webkitsupport/windows/sqlite3/lib/sqlite3.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/../webkitsupport/windows/icu/bin/icuuc36.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/../webkitsupport/windows/icu/bin/icudt36.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/../webkitsupport/windows/msvcr71.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
ENDIF()
