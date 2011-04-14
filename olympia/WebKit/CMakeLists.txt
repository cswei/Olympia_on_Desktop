
find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})
set(optionalLibs ${optionalLibs} ${JPEG_LIBRARIES})

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
find_package(PNG 1.4.5 EXACT REQUIRED)
else ()
find_package(PNG REQUIRED)
endif ()
include_directories(${PNG_INCLUDE_DIR})
set(optionalLibs ${optionalLibs} ${PNG_LIBRARIES})

find_package(ICU REQUIRED)
include_directories(${ICU_INCLUDE_DIR})
set(optionalLibs ${optionalLibs} ${ICU_LIBRARIES})

find_package(Sqlite REQUIRED)
include_directories(${SQLITE_INCLUDE_DIR})
set(optionalLibs ${optionalLibs} ${SQLITE_LIBRARIES})

find_package(Freetype REQUIRED)
include_directories(${Freetype_INCLUDE_DIR})
set(optionalLibs ${optionalLibs} ${Freetype_LIBRARIES})
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
include_directories(/usr/X11R6/include)
endif ()

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
set(optionalLibs ${optionalLibs} ${LIBXML2_LIBRARIES})

#find_package(Threads REQUIRED)
#include_directories(${THREAD_INCLUDE_DIR})
#set(optionalLibs ${optionalLibs} ${THREAD_LIBRARIES})

find_package(Qt4 REQUIRED)
set(QT_USE_QTNETWORK TRUE)
include(${QT_USE_FILE})

SET(WebKit_INCLUDE_DIRECTORIES
    "${WEBKIT_DIR}"
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
)

SET(WebKit_LIBRARIES
    ${JavaScriptCore_LIBRARY_NAME}
    ${WebCore_LIBRARY_NAME}
    ${WebKitSupport_LIBRARY_NAME}
    ${VG_LIB_NAME}
    EGL_AM
)

link_directories(${VG_DIR}/lib)
link_directories(${CMAKE_BINARY_DIR}/EGL)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
LIST(APPEND WebKit_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../webkitsupport/windows/OlympiaWebkitExport.def    
)

LIST(APPEND WebKit_LIBRARIES
    pthreadVC2
    winmm
)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../webkitsupport/windows/pthread/lib)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    #add_custom_target(${WebKit_LIBRARY_NAME}_PostBuild_for_Mac ALL
    #    install_name_tool -change libz.1.dylib /usr/local/lib/libz.dylib ${CMAKE_BINARY_DIR}/$<
    #    COMMAND install_name_tool -change libAmanithVG_SRE.1.dylib @executable_path/libAmanithVG_SRE.1.dylib ${CMAKE_BINARY_DIR}/$<
    #    COMMAND install_name_tool -change libEGL_AM.1.dylib @executable_path/libEGL_AM.1.dylib ${CMAKE_BINARY_DIR}/$<
    #    COMMAND install_name_tool -change libicuuc.dylib.36 /usr/local/lib/libicuuc.dylib ${CMAKE_BINARY_DIR}/$<
    #    DEPENDS ${WebKit_LIBRARY_NAME}
    #)
endif ()

GENERATE_WEBKIT_VERSION()

INCLUDE_IF_EXISTS(${WEBKIT_DIR}/CMakeLists${PORT}.txt)

ADD_DEFINITIONS(-DBUILD_WEBKIT)
INCLUDE_DIRECTORIES(${WebKit_INCLUDE_DIRECTORIES})

ADD_LIBRARY(${WebKit_LIBRARY_NAME} ${WebKit_LIBRARY_TYPE} ${WebKit_SOURCES})
TARGET_LINK_LIBRARIES(${WebKit_LIBRARY_NAME} ${WebKit_LIBRARIES} ${QT_LIBRARIES} ${optionalLibs})
ADD_TARGET_PROPERTIES(${WebKit_LIBRARY_NAME} LINK_FLAGS ${WebKit_LINK_FLAGS})
TARGET_LINK_LIBRARIES(${WebKit_LIBRARY_NAME} LINK_INTERFACE_LIBRARIES WebKitSupport)

INSTALL(TARGETS ${WebKit_LIBRARY_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin)

