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

#ifndef _GLOBALS_H
#define _GLOBALS_H

/*!
	\file globals.h
	\brief Global types and constants defintion, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "amanith_config.h"
#include "vgext.h"
#include <stddef.h>

#if defined RIM_VG_SRC
#include "kd.h"
#define VG_API_EXIT
#endif

/* AmanithVG extensions */
#if defined(OPENVG_VERSION_1_1)
	#define AM_OPENVG_VERSION 110
#elif defined(OPENVG_VERSION_1_0_1)
	#define AM_OPENVG_VERSION 101
#else
	#error Undefined OpenVG version
#endif

// Operating system groups
#if (defined(AM_OS_WIN32) || defined(AM_OS_WIN64)) && !defined(AM_OS_WINCE)
	#define AM_OS_WIN
	#if !defined(_WIN32_WINNT)
		#define _WIN32_WINNT 0x0500
	#endif
	#if !defined(_WIN32_WINDOWS)
		#define _WIN32_WINDOWS 0x0410
	#endif
#endif
#if defined(AM_OS_MAC9) || defined(AM_OS_MACX)
	#define AM_OS_MAC
#endif
#if defined(AM_OS_LINUX) || defined(AM_OS_IRIX) || defined(AM_OS_HPUX) || defined(AM_OS_SOLARIS) || defined(AM_OS_AIX) ||  defined(AM_OS_FREEBSD) || defined(AM_OS_NETBSD) || defined(AM_OS_OPENBSD) || defined(AM_OS_QNX)
	#define AM_OS_UNIXLIKE
#endif

// Target architecture
#if defined(__arm__)
	#define AM_ARCH_ARM
#endif
#if defined(__thumb__) || defined(__thumb2__)
	#define AM_ARCH_THUMB
#endif
#if defined(__i386__) || defined(AM_OS_WIN32)
	#define AM_ARCH_I386
#endif
#if defined(__amd64__) || defined(AM_OS_WIN64)
	#define AM_ARCH_AMD64
#endif
#if defined(__ppc__) || defined(__powerpc__)
	#define AM_ARCH_PPC
#endif
#if defined(__SH4__) || defined(_SH4_) || defined(SH4) || defined(__sh4__)
	#define AM_ARCH_SH4
#endif

// Check for inconsistencies between endianess and architecture
#if (defined(AM_ARCH_I386) || defined(AM_ARCH_AMD64)) && defined(AM_BIG_ENDIAN)
	#error Endianess and architecture inconsistency.
#endif
#if defined(AM_ARCH_PPC) && defined(AM_LITTLE_ENDIAN)
	#error Endianess and architecture inconsistency.
#endif

// Visual C/C++ compiler annoying warnings
#if defined(AM_CC_MSVC)
	#pragma warning(disable : 4251)
	// take care of Visual Studio .Net 2005 C deprecations
	#if AM_CC_MSVC >= 1400
		#if !defined(_CRT_SECURE_NO_DEPRECATE)
			#define _CRT_SECURE_NO_DEPRECATE 1
		#endif
		#if !defined(_CRT_NONSTDC_NO_DEPRECATE)
			#define _CRT_NONSTDC_NO_DEPRECATE 1
		#endif
		#pragma warning (disable:4996)
	#endif
#endif

//! Inline function definition.
#define AM_INLINE static __inline

//! Extern storage-class specifier.
#if !defined(AM_EXTERN_C)
	#if defined(__cplusplus)
		#define AM_EXTERN_C extern "C"
	#else
		#define AM_EXTERN_C extern
	#endif
#endif
#if !defined(AM_EXTERN)
	#define AM_EXTERN extern
#endif

//! Pure function qualifier.
#if defined(AM_CC_GCC) && (__GNUC__ >= 3)
	#define AM_PURE __attribute__ ((pure))
	#define AM_NOALIAS
#elif defined(AM_CC_ARMCC)
	#define AM_PURE __pure
	#define AM_NOALIAS
#elif defined(AM_CC_MSVC)
	#define AM_PURE
	#define AM_NOALIAS __declspec(noalias)
#else
	#define AM_PURE
	#define AM_NOALIAS
#endif

//! Very simple assertion macro.
#if defined(_DEBUG)
	#if !defined(AM_OS_BREW)
		#include <assert.h>
		#define AM_ASSERT(_val) assert((_val))
	#else
		// It seems that Brew doesn't support assertions
		#define AM_ASSERT(_val) ((void)0)
	#endif
#else
	#define AM_ASSERT(_val) ((void)0)
#endif

//! Very simple debug macro.
#if defined(_DEBUG)
	void amDebug(const char *message);
	#define AM_DEBUG(_message) amDebug(_message);
#else
#if defined RIM_VG_SRC
    #define AM_DEBUG(_message) kdLogMessage((KDchar*)_message);
#else
	#define AM_DEBUG(_message) ((void)0);
#endif
#endif

//! Indexes enumeration for single component access.
typedef enum _AMVectorIndex {
	//! X component.
	AM_X = 0,
	//! Y component.
	AM_Y = 1,
	//! Z component.
	AM_Z = 2,
	//! W component.
	AM_W = 3
} AMVectorIndex;

//! Indexes enumeration for single color component access.
typedef enum _AMColorIndex {
	//! Red component.
	AM_R = 0,
	//! Green component.
	AM_G = 1,
	//! Blue component.
	AM_B = 2,
	//! Alpha component.
	AM_A = 3
} AMColorIndex;

// We need a NULL pointer from time to time.
#if !defined(NULL)
	#if defined(__cplusplus)
		#define NULL 0
	#else
		#define NULL ((void *)0)
	#endif
#endif // NULL

//! Handle type.
#define AMhandle void*
//! Integer type, 1 byte size.
#define AMint8 signed char
//! Integer type, 2 bytes size.
#define AMint16 signed short
//! Integer type, 4 bytes size.
#define AMint32 signed int
//! Unsigned integer type, 1 byte size.
#define AMuint8 unsigned char
//! Unsigned integer type, 2 bytes size.
#define AMuint16 unsigned short
//! Unsigned integer type, 4 bytes size.
#define AMuint32 unsigned int
#if defined(AM_ARCH_AMD64) && defined(AM_CC_MSVC)
	//! Unsigned long type.
	#define AMulong unsigned __int64
	//! Signed long type.
	#define AMlong __int64
#else
	//! Unsigned long type.
	#define AMulong unsigned long
	//! Signed long type.
	#define AMlong long
#endif
#if defined(AM_NO_64BIT_SUPPORT)
	//! Unsigned integer type, 8 bytes size.
	typedef union _AMuint64 {
		struct {
		#if defined(AM_BIG_ENDIAN)
			// Motorola, Power PC
			//! Higher 32bit part.
			AMuint32 hi;
			//! Lower 32bit part.
			AMuint32 lo;
		#elif defined(AM_LITTLE_ENDIAN)
			// Intel
			//! Lower 32bit part.
			AMuint32 lo;
			//! Higher 32bit part.
			AMuint32 hi;
		#else
			#error Unreachable point, please define target machine endianess (check config.h inclusion).
		#endif
		} c;
		// Debug stuff
	#if defined(_DEBUG) && defined(AM_CC_MSVC)
		__int64 i64;
		unsigned __int64 u64;
	#endif
	} AMuint64;
	//! Signed integer type, 8 bytes size.
	#define AMint64 AMuint64
#else
	#if defined(AM_CC_MSVC)
		//! Signed integer type, 8 bytes size.
		#define AMint64 __int64
		//! Unsigned integer type, 8 bytes size.
		#define AMuint64 unsigned __int64
	#else
		//! Signed integer type, 8 bytes size.
		#define AMint64 long long
		//! Unsigned integer type, 8 bytes size.
		#define AMuint64 unsigned long long
	#endif
#endif
//! Fixed point type.
#define AMfixed int
//! Boolean type.
#define AMbool unsigned int
//! Float type.
#define AMfloat float
//! Double precision type.
#define AMdouble double
//! 8-bit signed char type.
#define AMchar8 char
//! 8-bit unsigned char type (sometime known as byte).
#define AMuchar8 unsigned char
//! Pointer types.
#if defined (AM_CC_MSVC) && (AM_CC_MSVC >= 1300) && defined(AM_OS_WIN)
	//! Unsigned int that can hold a pointer to any type.
	#define AMuintptr uintptr_t
	//! Signed int that can hold a pointer to any type.
	#define AMintptr intptr_t
#else
	//! Unsigned int that can hold a pointer to any type.
	#define AMuintptr unsigned long
	//! Signed int that can hold a pointer to any type.
	#define AMintptr signed long
#endif

//! Unsigned 64bit integer structure, useful to access to lower and higher 32bit components.
typedef union _AMUnsigned64 {
	struct {
	#if defined(AM_BIG_ENDIAN)
		// Motorola, Power PC
		//! Higher 32bit part.
		AMuint32 hi;
		//! Lower 32bit part.
		AMuint32 lo;
	#elif defined(AM_LITTLE_ENDIAN)
		// Intel
		//! Lower 32bit part.
		AMuint32 lo;
		//! Higher 32bit part.
		AMuint32 hi;
	#else
		#error Unreachable point, please define target machine endianess (check config.h inclusion).
	#endif
	} c;
	//! Alias to refer the whole 64bit value.
	AMuint64 n;
} AMUnsigned64;

//! Maximum signed AMint8 value.
#define AM_MAX_INT8 127
//! Minimum signed AMint8 value.
#define AM_MIN_INT8 (-128)
//! Maximum signed AMint16 value.
#define AM_MAX_INT16 32767
//! Minimum signed AMint16 value.
#define AM_MIN_INT16 (-32768)
//! Maximum signed AMint32 value.
#define AM_MAX_INT32 2147483647
//! Minimum signed AMint32 value.
#define AM_MIN_INT32 (-2147483647 - 1)
//! Maximum signed AMlLong value.
#define AM_MAX_LONG 2147483647L
//! Minimum signed AMlong value.
#define AM_MIN_LONG (-2147483647L - 1)
//! Maximum signed AMint64 value.
#define AM_MAX_INT64 9223372036854775807
//! Minimum signed AMint64 value.
#define AM_MIN_INT64 (-9223372036854775808)
//! Maximum unsigned AMuint8 value.
#define AM_MAX_UINT8 255
//! Maximum unsigned AMuint16 value.
#define AM_MAX_UINT16 65535
//! Maximum unsigned AMuint32 value.
#define AM_MAX_UINT32 4294967295U
//! Maximum unsigned AMulong value.
#define AM_MAX_ULONG 4294967295UL
//! Maximum unsigned AMuint64 value.
#define AM_MAX_UINT64 0xFFFFFFFFFFFFFFFF
//! Maximum signed AMfloat value.
#define AM_MAX_FLOAT 3.402823466e+38f
//! Minimum signed AMfloat value.
#define AM_MIN_FLOAT (-3.402823466e+38f)
//! Maximum signed AMdouble value.
#define AM_MAX_DOUBLE 1.7976931348623157e+308
//! Minimum signed AMdouble value.
#define AM_MIN_DOUBLE (-1.7976931348623157e+308)
//! Maximum signed AMint16 value expressed in float precision.
#define AM_MAX_INT16_F 32768.0f
//! Minimum signed AMint16 value expressed in float precision.
#define AM_MIN_INT16_F (-32768.0f)
//! True boolean value used by AMbool type.
#define AM_TRUE 1
//! False boolean value used by AMbool type.
#define AM_FALSE 0

//! Endian types.
typedef enum _AMEndianessType {
	//! Big endian order (Motorola, PowerPC).
	AM_BIG_ENDIANESS,
	//! Little endian order (Intel).
	AM_LITTLE_ENDIANESS
} AMEndianessType;

//! Flattening parameters, used by flattening functions.
typedef struct _AMFlattenParams {
	//! Squared chordal distance; chordal distance is defined as the maximum distance between a curve arc and the subsisted chord).
	AMfloat deviation;
	//! Chordal distance, sqrt(deviation).
	AMfloat flatness;
	//! 2 * sqrt(flatness).
	AMfloat two_sqrt_flatness;
	//! 3 / flatness.
	AMfloat three_over_flatness;
	//! 2 * sqrt(flatness / 3).
	AMfloat two_sqrt_flatness_over_three;
	//! 2 * cubeRoot(flatness / 3).
	AMfloat two_cuberoot_flatness_over_three;
	//! 64 * flatness.
	AMfloat sixtyfour_flatness;
	//! 1 + 1 / (3 * sqrt(sqrt(flatness))).
	AMint32 degenerative_curve_segments;
} AMFlattenParams;

//! Function prototype for a scanline filler.
typedef void (*AMScanlineFillerFunction)(void *,
										 void *,
										 AMint32,
										 AMint32,
										 AMint32);

// for OpenVG > 1.0, define invalid format/datatype for internal use
#if !defined(VG_PATH_DATATYPE_INVALID)
	#define VG_PATH_DATATYPE_INVALID (-1)
#endif
#if !defined(VG_IMAGE_FORMAT_INVALID)
	#define VG_IMAGE_FORMAT_INVALID (-1)
#endif

// handle IDs for AMImage, AMPaint, AMPath
#define AM_INVALID_HANDLE_ID	0
#define AM_PATH_HANDLE_ID		1
#define AM_IMAGE_HANDLE_ID		2
#define AM_PAINT_HANDLE_ID		3
#define AM_LAYER_HANDLE_ID		4
#define AM_FONT_HANDLE_ID		5

// internal path segments identifiers
#define AM_UNKNOWN_SEGMENT	0
#define AM_MOVE_TO_SEGMENT	1
#define AM_LINE_TO_SEGMENT	2
#define AM_BEZ2_TO_SEGMENT	3
#define AM_BEZ3_TO_SEGMENT	4
#define AM_ARC_TO_SEGMENT	5
#define AM_CLOSE_SEGMENT	6

// internal 32 bit pixel formats
typedef enum _AMPixelFormat {
	AM_PIXEL_FMT_RGBA  = 0,
	AM_PIXEL_FMT_ABGR  = 1,
	AM_PIXEL_FMT_ARGB  = 2,
	AM_PIXEL_FMT_BGRA  = 3
} AMPixelFormat;

//! Number of bits used to represent fractional parts (in fixed point) of color transform scale and bias.
#define AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION (32 - 8 - 1 - AM_COLOR_TRANSFORM_SCALE_BIAS_BITS)
//! Float factor used to convert color transform scale and bias into fixed point.
#define AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION_F ((AMfloat)(1 << AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION))
#define AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F ((AMfloat)((1 << (AM_COLOR_TRANSFORM_SCALE_BIAS_BITS - 1)) - 1))

#define AM_FIRST_MATRIX_MODE			VG_MATRIX_PATH_USER_TO_SURFACE
#if (AM_OPENVG_VERSION >= 110)
	#define AM_LAST_MATRIX_MODE			VG_MATRIX_GLYPH_USER_TO_SURFACE
#else
	#define AM_LAST_MATRIX_MODE			VG_MATRIX_STROKE_PAINT_TO_USER
#endif
#define AM_FIRST_PATH_DATATYPE			VG_PATH_DATATYPE_S_8
#define AM_LAST_PATH_DATATYPE			VG_PATH_DATATYPE_F
#define AM_FIRST_PAINT_TYPE				VG_PAINT_TYPE_COLOR
#define AM_LAST_PAINT_TYPE				VG_PAINT_TYPE_PATTERN
#if defined(VG_MZT_conical_gradient)
	#define AM_FIRST_EXT_PAINT_TYPE		VG_PAINT_TYPE_CONICAL_GRADIENT_MZT
	#define AM_LAST_EXT_PAINT_TYPE		VG_PAINT_TYPE_CONICAL_GRADIENT_MZT
	#define AM_EXT_PAINT_TYPES_COUNT	(AM_LAST_EXT_PAINT_TYPE - AM_FIRST_EXT_PAINT_TYPE + 1)
#else
	#define AM_FIRST_EXT_PAINT_TYPE		0
	#define AM_LAST_EXT_PAINT_TYPE		0
	#define AM_EXT_PAINT_TYPES_COUNT	0
#endif
#define AM_FIRST_BLEND_MODE				VG_BLEND_SRC
#define AM_LAST_BLEND_MODE				VG_BLEND_ADDITIVE
#if defined(VG_MZT_advanced_blend_modes)
	#define AM_FIRST_EXT_BLEND_MODE		VG_BLEND_CLEAR_MZT
	#define AM_LAST_EXT_BLEND_MODE		VG_BLEND_EXCLUSION_MZT
	#define AM_EXT_BLEND_MODES_COUNT	(AM_LAST_EXT_BLEND_MODE - AM_FIRST_EXT_BLEND_MODE + 1)
#else
	#define AM_FIRST_EXT_BLEND_MODE		0
	#define AM_LAST_EXT_BLEND_MODE		0
	#define AM_EXT_BLEND_MODES_COUNT	0
#endif
#define AM_FIRST_IMAGE_MODE				VG_DRAW_IMAGE_NORMAL
#define AM_LAST_IMAGE_MODE				VG_DRAW_IMAGE_STENCIL
#define AM_FIRST_IMAGE_FORMAT			VG_sRGBX_8888
#if (AM_OPENVG_VERSION >= 110)
	#define AM_LAST_IMAGE_FORMAT		VG_A_4
#else
	#define AM_LAST_IMAGE_FORMAT		VG_BW_1
#endif
// RGBA, ARGB, BGRA, ABGR
#define AM_PIXEL_BYTE_ORDERS_COUNT		4

// Number of (internal) path segment types
#define AM_PATH_SEGMENT_TYPES_COUNT (AM_CLOSE_SEGMENT - AM_UNKNOWN_SEGMENT + 1)
// Number of path data types
#define AM_PATH_DATATYPES_COUNT	(AM_LAST_PATH_DATATYPE - AM_FIRST_PATH_DATATYPE + 1)
// Offset to extended paint types
#define AM_EXT_PAINT_TYPE_OFFSET (AM_LAST_PAINT_TYPE - AM_FIRST_PAINT_TYPE + 1)
// Offset to extended blend modes
#define AM_EXT_BLEND_MODE_OFFSET (AM_LAST_BLEND_MODE - AM_FIRST_BLEND_MODE + 1)
// Number of paint types
#define AM_PAINT_TYPES_COUNT (AM_EXT_PAINT_TYPE_OFFSET + AM_EXT_PAINT_TYPES_COUNT)
//! Number of blend modes
#define AM_BLEND_MODES_COUNT (AM_EXT_BLEND_MODE_OFFSET + AM_EXT_BLEND_MODES_COUNT)
// Number of image modes
#define AM_IMAGE_MODES_COUNT (AM_LAST_IMAGE_MODE - AM_FIRST_IMAGE_MODE + 1)
// Number of image formats (base formats * byte order variants)
#define AM_IMAGE_FORMATS_COUNT ((AM_LAST_IMAGE_FORMAT - AM_FIRST_IMAGE_FORMAT + 1) * AM_PIXEL_BYTE_ORDERS_COUNT)

//! Given a paintType, it returns its 0-based index.
AM_INLINE AMuint32 amPaintTypeGetIndex(const AMuint32 paintType) {

#if defined(VG_MZT_conical_gradient)
	if (paintType >= AM_FIRST_EXT_PAINT_TYPE)
		return paintType - AM_FIRST_EXT_PAINT_TYPE + AM_EXT_PAINT_TYPE_OFFSET;
#endif
	return paintType - AM_FIRST_PAINT_TYPE;
}

//! Given a blendMode, it returns its 0-based index.
AM_INLINE AMuint32 amBlendModeGetIndex(const AMuint32 blendMode) {

#if defined(VG_MZT_advanced_blend_modes)
	if (blendMode >= AM_FIRST_EXT_BLEND_MODE)
		return blendMode - AM_FIRST_EXT_BLEND_MODE + AM_EXT_BLEND_MODE_OFFSET;
#endif
	return blendMode - AM_FIRST_BLEND_MODE;
}

//! Given an imageMode, it returns its 0-based index.
AM_INLINE AMuint32 amImageModeGetIndex(const AMuint32 imageMode) {

	return imageMode - AM_FIRST_IMAGE_MODE;
}

//! Check if a pointer is aligned.
AM_INLINE AMbool amPointerIsAligned(const void *pointer,
									const AMint32 alignment) {

	AM_ASSERT(alignment == 1 || alignment == 2 || alignment == 4);

	return (((AMuintptr)pointer) & (alignment - 1)) ? AM_FALSE : AM_TRUE;
}

//! Minimum float number that satisfies (1.0f + AM_EPSILON_FLOAT) != 1.0f
AM_EXTERN_C const AMfloat AM_EPSILON_FLOAT;
// Get system endianess.
AMEndianessType amSystemEndianessGet(void);
// Get system word size.
AMuint32 amSystemWordSizeGet(void);

/*!
	\brief Convert a single precision floating point value into a 16.16 fixed point, without using floating point calculations.
	\param f input floating point value.
	\return the corresponding 16.16 fixed point representation.
*/
AM_INLINE AM_NOALIAS AM_PURE AMfixed amFloatToFixed1616(AMfloat f) {

#if defined(AM_IEEE754_NOT_COMPLIANT)
	return (AMfixed)(f * 65536.0f);
#else
	AMuint32 data = *(AMuint32 *)&f;
	AMuint32 mantissa = (data & 0x007FFFFF) | 0x00800000;
	AMint8 exponent = (AMint8)((data >> 23) & 0xFF) - 134;

	mantissa = (exponent > 0) ? mantissa << exponent : (exponent < -23 ? 1 : mantissa >> (-exponent));
	return (data & 0x80000000) ? 0 - mantissa : mantissa;
#endif
}

/*!
	\brief Convert a single precision floating point value into a 17.15 fixed point, without using floating point calculations.
	\param f input floating point value.
	\return the corresponding 17.15 fixed point representation.
*/
AM_INLINE AM_NOALIAS AM_PURE AMfixed amFloatToFixed1715(AMfloat f) {

#if defined(AM_IEEE754_NOT_COMPLIANT)
	return (AMfixed)(f * 32768.0f);
#else
	AMuint32 data = *(AMuint32 *)&f;
	AMuint32 mantissa = (data & 0x007FFFFF) | 0x00800000;
	AMint8 exponent = (AMint8)((data >> 23) & 0xFF) - 135;

	mantissa = (exponent > 0) ? mantissa << exponent : (exponent < -23 ? 1 : mantissa >> (-exponent));
	return (data & 0x80000000) ? 0 - mantissa : mantissa;
#endif
}

#if defined(AM_FIXED_POINT_PIPELINE)
	#define AM_RAS_INPUT_VERTEX_COMPONENT_TYPE AMfixed
	#define AM_RAS_INPUT_VERTEX_TYPE AMVect2x
	#define AM_RAS_INPUT_VERTICES_ARRAY_TYPE AMVect2xDynArray
	#define AM_RAS_CLIP_BOX_TYPE AMAABox2i
	#define AM_RAS_MATRIX_TYPE AMMatrix33x
#else
	#define AM_RAS_INPUT_VERTEX_COMPONENT_TYPE AMfloat
	#define AM_RAS_INPUT_VERTEX_TYPE AMVect2f
	#define AM_RAS_INPUT_VERTICES_ARRAY_TYPE AMVect2fDynArray
	#define AM_RAS_CLIP_BOX_TYPE AMAABox2f
	#define AM_RAS_MATRIX_TYPE AMMatrix33f
#endif

//! Invalid hash value.
#define AM_HASH_INVALID 4294967291u
//! Hash value corresponding to the identity color transformation
#define AM_COLOR_TRANSFORM_IDENTITY_HASH 0xE0000000u

// Calculate the hash value corresponding to a set of float values.
AMuint32 amHashCalculate(const AMfloat *values,
						 const AMuint32 count);

#if defined(VG_MZT_statistics)
#if defined(AM_OS_WIN) || defined(AM_OS_WINCE)
	#include <Windows.h>
#elif defined(AM_OS_SYMBIAN) || defined(AM_OS_UNIXLIKE) || defined(AM_OS_MAC)
	#include <sys/time.h>
#endif
/*!
	This function returns the number of milliseconds since an absolute time point (it heavily depends on platform)
*/
AM_INLINE AMuint32 amTimeGet(void) {
#if defined(AM_OS_BREW)
	return (AMuint32)GETUPTIMEMS();
#elif defined(AM_OS_SYMBIAN) || defined(AM_OS_UNIXLIKE) || defined(AM_OS_MAC)

	struct timeval tp;
	struct timezone tzp;

	gettimeofday(&tp, &tzp);
	return (AMuint32)((tp.tv_sec * 1000) + (tp.tv_usec / 1000));
#elif defined(AM_OS_WIN) || defined(AM_OS_WINCE)
	return (AMuint32)GetTickCount();
#else
	Please define your platform!
#endif
}
#endif

#endif
