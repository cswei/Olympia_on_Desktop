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

#ifndef _VGCONVERSIONS_H
#define _VGCONVERSIONS_H

/*!
	\file vgconversions.h
	\brief Pixelmap conversion routines, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "mathematics.h"

// High level function used to convert two (sub)images.
void amPxlMapConvert(void *dstPixels,
					 const VGImageFormat dstFormat,
					 const AMint32 dstDataStride,
					 const AMint32 dstX,
					 const AMint32 dstY,
					 const void *srcPixels,
					 const VGImageFormat srcFormat,
					 const AMint32 srcDataStride,
					 const AMint32 srcX,
					 const AMint32 srcY,
					 const AMint32 width,
					 const AMint32 height,
					 const AMbool dither,
					 const AMbool consistentSrc);

// [destination bits][source bits]
// used to map a [0; 2^N - 1] -> [0; 2^M - 1]
// y = x * (amBitConversionTable[N - 1][M - 1]) / 256
// where y belongs to [0; 2^N -1], x belongs to [0; 2^M - 1]
AM_EXTERN_C const AMuint32 amBitConversionTable[8][8];
//! Gamma table for conversion from linear (l) to non-linear (s) color space, one channel value.
AM_EXTERN_C const AMuint32 amGammaTable[256];
//! Inverse gamma table for conversion from non-linear (s) to linear (l) color space, one channel value.
AM_EXTERN_C const AMuint32 amGammaInvTable[256];
//! Table used to convert an 8 bit value to the corresponding 4 bit value.
AM_EXTERN_C const AMuint8 am8To4BitTable[256];
//! Table used to remove color pre-multiplication.
AM_EXTERN_C const AMuint32 amUnpremultiplyTable[256];

//! Linear to non-linear color space conversion, for values in the range [0; 255].
#define AM_GAMMA_TABLE(_index) (amGammaTable[_index])
//! Non-linear to linear color space conversion, for values in the range [0; 255].
#define AM_GAMMA_INV_TABLE(_index) (amGammaInvTable[_index])

/*!
	\brief Linear to non-linear color space conversion, for values in the range [0.0f; 1.0f].
	\param value input value in the linear color space.
	\return the corresponding value in non-linear color space.
	\pre 0.0f <= value <= 1.0f
*/
AM_INLINE AMfloat amGammaConversion(const AMfloat value) {

	static const AMfloat gammaExp = 1.0f / 2.4f;

	AM_ASSERT(value >= 0.0f && value <= 1.0f);

	return (value <= 0.00304f) ? (value * 12.92f) : (1.0556f * amPowf(value, gammaExp) - 0.0556f);
}

/*!
	\brief Non-linear to linear color space conversion, for values in the range [0.0f; 1.0f].
	\param value input value in the non-linear color space.
	\return the corresponding value in linear color space.
	\pre 0.0f <= value <= 1.0f
*/
AM_INLINE AMfloat amGammaInvConversion(const AMfloat value){

	static const AMfloat invGammaExp = 2.4f;

	AM_ASSERT(value >= 0.0f && value <= 1.0f);

	return (value <= 0.003928f) ? (value / 12.92f) : amPowf((value + 0.0556f) / 1.0556f, invGammaExp);
}

//! Remove alpha premultiplication from the given red, green and blue values.
#define AM_UNPREMULTIPLY(_outR, _outG, _outB, _inR, _inG, _inB, _inA) { \
	AMuint32 unpreA = amUnpremultiplyTable[(_inA)]; \
	_outR = (unpreA * (_inR)) >> 23; \
	_outG = (unpreA * (_inG)) >> 23; \
	_outB = (unpreA * (_inB)) >> 23; \
}

#endif
