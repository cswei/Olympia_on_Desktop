CONFIG += debug

INCLUDEPATH += .
INCLUDEPATH += ../../include

HEADERS += \
          tiger.h

SOURCES += \
          tiger.c \
          main.c

win32 {

    # MSVC and Window SDK
    INCLUDEPATH += $(WIN32_SDK_ROOT)/include
    INCLUDEPATH += $(MSVC_SDK_ROOT)/include

    # Workaround a qmake bug.
    # We use QMAKE_LFLAGS instead of LIBS
    # because LIBS will strip ". When path contains space
    # we must use " to make sure linker work fine.
    QMAKE_LFLAGS += /LIBPATH:\"$(WIN32_SDK_ROOT)/lib\"
    QMAKE_LFLAGS += /LIBPATH:\"$(MSVC_SDK_ROOT)/lib\"

    INCLUDEPATH += ../../../windows/openvg/include

    LIBS += -L../../../windows/openvg/lib -llibAmanithVG_SRE
    LIBS += -L../../lib -llibEGL_AMWin32
    LIBS += -lgdi32 -luser32
}

