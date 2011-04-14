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

#ifndef _PIXEL_UTILS_H
#define _PIXEL_UTILS_H

/*!
	\file pixel_utils.h
	\brief Pixel utilities, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "amanith_globals.h"

//! 32bit RGBA pixel format.
typedef union _AMPixel32RGBA {
	struct {
	#if defined(AM_BIG_ENDIAN)
		// big endian machines (e.g. PowerPC).
		//! 8bit red component.
		AMuint8 r: 8;
		//! 8bit green component.
		AMuint8 g: 8;
		//! 8bit blue component.
		AMuint8 b: 8;
		//! 8bit alpha component.
		AMuint8 a: 8;
	#elif defined(AM_LITTLE_ENDIAN)
		// little endian machines (e.g. Intel).
		//! 8bit alpha component.
		AMuint8 a: 8;
		//! 8bit blue component.
		AMuint8 b: 8;
		//! 8bit green component.
		AMuint8 g: 8;
		//! 8bit red component.
		AMuint8 r: 8;
	#else
		#error Unreachable point, please define target machine endianess (check config.h inclusion).
	#endif
    } c;
	//!	\name Aliases to refer the whole 32bit pixel value.
	//@{
	AMuint32 rgba;
	AMuint32 value;
	//@}
} AMPixel32RGBA;

//! 32bit ARGB pixel format.
typedef union _AMPixel32ARGB {
	struct {
	#if defined(AM_BIG_ENDIAN)
		// big endian machines (e.g. PowerPC).
		//! 8bit alpha component.
		AMuint8 a: 8;
		//! 8bit red component.
		AMuint8 r: 8;
		//! 8bit green component.
		AMuint8 g: 8;
		//! 8bit blue component.
		AMuint8 b: 8;
	#elif defined(AM_LITTLE_ENDIAN)
		// little endian machines (e.g. Intel).
		//! 8bit blue component.
		AMuint8 b: 8;
		//! 8bit green component.
		AMuint8 g: 8;
		//! 8bit red component.
		AMuint8 r: 8;
		//! 8bit alpha component.
		AMuint8 a: 8;
	#else
		#error Unreachable point, please define target machine endianess (check config.h inclusion).
	#endif
	} c;
	//!	\name Aliases to refer the whole 32bit pixel value.
	//@{
	AMuint32 argb;
	AMuint32 value;
	//@}
} AMPixel32ARGB;

//! 32bit BGRA pixel format.
typedef union _AMPixel32BGRA {
	struct {
	#if defined(AM_BIG_ENDIAN)
		// big endian machines (e.g. PowerPC).
		//! 8bit blue component.
		AMuint8 b: 8;
		//! 8bit green component.
		AMuint8 g: 8;
		//! 8bit red component.
		AMuint8 r: 8;
		//! 8bit alpha component.
		AMuint8 a: 8;
	#elif defined(AM_LITTLE_ENDIAN)
		// little endian machines (e.g. Intel).
		//! 8bit alpha component.
		AMuint8 a: 8;
		//! 8bit red component.
		AMuint8 r: 8;
		//! 8bit green component.
		AMuint8 g: 8;
		//! 8bit blue component.
		AMuint8 b: 8;
	#else
		#error Unreachable point, please define target machine endianess (check config.h inclusion).
	#endif
    } c;
	//!	\name Aliases to refer the whole 32bit pixel value.
	//@{
	AMuint32 bgra;
	AMuint32 value;
	//@}
} AMPixel32BGRA;

//! 32bit ABGR pixel format.
typedef union _AMPixel32ABGR {
	struct {
	#if defined(AM_BIG_ENDIAN)
		// big endian machines (e.g. PowerPC).
		//! 8bit alpha component.
		AMuint8 a: 8;
		//! 8bit blue component.
		AMuint8 b: 8;
		//! 8bit green component.
		AMuint8 g: 8;
		//! 8bit red component.
		AMuint8 r: 8;
	#elif defined(AM_LITTLE_ENDIAN)
		// little endian machines (e.g. Intel).
		//! 8bit red component.
		AMuint8 r: 8;
		//! 8bit green component.
		AMuint8 g: 8;
		//! 8bit blue component.
		AMuint8 b: 8;
		//! 8bit alpha component.
		AMuint8 a: 8;
	#else
		#error Unreachable point, please define target machine endianess (check config.h inclusion).
	#endif
	} c;
	//!	\name Aliases to refer the whole 32bit pixel value.
	//@{
	AMuint32 abgr;
	AMuint32 value;
	//@}
} AMPixel32ABGR;

//! 16bit pixel format.
typedef union _AMPixel16 {
	struct {
	#if defined(AM_BIG_ENDIAN)
		// big endian machines (e.g. PowerPC).
		//! 8bit high component.
		AMuint8 hi: 8;
		//! 8bit low component.
		AMuint8 lo: 8;
	#elif defined(AM_LITTLE_ENDIAN)
		// little endian machines (e.g. Intel).
		//! 8bit low component.
		AMuint8 lo: 8;
		//! 8bit high component.
		AMuint8 hi: 8;
	#else
		#error Unreachable point, please define target machine endianess (check config.h inclusion).
	#endif
	} c;
	//! Alias to refer the whole 16bit pixel value.
	AMuint16 value;
} AMPixel16;

// default pixel format and associated macros
#if defined(AM_SURFACE_BYTE_ORDER_RGBA)
	//! Alias for a 32bit pixel in the specified internal drawing surface format.
	#define AMPixel32 AMPixel32RGBA
	//! Alias for a 32bit pixel component-wise constant scaling, preserving alpha.
	#define amPxlSclPreserveA amPxl_RGBA_SclPreserveA
	//! Alias for a 32bit pixel component-wise multiplication, excluding alpha.
	#define amPxlMulNoA amPxl_RGBA_MulNoA
	//! Alias for a 32bit pixel component-wise addition, excluding alpha.
	#define amPxlAddsNoA amPxl_RGBA_AddsNoA
	//! Alias for a 32bit pixel component-wise minimum extraction, excluding alpha.
	#define amPxlMinNoA amPxl_RGBA_MinNoA
	//! Alias for a 32bit pixel component-wise maximum extraction, excluding alpha.
	#define amPxlMaxNoA amPxl_RGBA_MaxNoA
#elif defined(AM_SURFACE_BYTE_ORDER_ARGB)
	//! Alias for a 32bit pixel in the specified internal drawing surface format.
	#define AMPixel32 AMPixel32ARGB
	//! Alias for a 32bit pixel component-wise constant scaling, preserving alpha.
	#define amPxlSclPreserveA amPxl_ARGB_SclPreserveA
	//! Alias for a 32bit pixel component-wise multiplication, excluding alpha.
	#define amPxlMulNoA amPxl_ARGB_MulNoA
	//! Alias for a 32bit pixel component-wise addition, excluding alpha.
	#define amPxlAddsNoA amPxl_ARGB_AddsNoA
	//! Alias for a 32bit pixel component-wise minimum extraction, excluding alpha.
	#define amPxlMinNoA amPxl_ARGB_MinNoA
	//! Alias for a 32bit pixel component-wise maximum extraction, excluding alpha.
	#define amPxlMaxNoA amPxl_ARGB_MaxNoA
#elif defined(AM_SURFACE_BYTE_ORDER_BGRA)
	//! Alias for a 32bit pixel in the specified internal drawing surface format.
	#define AMPixel32 AMPixel32BGRA
	//! Alias for a 32bit pixel component-wise constant scaling, preserving alpha.
	#define amPxlSclPreserveA amPxl_BGRA_SclPreserveA
	//! Alias for a 32bit pixel component-wise multiplication, excluding alpha.
	#define amPxlMulNoA amPxl_BGRA_MulNoA
	//! Alias for a 32bit pixel component-wise addition, excluding alpha.
	#define amPxlAddsNoA amPxl_BGRA_AddsNoA
	//! Alias for a 32bit pixel component-wise minimum extraction, excluding alpha.
	#define amPxlMinNoA amPxl_BGRA_MinNoA
	//! Alias for a 32bit pixel component-wise maximum extraction, excluding alpha.
	#define amPxlMaxNoA amPxl_BGRA_MaxNoA
#else
	// AM_SURFACE_BYTE_ORDER_ABGR
	//! Alias for a 32bit pixel in the specified internal drawing surface format.
	#define AMPixel32 AMPixel32ABGR
	//! Alias for a 32bit pixel component-wise constant scaling, preserving alpha.
	#define amPxlSclPreserveA amPxl_ABGR_SclPreserveA
	//! Alias for a 32bit pixel component-wise multiplication, excluding alpha.
	#define amPxlMulNoA amPxl_ABGR_MulNoA
	//! Alias for a 32bit pixel component-wise addition, excluding alpha.
	#define amPxlAddsNoA amPxl_ABGR_AddsNoA
	//! Alias for a 32bit pixel component-wise minimum extraction, excluding alpha.
	#define amPxlMinNoA amPxl_ABGR_MinNoA
	//! Alias for a 32bit pixel component-wise maximum extraction, excluding alpha.
	#define amPxlMaxNoA amPxl_ABGR_MaxNoA
#endif

/*!
	\brief 32bit pixel component-wise linear interpolation.
	\param a input interpolation parameter.
	\param d input pixel destination.
	\param s input pixel source.
	\return interpolated pixel: a * s + (256 - a) * d.
	\pre a must be between 0 and 256.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxlLerp(const AMuint32 a,
												const AMuint32 d,
												const AMuint32 s) {

	AMuint32 dstga = d & 0x00FF00FF;
	AMuint32 dstrb = (d >> 8) & 0x00FF00FF;
	AMuint32 srcga = s & 0x00FF00FF;
	AMuint32 srcrb = (s >> 8) & 0x00FF00FF;
	AMuint32 ga = ((((srcga - dstga) * a) >> 8) + dstga) & 0x00FF00FF;
	AMuint32 rb = (((((srcrb - dstrb) * a) >> 8) + dstrb) << 8) & 0xFF00FF00;
	return (rb | ga);
}

/*!
	\brief 32bit pixel component-wise constant scaling.
	\param scale input scale factor.
	\param p input pixel.
	\return scaled pixel.
	\pre scale must be between 0 and 256.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxlScl256(const AMuint32 scale,
												  const AMuint32 p) {

	AMuint32 rb = (p & 0xFF00FF00) >> 8;
	AMuint32 ga =  p & 0x00FF00FF;
	AMuint32 srb = scale * rb;
	AMuint32 sga = scale * ga;

	srb = srb & 0xFF00FF00;
	sga = (sga >> 8) & 0x00FF00FF;
	return srb | sga;
}

/*!
	\brief 32bit pixel (LSB alpha) component-wise constant scaling, preserving alpha.
	\param scale input scale factor.
	\param p input pixel.
	\return scaled pixel.
	\pre scale must be between 0 and 256.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxl_RGBA_Scl256PreserveA(const AMuint32 scale,
																 const AMuint32 p) {

	AMuint32 rb = (p & 0xFF00FF00) >> 8;
	AMuint32 ga =  p & 0x00FF00FF;
	AMuint32 srb = scale * rb;
	AMuint32 sga = scale * ga;

	srb = srb & 0xFF00FF00;
	sga = (sga >> 8) & 0x00FF0000;
	return srb | sga | (p & 0xFF);
}

/*!
	\brief 32bit pixel (MSB alpha) component-wise constant scaling, preserving alpha.
	\param scale input scale factor.
	\param p input pixel.
	\return scaled pixel.
	\pre scale must be between 0 and 256.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxl_ARGB_Scl256PreserveA(const AMuint32 scale,
																 const AMuint32 p) {

	AMuint32 ag = (p & 0xFF00FF00) >> 8;
	AMuint32 rb =  p & 0x00FF00FF;
	AMuint32 sag = scale * ag;
	AMuint32 srb = scale * rb;

	sag = sag & 0x0000FF00;
	srb = (srb >> 8) & 0x00FF00FF;
	return (p & 0xFF000000) | sag | srb;
}

//! Alias for 32bit pixel (LSB alpha) component-wise constant scaling, preserving alpha.
#define amPxl_BGRA_Scl256PreserveA amPxl_RGBA_Scl256PreserveA
//! Alias for 32bit pixel (MSB alpha) component-wise constant scaling, preserving alpha.
#define amPxl_ABGR_Scl256PreserveA amPxl_ARGB_Scl256PreserveA

/*!
	\brief 32bit pixel component-wise constant scaling.
	\param scale input scale factor.
	\param p input pixel.
	\return scaled pixel.
	\pre scale must be between 0 and 255.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxlScl255(const AMuint32 scale,
												  const AMuint32 p) {

	AMuint32 rb = (p & 0xFF00FF00) >> 8;
	AMuint32 ga =  p & 0x00FF00FF;
	AMuint32 srb = scale * rb + 0x00800080;
	AMuint32 sga = scale * ga + 0x00800080;

	srb += ((srb >> 8) & 0x00FF00FF);
	sga += ((sga >> 8) & 0x00FF00FF);
	srb = srb & 0xFF00FF00;
	sga = (sga >> 8) & 0x00FF00FF;
	return srb | sga;
}
/*!
	\brief 32bit pixel (LSB alpha) component-wise constant scaling, preserving alpha.
	\param scale input scale factor.
	\param p input pixel.
	\return scaled pixel.
	\pre scale must be between 0 and 255.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxl_RGBA_Scl255PreserveA(const AMuint32 scale,
																 const AMuint32 p) {

	AMuint32 rb = (p & 0xFF00FF00) >> 8;
	AMuint32 ga =  p & 0x00FF00FF;
	AMuint32 srb = scale * rb + 0x00800080;
	AMuint32 sga = scale * ga + 0x00800080;

	srb += ((srb >> 8) & 0x00FF00FF);
	sga += ((sga >> 8) & 0x00FF00FF);
	srb = srb & 0xFF00FF00;
	sga = (sga >> 8) & 0x00FF0000;
	return srb | sga | (p & 0xFF);
}

/*!
	\brief 32bit pixel (MSB alpha) component-wise constant scaling, preserving alpha.
	\param scale input scale factor.
	\param p input pixel.
	\return scaled pixel.
	\pre scale must be between 0 and 255.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxl_ARGB_Scl255PreserveA(const AMuint32 scale,
																 const AMuint32 p) {

	AMuint32 ag = (p & 0xFF00FF00) >> 8;
	AMuint32 rb =  p & 0x00FF00FF;
	AMuint32 sag = scale * ag + 0x00800080;
	AMuint32 srb = scale * rb + 0x00800080;

	sag += ((sag >> 8) & 0x00FF00FF);
	srb += ((srb >> 8) & 0x00FF00FF);
	sag = sag & 0x0000FF00;
	srb = (srb >> 8) & 0x00FF00FF;
	return (p & 0xFF000000) | sag | srb;
}

//! Alias for 32bit pixel (LSB alpha) component-wise constant scaling, preserving alpha.
#define amPxl_BGRA_Scl255PreserveA amPxl_RGBA_Scl255PreserveA
//! Alias for 32bit pixel (MSB alpha) component-wise constant scaling, preserving alpha.
#define amPxl_ABGR_Scl255PreserveA amPxl_ARGB_Scl255PreserveA

/*!
	\brief 32bit pixel component-wise multiplication.
	\param d first input pixel.
	\param s second input pixel.
	\return multiplied pixel.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxlMul(const AMuint32 d,
											   const AMuint32 s) {

	AMPixel16 t16;
	AMPixel32 d32, s32, res32;

	d32.value = d;
	s32.value = s;

	t16.value = (AMuint16)d32.c.a * s32.c.a;
	res32.c.a = t16.c.hi;
	t16.value = (AMuint16)d32.c.b * s32.c.b;
	res32.c.b = t16.c.hi;
	t16.value = (AMuint16)d32.c.g * s32.c.g;
	res32.c.g = t16.c.hi;
	t16.value = (AMuint16)d32.c.r * s32.c.r;
	res32.c.r = t16.c.hi;
	return res32.value;
}

/*!
	\brief 32bit pixel (LSB alpha) component-wise multiplication, excluding alpha.
	\param d first input pixel.
	\param s second input pixel.
	\return multiplied pixel, with (LSB) alpha component set to 0.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxl_RGBA_MulNoA(const AMuint32 d,
														const AMuint32 s) {

	AMPixel16 t16;
	AMPixel32RGBA d32, s32, res32;

	d32.rgba = d;
	s32.rgba = s;

	res32.c.a = 0;
	t16.value = (AMuint16)d32.c.b * s32.c.b;
	res32.c.b = t16.c.hi;
	t16.value = (AMuint16)d32.c.g * s32.c.g;
	res32.c.g = t16.c.hi;
	t16.value = (AMuint16)d32.c.r * s32.c.r;
	res32.c.r = t16.c.hi;
	return res32.rgba;
}

/*!
	\brief 32bit pixel (MSB alpha) component-wise multiplication, excluding alpha.
	\param d first input pixel.
	\param s second input pixel.
	\return multiplied pixel, with (MSB) alpha component set to 0.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxl_ARGB_MulNoA(const AMuint32 d,
														const AMuint32 s) {

	AMPixel16 t16;
	AMPixel32ARGB d32, s32, res32;

	d32.argb = d;
	s32.argb = s;

	res32.c.a = 0;
	t16.value = (AMuint16)d32.c.b * s32.c.b;
	res32.c.b = t16.c.hi;
	t16.value = (AMuint16)d32.c.g * s32.c.g;
	res32.c.g = t16.c.hi;
	t16.value = (AMuint16)d32.c.r * s32.c.r;
	res32.c.r = t16.c.hi;
	return res32.argb;
}

//! Alias for 32bit pixel (LSB alpha) component-wise multiplication, excluding alpha.
#define amPxl_BGRA_MulNoA amPxl_RGBA_MulNoA
//! Alias for 32bit pixel (MSB alpha) component-wise multiplication, excluding alpha.
#define amPxl_ABGR_MulNoA amPxl_ARGB_MulNoA

/*!
	\brief 32bit pixel component-wise inversion.
	\param s input pixel.
	\return inverted pixel: 255 - component.value, for each component.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxlInv(const AMuint32 s) {

	return (s ^ 0xFFFFFFFF);
}

/*!
	\brief 32bit pixel component-wise saturated addition.
	\param a first input pixel.
	\param b second input pixel.
	\return saturated addition pixel (add and saturate to 0xFF each channel independently).
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxlAdds(AMuint32 a,
												AMuint32 b) {

	AMuint32 t0 = (b ^ a) & 0x80808080;
	AMuint32 t1 = (b & a) & 0x80808080;

	a &= ~(0x80808080);
	b &= ~(0x80808080);
	a += b;
	t1 |= t0 & a;
	t1 = (t1 << 1) - (t1 >> 7);
	return (a ^ t0) | t1;
}

/*!
	\brief 32bit pixel (LSB alpha) component-wise saturated addition, excluding alpha.
	\param a first input pixel.
	\param b second input pixel.
	\return saturated addition pixel, with (LSB) alpha component set to 0.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxl_RGBA_AddsNoA(AMuint32 a,
														 AMuint32 b) {

	AMuint32 s, c;

	a >>= 8;
	b >>= 8;
	s = (a + b);
	c = (a ^ b ^ s) & 0x01010100;
	return (((s - c) | (c - (c >> 8))) << 8);
}

/*!
	\brief 32bit pixel (MSB alpha) component-wise saturated addition, excluding alpha.
	\param a first input pixel.
	\param b second input pixel.
	\return saturated addition pixel, with (MSB) alpha component set to 0.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxl_ARGB_AddsNoA(AMuint32 a,
														 AMuint32 b) {

	AMuint32 s, c;

	a &= 0x00FFFFFF;
	b &= 0x00FFFFFF;
	s = (a + b);
	c = (a ^ b ^ s) & 0x01010100;
	return (((s - c) | (c - (c >> 8))));
}

//! Alias for 32bit pixel (LSB alpha) component-wise saturated addition, excluding alpha.
#define amPxl_BGRA_AddsNoA amPxl_RGBA_AddsNoA
//! Alias for 32bit pixel (MSB alpha) component-wise saturated addition, excluding alpha.
#define amPxl_ABGR_AddsNoA amPxl_ARGB_AddsNoA


/*!
	\brief 32bit pixel component-wise minimum extraction, including alpha.
	\param a first input pixel.
	\param b second input pixel.
	\return a pixel with component-wise minimum components.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxlMin(const AMuint32 a,
											   const AMuint32 b) {

	AMuint32 x = a & 0xFF000000;
	AMuint32 y = b & 0xFF000000;
	AMuint32 w = y + ((x - y) & -(x < y)); // min(x, y)

	x = a & 0x00FF0000;
	y = b & 0x00FF0000;
	w |= y + ((x - y) & -(x < y)); // min(x, y)

	x = a & 0x0000FF00;
	y = b & 0x0000FF00;
	w |= y + ((x - y) & -(x < y)); // min(x, y)

	x = a & 0x000000FF;
	y = b & 0x000000FF;
	w |= y + ((x - y) & -(x < y)); // min(x, y)

	return w;
}

/*!
	\brief 32bit pixel (LSB alpha) component-wise minimum extraction, excluding alpha.
	\param a first input pixel.
	\param b second input pixel.
	\return a pixel with component-wise minimum components, with (LSB) alpha component set to 0.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxl_RGBA_MinNoA(const AMuint32 a,
														const AMuint32 b) {

	AMuint32 x = a & 0xFF000000;
	AMuint32 y = b & 0xFF000000;
	AMuint32 w = y + ((x - y) & -(x < y)); // min(x, y)

	x = a & 0x00FF0000;
	y = b & 0x00FF0000;
	w |= y + ((x - y) & -(x < y)); // min(x, y)

	x = a & 0x0000FF00;
	y = b & 0x0000FF00;
	w |= y + ((x - y) & -(x < y)); // min(x, y)
	return w;
}

/*!
	\brief 32bit pixel (MSB alpha) component-wise minimum extraction, excluding alpha.
	\param a first input pixel.
	\param b second input pixel.
	\return a pixel with component-wise minimum components, with (MSB) alpha component set to 0.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxl_ARGB_MinNoA(const AMuint32 a,
														const AMuint32 b) {

	AMuint32 x = a & 0x00FF0000;
	AMuint32 y = b & 0x00FF0000;
	AMuint32 w = y + ((x - y) & -(x < y)); // min(x, y)

	x = a & 0x0000FF00;
	y = b & 0x0000FF00;
	w |= y + ((x - y) & -(x < y)); // min(x, y)

	x = a & 0x000000FF;
	y = b & 0x000000FF;
	w |= y + ((x - y) & -(x < y)); // min(x, y)
	return w;
}

//! Alias for 32bit pixel (LSB alpha) component-wise minimum extraction, excluding alpha.
#define amPxl_BGRA_MinNoA amPxl_RGBA_MinNoA
//! Alias for 32bit pixel (MSB alpha) component-wise minimum extraction, excluding alpha.
#define amPxl_ABGR_MinNoA amPxl_ARGB_MinNoA

/*!
	\brief 32bit pixel component-wise maximum extraction, including alpha.
	\param a first input pixel.
	\param b second input pixel.
	\return a pixel with component-wise maximum components.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxlMax(const AMuint32 a,
											   const AMuint32 b) {

	AMuint32 x = a & 0xFF000000;
	AMuint32 y = b & 0xFF000000;
	AMuint32 w = x - ((x - y) & -(x < y)); // max(x, y)

	x = a & 0x00FF0000;
	y = b & 0x00FF0000;
	w |= x - ((x - y) & -(x < y)); // max(x, y)

	x = a & 0x0000FF00;
	y = b & 0x0000FF00;
	w |= x - ((x - y) & -(x < y)); // max(x, y)

	x = a & 0x000000FF;
	y = b & 0x000000FF;
	w |= x - ((x - y) & -(x < y)); // max(x, y)

	return w;
}

/*!
	\brief 32bit pixel (LSB alpha) component-wise maximum extraction, excluding alpha.
	\param a first input pixel.
	\param b second input pixel.
	\return a pixel with component-wise maximum components, with (LSB) alpha component set to 0.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxl_RGBA_MaxNoA(const AMuint32 a,
														const AMuint32 b) {

	AMuint32 x = a & 0xFF000000;
	AMuint32 y = b & 0xFF000000;
	AMuint32 w = x - ((x - y) & -(x < y)); // max(x, y)

	x = a & 0x00FF0000;
	y = b & 0x00FF0000;
	w |= x - ((x - y) & -(x < y)); // max(x, y)

	x = a & 0x0000FF00;
	y = b & 0x0000FF00;
	w |= x - ((x - y) & -(x < y)); // max(x, y)
	return w;
}

/*!
	\brief 32bit pixel (MSB alpha) component-wise maximum extraction, excluding alpha.
	\param a first input pixel.
	\param b second input pixel.
	\return a pixel with component-wise maximum components, with (MSB) alpha component set to 0.
*/
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amPxl_ARGB_MaxNoA(const AMuint32 a,
														const AMuint32 b) {

	AMuint32 x = a & 0x00FF0000;
	AMuint32 y = b & 0x00FF0000;
	AMuint32 w = x - ((x - y) & -(x < y)); // max(x, y)

	x = a & 0x0000FF00;
	y = b & 0x0000FF00;
	w |= x - ((x - y) & -(x < y)); // max(x, y)

	x = a & 0x000000FF;
	y = b & 0x000000FF;
	w |= x - ((x - y) & -(x < y)); // max(x, y)
	return w;
}

//! Alias for 32bit pixel (LSB alpha) component-wise maximum extraction, excluding alpha.
#define amPxl_BGRA_MaxNoA amPxl_RGBA_MaxNoA
//! Alias for 32bit pixel (MSB alpha) component-wise maximum extraction, excluding alpha.
#define amPxl_ABGR_MaxNoA amPxl_ARGB_MaxNoA

AM_INLINE AMuint32 amPxlUnpack565(const AMuint16 pixel565) {

#if defined(AM_SURFACE_BYTE_ORDER_RGBA) || defined(AM_SURFACE_BYTE_ORDER_BGRA)

	AMuint32 b32 = (pixel565 & 31) * 2106;
	AMuint32 g32 = ((((pixel565 >> 5) & 63) * 1037) << 8) & 0x00FF0000;
	AMuint32 r32 = ((((pixel565 >> 11) & 31) * 2106) << 16) & 0xFF000000;

	return ((0x000000FF) | (r32) | (g32) | (b32));
#else
	AMuint32 b32 = ((pixel565 & 31) * 2106) >> 8;
	AMuint32 g32 = (((pixel565 >> 5) & 63) * 1037) & 0x0000FF00;
	AMuint32 r32 = ((((pixel565 >> 11) & 31) * 2106) << 8) & 0x00FF0000;

	return ((0xFF000000) | (r32) | (g32) | (b32));
#endif
}

AM_INLINE AMuint16 amPxlPack565(const AMuint32 pixel32) {

#if defined(AM_SURFACE_BYTE_ORDER_RGBA) || defined(AM_SURFACE_BYTE_ORDER_BGRA)

	AMuint16 b16 = (AMuint16)((pixel32 >> 11) & 31);
	AMuint16 g16 = (AMuint16)((pixel32 >> 13) & 0x7E0);
	AMuint16 r16 = (AMuint16)((pixel32 >> 16) & 0xF800);

	return (r16) | (g16) | (b16);
#else
	AMuint16 b16 = (AMuint16)((pixel32 >> 3) & 31);
	AMuint16 g16 = (AMuint16)((pixel32 >> 5) & 0x7E0);
	AMuint16 r16 = (AMuint16)((pixel32 >> 8) & 0xF800);

	return (r16) | (g16) | (b16);
#endif
}

#endif
