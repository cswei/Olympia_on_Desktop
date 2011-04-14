
set(EGL_AM_HDRS
          EGLConfigImpl.h
          EGLContextImpl.h
          EGLDisplayImpl.h
          EGLGlobal.h
          EGLImpl.h
          EGLNoncopyable.h
          EGLRefCounted.h
          EGLSurfaceImpl.h
          EGLThread.h
)
set(EGL_AM_SRCS
          EGLApi.cpp
          EGLContextImpl.cpp
          EGLDisplayImpl.cpp
          EGLImpl.cpp
          EGLSurfaceImpl.cpp
)


add_definitions(-DEGL_PUB_MAKE_DYNAMIC_LIBRARY)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CMAKE_LINK_DEF_FILE_FLAG win/EGLExports.def)
    list(APPEND EGL_AM_SRCS
        win/EGLWin.cpp
    )
    #set (VG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../windows/openvg)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    list(APPEND EGL_AM_SRCS
        macx/EGLWin.cpp
    )
    #set (VG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../linux/openvg)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    list(APPEND EGL_AM_SRCS
        macx/EGLWin.cpp
    )
    #set (VG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../macx/openvg)
else()
    message(FATAL "Unknown OS: " ${CMAKE_SYSTEM_NAME})
endif()

include_directories(BEFORE
    .
    ./include
    ${VG_DIR}/include
)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

link_directories(${VG_DIR}/lib)
add_library(EGL_AM SHARED ${EGL_AM_HDRS} ${EGL_AM_SRCS})
target_link_libraries(EGL_AM ${VG_LIB_NAME})

install(TARGETS EGL_AM DESTINATION ${CMAKE_BINARY_DIR}/bin)

