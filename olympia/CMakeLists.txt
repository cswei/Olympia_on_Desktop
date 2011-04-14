CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(WebKit)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

SET(JAVASCRIPTCORE_DIR "${CMAKE_SOURCE_DIR}/JavaScriptCore")
SET(WEBCORE_DIR "${CMAKE_SOURCE_DIR}/WebCore")
SET(WEBKIT_DIR "${CMAKE_SOURCE_DIR}/WebKit")
SET(WEBKITTOOLS_DIR "${CMAKE_SOURCE_DIR}/WebKitTools")
SET(DERIVED_SOURCES_DIR "${CMAKE_BINARY_DIR}/DerivedSources")

INCLUDE(WebKitMacros)
INCLUDE(WebKitGenerators)
INCLUDE(WebKitHelpers)
INCLUDE(WebKitFeatures)

# -----------------------------------------------------------------------------
# Determine which port will be built
# -----------------------------------------------------------------------------
SET(ALL_PORTS "Efl BlackBerry")
OPTION(PORT "choose which WebKit port to build (one of ${ALL_PORTS})" "NOPORT")

# -----------------------------------------------------------------------------
# Find common packages (used by all ports)
# -----------------------------------------------------------------------------
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
endif ()
FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)
FIND_PACKAGE(Perl REQUIRED)
FIND_PACKAGE(PythonInterp REQUIRED)

# -----------------------------------------------------------------------------
# Determine the target processor
# FIXME: Why are we even setting WTF_CPU_FOO?
# -----------------------------------------------------------------------------
IF (CMAKE_SYSTEM_PROCESSOR MATCHES "(arm|xscale)")
    SET(WTF_CPU_ARM 1)
    add_definitions(-DWTF_CPU_ARM=1)
ELSEIF (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)")
    SET(WTF_CPU_X86_64 1)
    add_definitions(-DWTF_CPU_X86_64=1)
ELSEIF (CMAKE_SYSTEM_PROCESSOR MATCHES "(i[3-6]86|x86)")
    SET(WTF_CPU_X86 1)
    add_definitions(-DWTF_CPU_X86=1)
ELSE ()
    MESSAGE(FATAL_ERROR "Unknown CPU '${CMAKE_SYSTEM_PROCESSOR}'")
ENDIF ()

# -----------------------------------------------------------------------------
# Determine the operating system
# FIXME: Why are we even setting WTF_OS_FOO?
# -----------------------------------------------------------------------------
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    SET(WTF_OS_UNIX 1)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET(WTF_OS_WINDOWS 1)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "RVCT")
    # FIXME: Better way of determining Olympia
    SET(WTF_OS_OLYMPIA 1)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    SET(WTF_OS_MAC_OS_X 1)
    #SET(CMAKE_OSX_ARCHITECTURES x86_64)
ELSE ()
    MESSAGE(WARNING "Unknown OS '${CMAKE_SYSTEM_NAME}'")
ENDIF ()

# -----------------------------------------------------------------------------
# Default target names (can be overrriden in Options${PORT}.cmake file)
# -----------------------------------------------------------------------------
SET(JSC_EXECUTABLE_NAME JSC)
SET(WTF_LIBRARY_NAME WTF)
SET(JavaScriptCore_LIBRARY_NAME JavaScriptCore)
SET(WebCore_LIBRARY_NAME WebCore)
SET(WebKit_LIBRARY_NAME WebKit)
SET(WebKitSupport_LIBRARY_NAME WebKitSupport)

# -----------------------------------------------------------------------------
# Default library types (can be overrriden in Options${PORT}.cmake file)
# -----------------------------------------------------------------------------
SET(WTF_LIBRARY_TYPE STATIC)
SET(JavaScriptCore_LIBRARY_TYPE STATIC)
SET(WebCore_LIBRARY_TYPE STATIC)
SET(WebKit_LIBRARY_TYPE SHARED)

# -----------------------------------------------------------------------------
# Port-specific options
# -----------------------------------------------------------------------------
INCLUDE(OptionsCommon)
INCLUDE(Options${PORT})

# -----------------------------------------------------------------------------
# Add module directories
# -----------------------------------------------------------------------------
ADD_SUBDIRECTORY(JavaScriptCore)
ADD_SUBDIRECTORY(WebCore)
ADD_SUBDIRECTORY(WebKit)
ADD_SUBDIRECTORY(../webkitsupport/blackberry WebKitSupport)
ADD_SUBDIRECTORY(../webkitsupport/egl EGL)
ADD_SUBDIRECTORY(../Browser Browser)

ADD_DEPENDENCIES(${JavaScriptCore_LIBRARY_NAME} ${WTF_LIBRARY_NAME})
ADD_DEPENDENCIES(${WebCore_LIBRARY_NAME} ${JavaScriptCore_LIBRARY_NAME} WebKitVersion)
ADD_DEPENDENCIES(${WebKitSupport_LIBRARY_NAME} ${WebCore_LIBRARY_NAME})
ADD_DEPENDENCIES(${WebKit_LIBRARY_NAME} ${WebKitSupport_LIBRARY_NAME})
ADD_DEPENDENCIES(browser ${WebKit_LIBRARY_NAME} EGL_AM)

# -----------------------------------------------------------------------------
# Set compiler flags for all targets
# -----------------------------------------------------------------------------
WEBKIT_SET_EXTRA_COMPILER_FLAGS(${WTF_LIBRARY_NAME})
WEBKIT_SET_EXTRA_COMPILER_FLAGS(${JavaScriptCore_LIBRARY_NAME})
WEBKIT_SET_EXTRA_COMPILER_FLAGS(${WebCore_LIBRARY_NAME})
WEBKIT_SET_EXTRA_COMPILER_FLAGS(${WebKit_LIBRARY_NAME})

# -----------------------------------------------------------------------------
# Create the configuration file
# -----------------------------------------------------------------------------
CONFIGURE_FILE(cmakeconfig.h.cmake ${CMAKE_BINARY_DIR}/cmakeconfig.h @ONLY)

# -----------------------------------------------------------------------------
# Prints a summary of all enabled/disabled features
# -----------------------------------------------------------------------------
WEBKIT_PRINT_FEATURES()
