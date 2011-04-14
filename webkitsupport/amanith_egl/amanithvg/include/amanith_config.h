/****************************************************************************
** Copyright (C) 2004-2009 Mazatech S.r.l. All rights reserved.
**
** This file is part of AmanithVG software, an OpenVG implementation.
** This file is strictly confidential under the signed Mazatech Software
** Non-disclosure agreement and it's provided according to the signed
** Mazatech Software licensing agreement.
**
** Khronos and OpenVG are trademarks of The Khronos Group Inc.
** OpenGL is a registered trademark and OpenGL ES is a trademark of
** Silicon Graphics, Inc.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** For any information, please contact info@mazatech.com
**
****************************************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H

/*!
	\file config.h
	\brief Configuration defines, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

/*
	- Library type:
		AM_STANDALONE

	- Library format:
		AM_MAKE_STATIC_LIBRARY / AM_MAKE_DYNAMIC_LIBRARY

	- Engine:
		AM_SRE / AM_GLE / AM_GLS

	- Evaluation version:
		AM_EVALUATE

	- Target operating system
		AM_OS_WIN32 / AM_OS_WIN64 / AM_OS_WINCE / AM_OS_MAC9 / AM_OS_MACX / AM_OS_LINUX / AM_OS_IRIX / AM_OS_HPUX
		AM_OS_SOLARIS / AM_OS_AIX / AM_OS_FREEBSD / AM_OS_NETBSD / AM_OS_OPENBSD / AM_OS_BREW / AM_OS_SYMBIAN / AM_OS_QNX
		AM_OS_AMIGAOS

	- Target machine endianess:
		AM_BIG_ENDIAN / AM_LITTLE_ENDIAN

	- Drawing surface dimension:
		AM_SURFACE_MAX_DIMENSION

	- Drawing surface pixel format byte order:
		AM_SURFACE_BYTE_ORDER_RGBA / AM_SURFACE_BYTE_ORDER_ARGB / AM_SURFACE_BYTE_ORDER_BGRA / AM_SURFACE_BYTE_ORDER_ABGR

	- Maximum number of dirty regions that can be handled (SRE only)
		AM_MAX_DIRTY_REGIONS_NUMBER

	- Normalized object space bit precision:
		AM_NORMALIZED_OBJSPACE_16BIT / AM_NORMALIZED_OBJSPACE_21BIT

	- Path cache slots:
		AM_PATH_CACHE_SLOTS_COUNT

	- Bezier degeneration threshold:
		AM_BEZIER_DEGENERATION_THRESHOLD

	- Default flattening quality:
		AM_FLATTENING_DEFAULT_QUALITY

	- Maximum number of points that can be generated during the flattening of degenerated traits:
		AM_MAX_POINTS_TO_FLATTEN_DEGENERATIONS

	- Linear/radial gradients texture width bits:
		AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS

	- Conical gradients texture dimension bits:
		AM_GRADIENTS_CONICAL_TEXTURE_WIDTH_BITS
	
	- Mesh vertex pool capacity (GLE/GLS specific):
		AM_MESH_VERTEX_POOL_CAPACITY

	- Mesh edge pool capacity (GLE/GLS specific):
		AM_MESH_EDGE_POOL_CAPACITY

	- Paths pool capacity
		AM_PATHS_POOL_CAPACITY

	- Paints pool capacity
		AM_PAINTS_POOL_CAPACITY

	- Font glyph pool capacity (OpenVG 1.1 or greater)
		AM_FONT_GLYPH_POOL_CAPACITY

	- The number of OpenVG calls (handles creation/destruction and drawing functions) to be done before to recover/retrieve unused memory.
		AM_OPENVG_CALLS_BEFORE_MEMORY_RECOVERY

	- External config file:
		AM_CONFIG_FILE
*/

#if defined ( RIM_VG_SRC ) || defined ( TORCH_VG_SRC )

#ifndef AM_PATHS_POOL_CAPACITY
    #define AM_PATHS_POOL_CAPACITY                                 32
#endif

#ifndef AM_PAINTS_POOL_CAPACITY
    #define AM_PAINTS_POOL_CAPACITY                                32
#endif

#ifndef AM_FONT_GLYPH_POOL_CAPACITY 
    #define AM_FONT_GLYPH_POOL_CAPACITY                            32 
#endif

#ifndef AM_OPENVG_CALLS_BEFORE_MEMORY_RECOVERY
    #define AM_OPENVG_CALLS_BEFORE_MEMORY_RECOVERY                 6400
#endif

#ifndef AM_BEZIER_DEGENERATION_THRESHOLD
    #define AM_BEZIER_DEGENERATION_THRESHOLD                       0.0001f
#endif

#endif // #if defined ( RIM_VG_SRC )  

// engine type
#if defined(AM_SRE)
	#if defined(AM_GLE) || defined(AM_GLS)
		#error Please define only one of: AM_SRE, AM_GLE, AM_GLS.
	#endif
	#undef AM_OPENGL
	#undef AM_OPENGL_ES
#else
	#if defined(AM_OPENGL) && defined(AM_OPENGL_ES)
		#error Please define only one of: AM_OPENGL, AM_OPENGL_ES.
	#endif

	#if defined(AM_GLE)
		#if defined(AM_SRE) || defined(AM_GLS)
			#error Please define only one of: AM_SRE, AM_GLE, AM_GLS.
		#endif
	#elif defined(AM_GLS)
		#if defined(AM_SRE) || defined(AM_GLE)
			#error Please define only one of: AM_SRE, AM_GLE, AM_GLS.
		#endif
	#endif
	
	#if !defined(AM_OPENGL) && !defined(AM_OPENGL_ES)
		#error Please define OpenGL or OpenGL ES platform (AM_OPENGL / AM_OPENGL_ES).
	#endif
#endif

// check library type
#if !defined(AM_STANDALONE)
	#if defined(AM_GLE) || defined(AM_GLS)
		#error Non standalone (EGL dependent) AmanithVG not yet supported in GLE / GLS.
	#endif
#endif

#if defined(AM_FIXED_POINT_PIPELINE)
	#if !defined(AM_SRE)
		#error AM_FIXED_POINT_PIPELINE supported in AmanithVG SRE only.
	#endif
#endif

#if defined(AM_LITE_PROFILE)
	#if !defined(AM_SRE) || !defined(AM_STANDALONE)
		#error AM_LITE_PROFILE supported in AmanithVG SRE + Standalone only.
	#endif
#endif

// operating system
#if !defined(AM_OS_WIN32) && !defined(AM_OS_WIN64) && !defined(AM_OS_WINCE) && !defined(AM_OS_MAC9) && !defined(AM_OS_MACX) && \
	!defined(AM_OS_LINUX) && !defined(AM_OS_IRIX) && !defined(AM_OS_HPUX) && !defined(AM_OS_SOLARIS) && !defined(AM_OS_AIX) && \
	!defined(AM_OS_FREEBSD) && !defined(AM_OS_NETBSD) && !defined(AM_OS_OPENBSD) && \
	!defined(AM_OS_BREW) && !defined(AM_OS_SYMBIAN) &&!defined(AM_OS_QNX) && !defined(AM_OS_AMIGAOS)
	#error Target operating system must be defined!
#endif

// endianess
#if defined(AM_BIG_ENDIAN) && defined(AM_LITTLE_ENDIAN)
	#error Please define only one of AM_BIG_ENDIAN, AM_LITTLE_ENDIAN.
#endif

#if !defined(AM_BIG_ENDIAN) && !defined(AM_LITTLE_ENDIAN)
	#if defined(_MSC_VER)
		#pragma message("Target machine endianess has not been specified, AM_LITTLE_ENDIAN is used as default.")
	#else
		#warning Target machine endianess has not been specified, AM_LITTLE_ENDIAN is used as default.
	#endif
	#define AM_LITTLE_ENDIAN
#endif

// rasterizer
#if !defined(AM_SURFACE_MAX_DIMENSION)
	#if defined(_MSC_VER)
		#pragma message("Maximum dimension (in pixels) of the internal drawing surface has not been specified, 2048 is used as default.")
	#else
		#warning Maximum dimension (in pixels) of the internal drawing surface has not been specified, 2048 is used as default.
	#endif
	// Maximum dimension of the internal drawing surface.
	#define AM_SURFACE_MAX_DIMENSION						2048
#endif

// drawing surface
#if defined(AM_SURFACE_BYTE_ORDER_RGBA)
	#if defined(AM_SURFACE_BYTE_ORDER_ARGB) || defined(AM_SURFACE_BYTE_ORDER_BGRA) || defined(AM_SURFACE_BYTE_ORDER_ABGR)
		#error Please define only one of AM_SURFACE_BYTE_ORDER_XXXX.
	#endif
#elif defined(AM_SURFACE_BYTE_ORDER_ARGB)
	#if defined(AM_SURFACE_BYTE_ORDER_RGBA) || defined(AM_SURFACE_BYTE_ORDER_BGRA) || defined(AM_SURFACE_BYTE_ORDER_ABGR)
		#error Please define only one of AM_SURFACE_BYTE_ORDER_XXXX.
	#endif
#elif defined(AM_SURFACE_BYTE_ORDER_BGRA)
	#if defined(AM_SURFACE_BYTE_ORDER_RGBA) || defined(AM_SURFACE_BYTE_ORDER_ARGB) || defined(AM_SURFACE_BYTE_ORDER_ABGR)
		#error Please define only one of AM_SURFACE_BYTE_ORDER_XXXX.
	#endif
#elif defined(AM_SURFACE_BYTE_ORDER_ABGR)
	#if defined(AM_SURFACE_BYTE_ORDER_RGBA) || defined(AM_SURFACE_BYTE_ORDER_ARGB) || defined(AM_SURFACE_BYTE_ORDER_BGRA)
		#error Please define only one of AM_SURFACE_BYTE_ORDER_XXXX.
	#endif
#endif

#if !defined(AM_SURFACE_BYTE_ORDER_RGBA) && !defined(AM_SURFACE_BYTE_ORDER_ARGB) && !defined(AM_SURFACE_BYTE_ORDER_BGRA) && !defined(AM_SURFACE_BYTE_ORDER_ABGR)
	#if defined(_MSC_VER)
		#pragma message("Byte order of the internal drawing surface pixel format has not been specified, AM_SURFACE_BYTE_ORDER_ARGB is used as default.")
	#else
		#warning Byte order of the internal drawing surface pixel format has not been specified, AM_SURFACE_BYTE_ORDER_ARGB is used as default.
	#endif
	// Internal drawing surface byte order.
	#define AM_SURFACE_BYTE_ORDER_ARGB
#endif

#if defined(AM_SRE)
	// Maximum number of dirty regions that can be handled
	#if !defined(AM_MAX_DIRTY_REGIONS_NUMBER)
		#if defined(_MSC_VER)
			#pragma message("Maximum number of dirty regions has not been specified, 1024 is used as default.")
		#else
			#warning Maximum number of dirty regions has not been specified, 1024 is used as default.
		#endif
		#define AM_MAX_DIRTY_REGIONS_NUMBER					1024
	#endif
#endif

// triangulator object space
#if defined(AM_NORMALIZED_OBJSPACE_16BIT) && defined(AM_NORMALIZED_OBJSPACE_21BIT)
	#error Please define only one of AM_NORMALIZED_OBJSPACE_16BIT, AM_NORMALIZED_OBJSPACE_21BIT.
#endif

#if defined(AM_GLE) || defined(AM_GLS)
	#if !defined(AM_NORMALIZED_OBJSPACE_16BIT) && !defined(AM_NORMALIZED_OBJSPACE_21BIT)
		#if defined(_MSC_VER)
			#pragma message("Fixed point bit precision used to represent normalized object space coordinates has not been specified, AM_NORMALIZED_OBJSPACE_21BIT is used as default.")
		#else
			#warning "Fixed point bit precision used to represent normalized object space coordinates has not been specified, AM_NORMALIZED_OBJSPACE_21BIT is used as default."
		#endif
		// Fixed point bit precision used to represent normalized object space coordinates.
		#define AM_NORMALIZED_OBJSPACE_21BIT
	#endif
#endif

#if defined(AM_OPENGL_ES) && defined(AM_USE_GL_FIXED_INTERFACE) && defined(AM_NORMALIZED_OBJSPACE_21BIT)
	#if defined(_MSC_VER)
		#pragma message("OpenGL ES using fixed interface is supported with 16 bit normalized object space only, AM_NORMALIZED_OBJSPACE_16BIT will be forced.")
	#else
		#warning "OpenGL ES using fixed interface is supported with 16 bit normalized object space only, AM_NORMALIZED_OBJSPACE_16BIT will be forced."
	#endif
	#undef AM_NORMALIZED_OBJSPACE_21BIT
	#define AM_NORMALIZED_OBJSPACE_16BIT
#endif

// paths
#if !defined(AM_PATH_CACHE_SLOTS_COUNT)
	#if defined(_MSC_VER)
		#pragma message("Number of cache slots, used to store fill and stroke polygons/triangles of each path, has not been specified, 5 is used as default.")
	#else
		#warning Number of cache slots, used to store fill and stroke polygons/triangles of each path, has not been specified, 5 is used as default.
	#endif
	// Number of cache slots for each path.
	#define AM_PATH_CACHE_SLOTS_COUNT						5
#endif

// Bezier
#if !defined(AM_BEZIER_DEGENERATION_THRESHOLD)
	#if defined(_MSC_VER)
		#pragma message("The threshold (maximum bounding box dimension) under which a Bezier curve is considered degenerated has not been specified, 0.0001f is used as default.")
	#else
    #if !defined ( RIM_VG_SRC )  
        //suppress this warning as we cannot seem to add AM_BEZIER_DEGENERATION_THRESHOLD into product.xml as it is a floating
        //point value
		#warning The threshold (maximum bounding box dimension) under which a Bezier curve is considered degenerated has not been specified, 0.0001f is used as default.
	#endif
    #endif
	// Threshold under which a Bezier curve is considered degenerated.
	#define AM_BEZIER_DEGENERATION_THRESHOLD				0.0001f
#endif

// flattening
#if !defined(AM_FLATTENING_DEFAULT_QUALITY)
	#if defined(_MSC_VER)
		#pragma message("The default quality level (0-100) used to flatten curves has not been specified, 90 is used as default.")
	#else
		#warning The default quality level (0-100) used to flatten curves has not been specified, 90 is used as default.
	#endif
	#define AM_FLATTENING_DEFAULT_QUALITY					90
#endif

#if !defined(AM_MAX_POINTS_TO_FLATTEN_DEGENERATIONS)
	#if defined(_MSC_VER)
		#pragma message("Maximum number of points that can be generated during the flattening of degenerated traits has not been specified, 256 is used as default.")
	#else
		#warning Maximum number of points that can be generated during the flattening of degenerated traits has not been specified, 256 is used as default.
	#endif
	// Maximum number of points that can be generated during the flattening of degenerated traits.
	#define AM_MAX_POINTS_TO_FLATTEN_DEGENERATIONS			256
#endif

// gradients
#if defined(AM_GLE) || defined(AM_GLS)
	// this is just an hint for the maximum number of bits useable for linear/radial grandient textures.
	// AmanithVG adapts it taking care of the maximum OpenGL texture size.
	#if !defined(AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS)
		#if defined(_MSC_VER)
			#pragma message("Number of bits for linear/radial gradients texture width has not been specified, 13 (2^13 = 8192 pixels) is used as default.")
		#else
			#warning Number of bits for linear/radial gradients texture width has not been specified, 13 (2^13 = 8192 pixels) is used as default.
		#endif
		// Number of bits such as (1 << AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS) = width of texture
		// used by linear and radial gradients.
		#define AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS	13
	#else
		#if AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS > 13
			#error AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS must be less or equal than 13.
		#endif
	#endif
#else
	#if !defined(AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS)
		#if defined(_MSC_VER)
			#pragma message("Number of bits for linear/radial gradients texture width has not been specified, 10 (2^10 = 1024 pixels) is used as default.")
		#else
			#warning Number of bits for linear/radial gradients texture width has not been specified, 10 (2^10 = 1024 pixels) is used as default.
		#endif
		// Number of bits such as (1 << AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS) = width of texture
		// used by linear and radial gradients.
		#define AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS	10
	#else
		#if AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS > 10
			#error AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS must be less or equal than 10.
		#endif
	#endif
#endif

#if !defined(AM_GRADIENTS_CONICAL_TEXTURE_WIDTH_BITS)
	#if defined(_MSC_VER)
		#pragma message("Number of bits for conical gradients texture dimension has not been specified, 6 (2^6 = 64 pixels) is used as default.")
	#else
		#warning Number of bits for conical gradients texture dimension has not been specified, 6 (2^6 = 64 pixels) is used as default.
	#endif
	// Number of bits such as (1 << AM_GRADIENTS_CONICAL_TEXTURE_WIDTH_BITS) = dimensions of texture
	// used by conical gradients.
	#define AM_GRADIENTS_CONICAL_TEXTURE_WIDTH_BITS			6
#endif

#if defined(AM_GLE) || defined(AM_GLS)
// triangulator
#if !defined(AM_MESH_VERTEX_POOL_CAPACITY)
	#if defined(_MSC_VER)
		#pragma message("Capacity of each mesh vertex pool (used to store dynamic created vertices during triangulation) has not been specified, 64 is used as default.")
	#else
		#warning Capacity of each mesh vertex pool (used to store dynamic created vertices during triangulation) has not been specified, 64 is used as default.
	#endif
	// Capacity of each mesh vertex pool.
	#define AM_MESH_VERTEX_POOL_CAPACITY					64
#endif

#if !defined(AM_MESH_EDGE_POOL_CAPACITY)
	#if defined(_MSC_VER)
		#pragma message("Capacity of each mesh edge pool (used to store dynamic created edges during triangulation) has not been specified, 64 is used as default.")
	#else
		#warning Capacity of each mesh edge pool (used to store dynamic created edges during triangulation) has not been specified, 64 is used as default.
	#endif
	// Capacity of each mesh edge pool.
	#define AM_MESH_EDGE_POOL_CAPACITY						64
#endif
#endif

#if !defined(AM_PATHS_POOL_CAPACITY)
	#if defined(_MSC_VER)
		#pragma message("Capacity of each paths pool (used to store paths) has not been specified, 32 is used as default.")
	#else
		#warning Capacity of each paths pool (used to store paths) has not been specified, 32 is used as default.
	#endif
	// Capacity of each paths pool.
	#define AM_PATHS_POOL_CAPACITY							32
#endif

#if !defined(AM_PAINTS_POOL_CAPACITY)
	#if defined(_MSC_VER)
		#pragma message("Capacity of each paints pool (used to store paints) has not been specified, 32 is used as default.")
	#else
		#warning Capacity of each paints pool (used to store paints) has not been specified, 32 is used as default.
	#endif
	// Capacity of each paints pool.
	#define AM_PAINTS_POOL_CAPACITY							32
#endif

#if !defined(AM_FONT_GLYPH_POOL_CAPACITY)
	#if defined(_MSC_VER)
		#pragma message("Capacity of each font glyphs pool (used to store font glyphs) has not been specified, 32 is used as default.")
	#else
		#warning Capacity of each font glyphs pool (used to store font glyphs) has not been specified, 32 is used as default.
	#endif
	// Capacity of each font glyphs pool.
	#define AM_FONT_GLYPH_POOL_CAPACITY						32
#endif

#if !defined(AM_OPENVG_CALLS_BEFORE_MEMORY_RECOVERY)
	#if defined(_MSC_VER)
		#pragma message("The number of OpenVG calls to be done before to recover/retrieve unused memory has not been specified, 6400 is used as default.")
	#else
		#warning The number of OpenVG calls to be done before to recover/retrieve unused memory has not been specified, 6400 is used as default.
	#endif
	// The number of OpenVG calls to be done before to recover/retrieve unused memory.
	#define AM_OPENVG_CALLS_BEFORE_MEMORY_RECOVERY			6400
#endif


#if defined(AM_GLE) || defined(AM_GLS)
	#if !defined(AM_SCISSOR_BUFFER_BIT_GRANULARITY)
		#define AM_SCISSOR_BUFFER_BIT_GRANULARITY			2
	#endif
#endif

#if defined(AM_GLE)
// default quality for radial gradients
#if !defined(AM_GLE_RADIAL_GRADIENTS_DEFAULT_QUALITY)
	#if defined(_MSC_VER)
		#pragma message("The default quality level (0-100) used to realize radial gradients (GLE) has not been specified, 75 is used as default.")
	#else
		#warning The default quality level (0-100) used to relaize radial gradients (GLE) has not been specified, 75 is used as default.
	#endif
	#define AM_GLE_RADIAL_GRADIENTS_DEFAULT_QUALITY			75
#endif

// default quality for conical gradients
#if !defined(AM_GLE_CONICAL_GRADIENTS_DEFAULT_QUALITY)
	#if defined(_MSC_VER)
		#pragma message("The default quality level (0-100) used to realize conical gradients (GLE) has not been specified, 75 is used as default.")
	#else
		#warning The default quality level (0-100) used to realize conical gradients (GLE) has not been specified, 75 is used as default.
	#endif
	#define AM_GLE_CONICAL_GRADIENTS_DEFAULT_QUALITY		75
#endif
#endif

#if !defined(AM_COLOR_TRANSFORM_SCALE_BIAS_BITS)
	#if defined(_MSC_VER)
		#pragma message("The default number of bits used to represent integer parts (in fixed point) of color transform scale and bias has not been specified, 9 is used as default (range [-255.0f; 255.0f]).")
	#else
		#warning The default number of bits used to represent integer parts (in fixed point) of color transform scale and bias has not been specified, 9 is used as default (range [-255.0f; 255.0f]).
	#endif
	#define AM_COLOR_TRANSFORM_SCALE_BIAS_BITS				9
#endif

#if (AM_COLOR_TRANSFORM_SCALE_BIAS_BITS > 21)
	#error Maximum number of bits used to represent integer parts (in fixed point) of color transform scale and bias is 21.
#endif

#endif
