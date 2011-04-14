TEMPLATE = lib

DESTDIR = ../lib

CONFIG += debug

CONFIG += shared
DEFINES += EGL_PUB_MAKE_DYNAMIC_LIBRARY

INCLUDEPATH += ..
INCLUDEPATH += ../include

HEADERS += \
          ../EGLConfigImpl.h \
          ../EGLContextImpl.h \
          ../EGLDisplayImpl.h \
          ../EGLGlobal.h \
          ../EGLImpl.h \
          ../EGLNoncopyable.h \
          ../EGLRefCounted.h \
          ../EGLSurfaceImpl.h \
          ../EGLThread.h

SOURCES += \
          ../EGLApi.cpp \
          ../EGLContextImpl.cpp \
          ../EGLDisplayImpl.cpp \
          ../EGLImpl.cpp \
          ../EGLSurfaceImpl.cpp

win32 {
    TARGET = libEGL_AMWin32

    # MSVC and Window SDK
    INCLUDEPATH += $(WIN32_SDK_ROOT)/include
    INCLUDEPATH += $(MSVC_SDK_ROOT)/include

    # Workaround a qmake bug.
    # We use QMAKE_LFLAGS instead of LIBS
    # because LIBS will strip ". When path contains space
    # we must use " to make sure linker work fine.
    QMAKE_LFLAGS += /LIBPATH:\"$(WIN32_SDK_ROOT)/lib\"
    QMAKE_LFLAGS += /LIBPATH:\"$(MSVC_SDK_ROOT)/lib\"

    # OpenVG header
    INCLUDEPATH += ../../windows/openvg/include

    SOURCES += ../win/EGLWin.cpp

    DEF_FILE += \
              ../win/EGLExports.def
    LIBS += -L../../windows/openvg/lib -llibAmanithVG_SRE
    LIBS += -lgdi32 -luser32
}



macx {

    TARGET = EGL_AM
    CONFIG += x86_64
    #DEFINES += AM_OS_MACX
    # OpenVG header
    INCLUDEPATH += ../../macx/openvg/include

    SOURCES += ../macx/EGLWin.cpp
    LIBS += -L../../macx/openvg/lib -lAmanithVG_SRE
}


unix {

    #TARGET = EGL_AM

    # OpenVG header
    INCLUDEPATH += ../../linux/openvg/include

    #SOURCES += ../linux/EGLWin.cpp
    LIBS += -L../../linux/openvg/lib -lAmanithVG_SRE
}
