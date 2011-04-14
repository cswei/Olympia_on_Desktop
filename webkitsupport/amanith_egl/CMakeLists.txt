CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(AmanithVG)

add_definitions(-DAM_STANDALONE
           -DAM_MAKE_DYNAMIC_LIBRARY
           -DAM_SRE
           -DAM_EVALUATE
           -DTORCH_VG_SRC
           -DOPENVG_VERSION_1_0_1
           -DAM_MAX_DIRTY_REGIONS_NUMBER=0
           -DAM_LITTLE_ENDIAN
           -DAM_SURFACE_MAX_DIMENSION=2048
           -DAM_SURFACE_BYTE_ORDER_ARGB
           -DAM_PATH_CACHE_SLOTS_COUNT=5
           -DAM_BEZIER_DEGENERATION_THRESHOLD=0.0001f
           -DAM_FLATTENING_DEFAULT_QUALITY=90
           -DAM_MAX_POINTS_TO_FLATTEN_DEGENERATIONS=256
           -DAM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS=10
           -DAM_GRADIENTS_CONICAL_TEXTURE_WIDTH_BITS=6
           -DAM_COLOR_TRANSFORM_SCALE_BIAS_BITS=9
           -DAM_FONT_GLYPH_POOL_CAPACITY=32
           -DAM_CONFIG_FILE
)
include_directories(
           amanithvg/include
           amanithvg/include/curves
           amanithvg/include/geometry
           amanithvg/include/geometry/triangulator
           amanithvg/include/openvg
           amanithvg/include/openvg/brew
           amanithvg/include/openvg/rendering
           amanithvg/include/openvg/rendering/fillers
           amanithvg/include/openvg/rendering/gl
           amanithvg/include/openvg/rendering/gl/gle
           amanithvg/include/openvg/rendering/gl/gls
           amanithvg/include/openvg/rendering/rasterizer
           amanithvg/include/openvg/rendering/sre
           amanithvg/include/openvg/symbian
           amanithvg/include/utils
           amanithvg/include/VG
)

set(AmanithVG_SRE_SRCS
          amanithvg/src/curves/bezier.c
          amanithvg/src/curves/ellipse.c
          amanithvg/src/geometry/aabox.c
          amanithvg/src/geometry/intersect.c
          amanithvg/src/geometry/matrix.c
          amanithvg/src/openvg/vg_priv.c
          amanithvg/src/openvg/vgcontext.c
          amanithvg/src/openvg/vgconversions.c
          amanithvg/src/openvg/vgext.c
          amanithvg/src/openvg/vgfilters.c
          amanithvg/src/openvg/vgfont.c
          amanithvg/src/openvg/vggetset.c
          amanithvg/src/openvg/vgimage.c
          amanithvg/src/openvg/vgmask.c
          amanithvg/src/openvg/vgmatrix.c
          amanithvg/src/openvg/vgpaint.c
          amanithvg/src/openvg/vgpath.c
          amanithvg/src/openvg/vgtexture.c
          amanithvg/src/openvg/vgu.c
          amanithvg/src/openvg/brew/brewcontext.c
          amanithvg/src/openvg/rendering/vgcompositing.c
          amanithvg/src/openvg/rendering/vgdrawingsurface.c
          amanithvg/src/openvg/rendering/vggradients.c
          amanithvg/src/openvg/rendering/vgprimitives.c
          amanithvg/src/openvg/rendering/vgscissors.c
          amanithvg/src/openvg/rendering/vgstroke.c
          amanithvg/src/openvg/rendering/rasterizer/rasterizer.c
          amanithvg/src/openvg/rendering/rasterizer/rasterizer_better.c
          amanithvg/src/openvg/rendering/rasterizer/rasterizer_common.c
          amanithvg/src/openvg/rendering/rasterizer/rasterizer_faster.c
          amanithvg/src/openvg/rendering/rasterizer/rasterizer_noaa.c
          amanithvg/src/openvg/rendering/fillers/fillers.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_Additive.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_ColorBurn.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_ColorDodge.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_Darken.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_Difference.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_DstAtop.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_DstIn.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_DstOut.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_DstOver.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_Exclusion.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_HardLight.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_Lighten.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_Multiply.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_Overlay.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_Screen.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_SoftLight.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_Src.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_SrcAtop.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_SrcIn.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_SrcOut.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_SrcOver.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_table.c
          amanithvg/src/openvg/rendering/fillers/full/fillers_Xor.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_Additive.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_ColorBurn.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_ColorDodge.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_Darken.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_Difference.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_DstAtop.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_DstIn.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_DstOut.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_DstOver.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_Exclusion.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_HardLight.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_Lighten.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_Multiply.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_Overlay.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_Screen.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_SoftLight.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_Src.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_SrcAtop.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_SrcIn.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_SrcOut.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_SrcOver.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_table.c
          amanithvg/src/openvg/rendering/fillers/full/img_fillers_Xor.c
          amanithvg/src/openvg/rendering/sre/srecompositing.c
          amanithvg/src/openvg/rendering/sre/sredrawingsurface.c
          amanithvg/src/openvg/rendering/sre/sremask.c
          amanithvg/src/openvg/rendering/sre/sreprimitives.c
          amanithvg/src/utils/globals.c
          amanithvg/src/utils/int64_math.c
          amanithvg/src/utils/integration.c
          amanithvg/src/utils/mathematics.c
          amanithvg/src/utils/stdlib_abstraction.c
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(-DAM_OS_WIN32)
    list(APPEND AmanithVG_SRE_SRCS
        amanithvg/src/dllmain.c
    )
    set(VG_INSTALL_DEST ${CMAKE_SOURCE_DIR}/../webkitsupport/windows/openvg/lib)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-DAM_OS_LINUX)
    set(VG_INSTALL_DEST ${CMAKE_SOURCE_DIR}/../webkitsupport/linux/openvg/lib)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-DAM_OS_MACX)
    set(VG_INSTALL_DEST ${CMAKE_SOURCE_DIR}/../webkitsupport/macx/openvg/lib)
else()
    message(FATAL_ERROR "Unknown OS: " ${CMAKE_SYSTEM_NAME})
endif()

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
    set(VG_LIB AmanithVG_SRE64)
else ()
    set(VG_LIB AmanithVG_SRE)
endif ()

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
#set(CMAKE_BUILD_TYPE Release)

add_library(${VG_LIB} SHARED ${AmanithVG_SRE_SRCS})

install(TARGETS ${VG_LIB} DESTINATION ${VG_INSTALL_DEST})

