SET(BlackBerry_STYLE_SHEET
  ${WEBCORE_DIR}/css/themeBlackBerry.css
)
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET(PREPROCESSOR --preprocessor="gcc -E -P -x c++")

    SET(WEBKIT_BUILDINFO_GENERATOR perl ${CMAKE_SOURCE_DIR}/WebKitTools/Scripts/generate-buildinfo)
ELSE()
    SET(PREPROCESSOR --preprocessor="")
    SET(WEBKIT_BUILDINFO_GENERATOR ${CMAKE_SOURCE_DIR}/WebKitTools/Scripts/generate-buildinfo)
ENDIF()

MACRO(GENERATE_BUILD_INFO)
    ADD_CUSTOM_COMMAND(
        OUTPUT ${DERIVED_SOURCES_DIR}/BuildInformation.cpp
        COMMAND ${WEBKIT_BUILDINFO_GENERATOR} ${DERIVED_SOURCES_DIR}/BuildInformation.cpp
    )
    ADD_CUSTOM_TARGET(BuildInformation ALL DEPENDS ${DERIVED_SOURCES_DIR}/BuildInformation.cpp)
ENDMACRO()
