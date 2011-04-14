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

/*!
	\file vgfilters.c
	\brief Image filters, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgconversions.h"
#include "vg_priv.h"
#include "vgcontext.h"


#if defined RIM_VG_SRC
#define VG_API_ENTRY 
#endif

#if !defined(AM_LITE_PROFILE)

//***************************************************************************************
//    Conversion a single pixel from (sRGBA, sRGBApre, lRGBA, lRGBApre) to another format
//***************************************************************************************

/*!
	\brief Convert a single pixel from s8888 formats to s888X formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_s8888Tos888X(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)a;
	(void)x;

	if (!(colMask & VG_RED))
		r = (*dst >> rShift) & 0xFF;
	if (!(colMask & VG_GREEN))
		g = (*dst >> gShift) & 0xFF;
	if (!(colMask & VG_BLUE))
		b = (*dst >> bShift) & 0xFF;

	// write the destination pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | ((AMuint32)0xFF << aShift);
}

/*!
	\brief Convert a single pixel from s8888Pre formats to s888X formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_s8888PreTos888X(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	AMuint32 unpreA = amUnpremultiplyTable[a];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	// remove alpha
	r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : (unpreA * r) >> 23;
	g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : (unpreA * g) >> 23;
	b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : (unpreA * b) >> 23;

	// write the destination pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | ((AMuint32)0xFF << aShift);
}

/*!
	\brief Convert a single pixel from l8888 formats to s888X formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_l8888Tos888X(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)a;
	(void)x;

	// use gamma function to convert from linear color space to non-linear color space
	r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : AM_GAMMA_TABLE(r);
	g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : AM_GAMMA_TABLE(g);
	b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : AM_GAMMA_TABLE(b);

	// write the destination pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | ((AMuint32)0xFF << aShift);
}

/*!
	\brief Convert a single pixel from l8888Pre formats to s888X formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_l8888PreTos888X(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	if (a == 0) {
		r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : 0x00;
		g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : 0x00;
		b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : 0x00;

		// write the destination pixel
		*dst = (r << rShift) | (g << gShift) | (b << bShift) | ((AMuint32)0xFF << aShift);
	}
	else {
		AMuint32 unpreA = amUnpremultiplyTable[a];

		// remove alpha
		if (!(colMask & VG_RED))
			r = (*dst >> rShift) & 0xFF;
		else {
			r = (unpreA * r) >> 23;
			r = AM_GAMMA_TABLE(r);
		}
		if (!(colMask & VG_GREEN))
			g = (*dst >> gShift) & 0xFF;
		else {
			g = (unpreA * g) >> 23;
			g = AM_GAMMA_TABLE(g);
		}
		if (!(colMask & VG_BLUE))
			b = (*dst >> bShift) & 0xFF;
		else {
			b = (unpreA * b) >> 23;
			b = AM_GAMMA_TABLE(b);
		}

		// write the destination pixel
		*dst = (r << rShift) | (g << gShift) | (b << bShift) | ((AMuint32)0xFF << aShift);
	}
}

/*!
	\brief Convert a single pixel from s8888 formats to s8888 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_s8888Tos8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)x;

	if (!(colMask & VG_RED))
		r = (*dst >> rShift) & 0xFF;
	if (!(colMask & VG_GREEN))
		g = (*dst >> gShift) & 0xFF;
	if (!(colMask & VG_BLUE))
		b = (*dst >> bShift) & 0xFF;
	if (!(colMask & VG_ALPHA))
		a = (*dst >> aShift) & 0xFF;
	// write the destination pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
}

/*!
	\brief Convert a single pixel from s8888Pre formats to s8888 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_s8888PreTos8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	AMuint32 unpreA = amUnpremultiplyTable[a];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	// remove alpha
	r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : (unpreA * r) >> 23;
	g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : (unpreA * g) >> 23;
	b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : (unpreA * b) >> 23;
	if (!(colMask & VG_ALPHA))
		a = (*dst >> aShift) & 0xFF;
	// write the destination pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
}

/*!
	\brief Convert a single pixel from l8888 formats to s8888 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_l8888Tos8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)x;

	// use gamma function to convert from linear color space to non-linear color space
	r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : AM_GAMMA_TABLE(r);
	g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : AM_GAMMA_TABLE(g);
	b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : AM_GAMMA_TABLE(b);
	if (!(colMask & VG_ALPHA))
		a = (*dst >> aShift) & 0xFF;
	// write the destination pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
}

/*!
	\brief Convert a single pixel from l8888Pre formats to s8888 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_l8888PreTos8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	if (a == 0) {
		r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : 0x00;
		g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : 0x00;
		b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : 0x00;
		if (!(colMask & VG_ALPHA))
			a = (*dst >> aShift) & 0xFF;
		// write the destination pixel
		*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
	}
	else {
		AMuint32 unpreA = amUnpremultiplyTable[a];

		// remove alpha
		if (!(colMask & VG_RED))
			r = (*dst >> rShift) & 0xFF;
		else {
			r = (unpreA * r) >> 23;
			r = AM_GAMMA_TABLE(r);
		}
		if (!(colMask & VG_GREEN))
			g = (*dst >> gShift) & 0xFF;
		else {
			g = (unpreA * g) >> 23;
			g = AM_GAMMA_TABLE(g);
		}
		if (!(colMask & VG_BLUE))
			b = (*dst >> bShift) & 0xFF;
		else {
			b = (unpreA * b) >> 23;
			b = AM_GAMMA_TABLE(b);
		}
		if (!(colMask & VG_ALPHA))
			a = (*dst >> aShift) & 0xFF;
		// write the destination pixel
		*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
	}
}

/*!
	\brief Convert a single pixel from s8888 formats to s8888Pre formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_s8888Tos8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)x;

	// multiply alpha
	if (!(colMask & VG_RED))
		r = (*dst >> rShift) & 0xFF;
	else {
		MULT_DIV_255(r, r, a)
	}
	if (!(colMask & VG_GREEN))
		g = (*dst >> gShift) & 0xFF;
	else {
		MULT_DIV_255(g, g, a)
	}
	if (!(colMask & VG_BLUE))
		b = (*dst >> bShift) & 0xFF;
	else {
		MULT_DIV_255(b, b, a)
	}
	if (!(colMask & VG_ALPHA))
		a = (*dst >> aShift) & 0xFF;
	// write pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
}

/*!
	\brief Convert a single pixel from s8888Pre formats to s8888Pre formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_s8888PreTos8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	if (!(colMask & VG_RED))
		r = (*dst >> rShift) & 0xFF;
	if (!(colMask & VG_GREEN))
		g = (*dst >> gShift) & 0xFF;
	if (!(colMask & VG_BLUE))
		b = (*dst >> bShift) & 0xFF;
	if (!(colMask & VG_ALPHA))
		a = (*dst >> aShift) & 0xFF;
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
}

/*!
	\brief Convert a single pixel from l8888 formats to s8888Pre formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_l8888Tos8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)x;

	if (!(colMask & VG_RED))
		r = (*dst >> rShift) & 0xFF;
	else {
		r = AM_GAMMA_TABLE(r);
		MULT_DIV_255(r, r, a)
	}
	if (!(colMask & VG_GREEN))
		g = (*dst >> gShift) & 0xFF;
	else {
		g = AM_GAMMA_TABLE(g);
		MULT_DIV_255(g, g, a)
	}
	if (!(colMask & VG_BLUE))
		b = (*dst >> bShift) & 0xFF;
	else {
		b = AM_GAMMA_TABLE(b);
		MULT_DIV_255(b, b, a)
	}
	if (!(colMask & VG_ALPHA))
		a = (*dst >> aShift) & 0xFF;
	// write pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
}

/*!
	\brief Convert a single pixel from l8888Pre formats to s8888Pre formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_l8888PreTos8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	if (a == 0) {
		r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : 0x00;
		g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : 0x00;
		b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : 0x00;
		if (!(colMask & VG_ALPHA))
			a = (*dst >> aShift) & 0xFF;
		// write pixel
		*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
	}
	else {
		AMuint32 unpreA = amUnpremultiplyTable[a];

		if (!(colMask & VG_RED))
			r = (*dst >> rShift) & 0xFF;
		else {
			r = (unpreA * r) >> 23;
			r = AM_GAMMA_TABLE(r);
			MULT_DIV_255(r, r, a)
		}
		if (!(colMask & VG_GREEN))
			g = (*dst >> gShift) & 0xFF;
		else {
			g = (unpreA * g) >> 23;
			g = AM_GAMMA_TABLE(g);
			MULT_DIV_255(g, g, a)
		}
		if (!(colMask & VG_BLUE))
			b = (*dst >> bShift) & 0xFF;
		else {
			b = (unpreA * b) >> 23;
			b = AM_GAMMA_TABLE(b);
			MULT_DIV_255(b, b, a)
		}
		if (!(colMask & VG_ALPHA))
			a = (*dst >> aShift) & 0xFF;
		// write pixel
		*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
	}
}

/*!
	\brief Convert a single pixel from s8888 formats to s565 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_s8888Tos565(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint16 *dst = (AMuint16 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint16 r16, g16, b16;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)a;
	(void)x;

	r16 = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0x1F : (AMuint16)(r >> 3);
	g16 = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0x3F : (AMuint16)(g >> 2);
	b16 = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0x1F : (AMuint16)(b >> 3);
	// write the destination pixel
	*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift);
}

/*!
	\brief Convert a single pixel from s8888Pre formats to s565 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_s8888PreTos565(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint16 *dst = (AMuint16 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 unpreA = amUnpremultiplyTable[a];
	AMuint16 r16, g16, b16;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	// remove alpha
	r16 = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0x1F : (AMuint16)((unpreA * r) >> 26);
	g16 = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0x3F : (AMuint16)((unpreA * g) >> 25);
	b16 = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0x1F : (AMuint16)((unpreA * b) >> 26);
	// write the destination pixel
	*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift);
}

/*!
	\brief Convert a single pixel from l8888 formats to s565 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_l8888Tos565(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint16 *dst = (AMuint16 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint16 r16, g16, b16;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)a;
	(void)x;

	// use gamma function to convert from linear color space to non-linear color space
	if (!(colMask & VG_RED))
		r16 = (*dst >> rShift) & 0x1F;
	else {
		r = AM_GAMMA_TABLE(r);
		r16 = (AMuint16)(r >> 3);
	}
	if (!(colMask & VG_GREEN))
		g16 = (*dst >> gShift) & 0x3F;
	else {
		g = AM_GAMMA_TABLE(g);
		g16 = (AMuint16)(g >> 2);
	}
	if (!(colMask & VG_BLUE))
		b16 = (*dst >> bShift) & 0x1F;
	else {
		b = AM_GAMMA_TABLE(b);
		b16 = (AMuint16)(b >> 3);
	}
	// write the destination pixel
	*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift);
}

/*!
	\brief Convert a single pixel from l8888Pre formats to s565 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_l8888PreTos565(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint16 *dst = (AMuint16 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint16 r16, g16, b16;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	if (a == 0) {
		r16 = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0x1F : 0x00;
		g16 = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0x3F : 0x00;
		b16 = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0x1F : 0x00;
		// write the destination pixel
		*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift);
	}
	else {
		AMuint32 unpreA = amUnpremultiplyTable[a];

		// remove alpha
		if (!(colMask & VG_RED))
			r16 = (*dst >> rShift) & 0x1F;
		else {
			r = (unpreA * r) >> 23;
			r = AM_GAMMA_TABLE(r);
			r16 = (AMuint16)(r >> 3);
		}
		if (!(colMask & VG_GREEN))
			g16 = (*dst >> gShift) & 0x3F;
		else {
			g = (unpreA * g) >> 23;
			g = AM_GAMMA_TABLE(g);
			g16 = (AMuint16)(g >> 2);
		}
		if (!(colMask & VG_BLUE))
			b16 = (*dst >> bShift) & 0x1F;
		else {
			b = (unpreA * b) >> 23;
			b = AM_GAMMA_TABLE(b);
			b16 = (AMuint16)(b >> 3);
		}
		// write the destination pixel
		*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift);
	}
}

/*!
	\brief Convert a single pixel from s8888 formats to s5551 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_s8888Tos5551(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint16 *dst = (AMuint16 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	AMuint16 r16, g16, b16, a16;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)x;

	r16 = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0x1F : (AMuint16)(r >> 3);
	g16 = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0x1F : (AMuint16)(g >> 3);
	b16 = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0x1F : (AMuint16)(b >> 3);
	a16 = (!(colMask & VG_ALPHA)) ? (*dst >> aShift) & 0x01 : (AMuint16)(a >> 7);
	// write pixel
	*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift) | (a16 << aShift);
}

/*!
	\brief Convert a single pixel from s8888Pre formats to s5551 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_s8888PreTos5551(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint16 *dst = (AMuint16 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	AMuint32 unpreA = amUnpremultiplyTable[a];
	AMuint16 r16, g16, b16, a16;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	// remove alpha
	r16 = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0x1F : (AMuint16)((unpreA * r) >> 26);
	g16 = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0x1F : (AMuint16)((unpreA * g) >> 26);
	b16 = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0x1F : (AMuint16)((unpreA * b) >> 26);
	a16 = (!(colMask & VG_ALPHA)) ? (*dst >> aShift) & 0x01 : (AMuint16)(a >> 7);
	// write pixel
	*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift) | (a16 << aShift);
}

/*!
	\brief Convert a single pixel from l8888 formats to s5551 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_l8888Tos5551(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint16 *dst = (AMuint16 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	AMuint16 r16, g16, b16, a16;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)x;

	// use gamma function to convert from linear color space to non-linear color space
	if (!(colMask & VG_RED))
		r16 = (*dst >> rShift) & 0x1F;
	else {
		r = AM_GAMMA_TABLE(r);
		r16 = (AMuint16)(r >> 3);
	}
	if (!(colMask & VG_GREEN))
		g16 = (*dst >> gShift) & 0x1F;
	else {
		g = AM_GAMMA_TABLE(g);
		g16 = (AMuint16)(g >> 3);
	}
	if (!(colMask & VG_BLUE))
		b16 = (*dst >> bShift) & 0x1F;
	else {
		b = AM_GAMMA_TABLE(b);
		b16 = (AMuint16)(b >> 3);
	}
	a16 = (!(colMask & VG_ALPHA)) ? (*dst >> aShift) & 0x01 : (AMuint16)(a >> 7);
	// write pixel
	*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift) | (a16 << aShift);
}

/*!
	\brief Convert a single pixel from l8888Pre formats to s5551 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_l8888PreTos5551(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint16 *dst = (AMuint16 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	AMuint16 r16, g16, b16, a16;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	if (a == 0) {
		r16 = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0x1F : 0x00;
		g16 = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0x1F : 0x00;
		b16 = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0x1F : 0x00;
		a16 = (!(colMask & VG_ALPHA)) ? (*dst >> aShift) & 0x01 : 0x00;
		// write pixel
		*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift) | (a16 << aShift);
	}
	else {
		AMuint32 unpreA = amUnpremultiplyTable[a];

		// remove alpha
		if (!(colMask & VG_RED))
			r16 = (*dst >> rShift) & 0x1F;
		else {
			r = (unpreA * r) >> 23;
			r = AM_GAMMA_TABLE(r);
			r16 = (AMuint16)(r >> 3);
		}
		if (!(colMask & VG_GREEN))
			g16 = (*dst >> gShift) & 0x1F;
		else {
			g = (unpreA * g) >> 23;
			g = AM_GAMMA_TABLE(g);
			g16 = (AMuint16)(g >> 3);
		}
		if (!(colMask & VG_BLUE))
			b16 = (*dst >> bShift) & 0x1F;
		else {
			b = (unpreA * b) >> 23;
			b = AM_GAMMA_TABLE(b);
			b16 = (AMuint16)(b >> 3);
		}
		a16 = (!(colMask & VG_ALPHA)) ? (*dst >> aShift) & 0x01 : (AMuint16)(a >> 7);
		// write pixel
		*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift) | (a16 << aShift);
	}
}

/*!
	\brief Convert a single pixel from s8888 formats to s4444 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_s8888Tos4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint16 *dst = (AMuint16 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	AMuint16 r16, g16, b16, a16;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)x;

	r16 = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0x0F : (AMuint16)(r >> 4);
	g16 = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0x0F : (AMuint16)(g >> 4);
	b16 = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0x0F : (AMuint16)(b >> 4);
	a16 = (!(colMask & VG_ALPHA)) ? (*dst >> aShift) & 0x0F : (AMuint16)(a >> 4);
	// write pixel
	*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift) | (a16 << aShift);
}

/*!
	\brief Convert a single pixel from s8888Pre formats to s4444 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_s8888PreTos4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint16 *dst = (AMuint16 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	AMuint32 unpreA = amUnpremultiplyTable[a];
	AMuint16 r16, g16, b16, a16;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	// remove alpha
	r16 = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0x0F : (AMuint16)((unpreA * r) >> 27);
	g16 = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0x0F : (AMuint16)((unpreA * g) >> 27);
	b16 = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0x0F : (AMuint16)((unpreA * b) >> 27);
	a16 = (!(colMask & VG_ALPHA)) ? (*dst >> aShift) & 0x0F : (AMuint16)(a >> 4);
	// write pixel
	*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift) | (a16 << aShift);
}

/*!
	\brief Convert a single pixel from l8888 formats to s4444 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_l8888Tos4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint16 *dst = (AMuint16 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	AMuint16 r16, g16, b16, a16;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)x;

	// use gamma function to convert from linear color space to non-linear color space
	if (!(colMask & VG_RED))
		r16 = (*dst >> rShift) & 0x0F;
	else {
		r = AM_GAMMA_TABLE(r);
		r16 = (AMuint16)(r >> 4);
	}
	if (!(colMask & VG_GREEN))
		g16 = (*dst >> gShift) & 0x0F;
	else {
		g = AM_GAMMA_TABLE(g);
		g16 = (AMuint16)(g >> 4);
	}
	if (!(colMask & VG_BLUE))
		b16 = (*dst >> bShift) & 0x0F;
	else {
		b = AM_GAMMA_TABLE(b);
		b16 = (AMuint16)(b >> 4);
	}
	a16 = (!(colMask & VG_ALPHA)) ? (*dst >> aShift) & 0x0F : (AMuint16)(a >> 4);
	// write pixel
	*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift) | (a16 << aShift);
}

/*!
	\brief Convert a single pixel from l8888Pre formats to s4444 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_l8888PreTos4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint16 *dst = (AMuint16 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	AMuint16 r16, g16, b16, a16;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	if (a == 0) {
		r16 = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0x0F : 0x00;
		g16 = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0x0F : 0x00;
		b16 = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0x0F : 0x00;
		a16 = (!(colMask & VG_ALPHA)) ? (*dst >> aShift) & 0x0F: 0x00;
		// write pixel
		*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift) | (a16 << aShift);
	}
	else {
		AMuint32 unpreA = amUnpremultiplyTable[a];

		// remove alpha
		if (!(colMask & VG_RED))
			r16 = (*dst >> rShift) & 0x0F;
		else {
			r = (unpreA * r) >> 23;
			r = AM_GAMMA_TABLE(r);
			r16 = (AMuint16)(r >> 4);
		}
		if (!(colMask & VG_GREEN))
			g16 = (*dst >> gShift) & 0x0F;
		else {
			g = (unpreA * g) >> 23;
			g = AM_GAMMA_TABLE(g);
			g16 = (AMuint16)(g >> 4);
		}
		if (!(colMask & VG_BLUE))
			b16 = (*dst >> bShift) & 0x0F;
		else {
			b = (unpreA * b) >> 23;
			b = AM_GAMMA_TABLE(b);
			b16 = (AMuint16)(b >> 4);
		}
		a16 = (!(colMask & VG_ALPHA)) ? (*dst >> aShift) & 0x0F : (AMuint16)(a >> 4);
		// write pixel
		*dst = (r16 << rShift) | (g16 << gShift) | (b16 << bShift) | (a16 << aShift);
	}
}

/*!
	\brief Convert a single pixel from s8888 formats to l888X formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_s8888Tol888X(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)a;
	(void)x;

	// use inverse gamma function to convert from non-linear color space to linear color space
	r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : AM_GAMMA_INV_TABLE(r);
	g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : AM_GAMMA_INV_TABLE(g);
	b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : AM_GAMMA_INV_TABLE(b);

	// write pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | ((AMuint32)0xFF << aShift);
}

/*!
	\brief Convert a single pixel from s8888Pre formats to l888X formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_s8888PreTol888X(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	if (a == 0) {
		r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : 0x00;
		g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : 0x00;
		b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : 0x00;

		// write pixel
		*dst = (r << rShift) | (g << gShift) | (b << bShift) | ((AMuint32)0xFF << aShift);
	}
	else {
		AMuint32 unpreA = amUnpremultiplyTable[a];

		if (!(colMask & VG_RED))
			r = (*dst >> rShift) & 0xFF;
		else {
			r = (unpreA * r) >> 23;
			r = AM_GAMMA_INV_TABLE(r);
		}
		if (!(colMask & VG_GREEN))
			g = (*dst >> gShift) & 0xFF;
		else {
			g = (unpreA * g) >> 23;
			g = AM_GAMMA_INV_TABLE(g);
		}
		if (!(colMask & VG_BLUE))
			b = (*dst >> bShift) & 0xFF;
		else {
			b = (unpreA * b) >> 23;
			b = AM_GAMMA_INV_TABLE(b);
		}

		// write pixel
		*dst = (r << rShift) | (g << gShift) | (b << bShift) | ((AMuint32)0xFF << aShift);
	}
}

/*!
	\brief Convert a single pixel from l8888 formats to l888X formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_l8888Tol888X(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)a;
	(void)x;

	if (!(colMask & VG_RED))
		r = (*dst >> rShift) & 0xFF;
	if (!(colMask & VG_GREEN))
		g = (*dst >> gShift) & 0xFF;
	if (!(colMask & VG_BLUE))
		b = (*dst >> bShift) & 0xFF;

	*dst = (r << rShift) | (g << gShift) | (b << bShift) | ((AMuint32)0xFF << aShift);
}

/*!
	\brief Convert a single pixel from l8888Pre formats to l888X formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_l8888PreTol888X(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	AMuint32 unpreA = amUnpremultiplyTable[a];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : (unpreA * r) >> 23;
	g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : (unpreA * g) >> 23;
	b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : (unpreA * b) >> 23;

	// write pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | ((AMuint32)0xFF << aShift);
}

/*!
	\brief Convert a single pixel from s8888 formats to l8888 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_s8888Tol8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)x;

	// use inverse gamma function to convert from non-linear color space to linear color space
	r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : AM_GAMMA_INV_TABLE(r);
	g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : AM_GAMMA_INV_TABLE(g);
	b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : AM_GAMMA_INV_TABLE(b);
	if (!(colMask & VG_ALPHA))
		a = (*dst >> aShift) & 0xFF;
	// write pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
}

/*!
	\brief Convert a single pixel from s8888Pre formats to l8888 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_s8888PreTol8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	if (a == 0) {
		r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : 0x00;
		g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : 0x00;
		b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : 0x00;
		if (!(colMask & VG_ALPHA))
			a = (*dst >> aShift) & 0xFF;
		// write pixel
		*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
	}
	else {
		AMuint32 unpreA = amUnpremultiplyTable[a];

		if (!(colMask & VG_RED))
			r = (*dst >> rShift) & 0xFF;
		else {
			r = (unpreA * r) >> 23;
			r = AM_GAMMA_INV_TABLE(r);
		}
		if (!(colMask & VG_GREEN))
			g = (*dst >> gShift) & 0xFF;
		else {
			g = (unpreA * g) >> 23;
			g = AM_GAMMA_INV_TABLE(g);
		}
		if (!(colMask & VG_BLUE))
			b = (*dst >> bShift) & 0xFF;
		else {
			b = (unpreA * b) >> 23;
			b = AM_GAMMA_INV_TABLE(b);
		}
		if (!(colMask & VG_ALPHA))
			a = (*dst >> aShift) & 0xFF;
		// write pixel
		*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
	}
}

/*!
	\brief Convert a single pixel from l8888 formats to l8888 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_l8888Tol8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)x;

	if (!(colMask & VG_RED))
		r = (*dst >> rShift) & 0xFF;
	if (!(colMask & VG_GREEN))
		g = (*dst >> gShift) & 0xFF;
	if (!(colMask & VG_BLUE))
		b = (*dst >> bShift) & 0xFF;
	if (!(colMask & VG_ALPHA))
		a = (*dst >> aShift) & 0xFF;
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
}

/*!
	\brief Convert a single pixel from l8888Pre formats to l8888 formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_l8888PreTol8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	AMuint32 unpreA = amUnpremultiplyTable[a];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : (unpreA * r) >> 23;
	g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : (unpreA * g) >> 23;
	b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : (unpreA * b) >> 23;
	if (!(colMask & VG_ALPHA))
		a = (*dst >> aShift) & 0xFF;
	// write pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
}

/*!
	\brief Convert a single pixel from s8888 formats to l8888Pre formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_s8888Tol8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)x;

	// use inverse gamma function to convert from non-linear color space to linear color space
	if (!(colMask & VG_RED))
		r = (*dst >> rShift) & 0xFF;
	else {
		r = AM_GAMMA_INV_TABLE(r);
		MULT_DIV_255(r, r, a)
	}
	if (!(colMask & VG_GREEN))
		g = (*dst >> gShift) & 0xFF;
	else {
		g = AM_GAMMA_INV_TABLE(g);
		MULT_DIV_255(g, g, a)
	}
	if (!(colMask & VG_BLUE))
		b = (*dst >> bShift) & 0xFF;
	else {
		b = AM_GAMMA_INV_TABLE(b);
		MULT_DIV_255(b, b, a)
	}
	if (!(colMask & VG_ALPHA))
		a = (*dst >> aShift) & 0xFF;
	// write pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
}

/*!
	\brief Convert a single pixel from s8888Pre formats to l8888Pre formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_s8888PreTol8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	if (a == 0) {
		// remove alpha
		r = (!(colMask & VG_RED)) ? (*dst >> rShift) & 0xFF : 0x00;
		g = (!(colMask & VG_GREEN)) ? (*dst >> gShift) & 0xFF : 0x00;
		b = (!(colMask & VG_BLUE)) ? (*dst >> bShift) & 0xFF : 0x00;
		if (!(colMask & VG_ALPHA))
			a = (*dst >> aShift) & 0xFF;
		// write pixel
		*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
	}
	else {
		AMuint32 unpreA = amUnpremultiplyTable[a];

		// remove alpha
		if (!(colMask & VG_RED))
			r = (*dst >> rShift) & 0xFF;
		else {
			r = (unpreA * r) >> 23;
			r = AM_GAMMA_INV_TABLE(r);
			MULT_DIV_255(r, r, a)
		}
		if (!(colMask & VG_GREEN))
			g = (*dst >> gShift) & 0xFF;
		else {
			g = (unpreA * g) >> 23;
			g = AM_GAMMA_INV_TABLE(g);
			MULT_DIV_255(g, g, a)
		}
		if (!(colMask & VG_BLUE))
			b = (*dst >> bShift) & 0xFF;
		else {
			b = (unpreA * b) >> 23;
			b = AM_GAMMA_INV_TABLE(b);
			MULT_DIV_255(b, b, a)
		}
		if (!(colMask & VG_ALPHA))
			a = (*dst >> aShift) & 0xFF;
		// write pixel
		*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
	}
}

/*!
	\brief Convert a single pixel from l8888 formats to l8888Pre formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_l8888Tol8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)x;

	// multiply alpha
	if (!(colMask & VG_RED))
		r = (*dst >> rShift) & 0xFF;
	else {
		MULT_DIV_255(r, r, a)
	}
	if (!(colMask & VG_GREEN))
		g = (*dst >> gShift) & 0xFF;
	else {
		MULT_DIV_255(g, g, a)
	}
	if (!(colMask & VG_BLUE))
		b = (*dst >> bShift) & 0xFF;
	else {
		MULT_DIV_255(b, b, a)
	}
	if (!(colMask & VG_ALPHA))
		a = (*dst >> aShift) & 0xFF;
	// write pixel
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
}

/*!
	\brief Convert a single pixel from l8888Pre formats to l8888Pre formats.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\param dstFormat format of the destination pixel.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_l8888PreTol8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask, const VGImageFormat dstFormat) {

	AMuint32 *dst = (AMuint32 *)dstPixel;
	AMuint32 tableIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	AMuint32 gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	AMuint32 bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	AMuint32 aShift = pxlFormatTable[tableIdx][FMT_A_SH];

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	// assert premultiplied rgb values are valid
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;

	if (!(colMask & VG_RED))
		r = (*dst >> rShift) & 0xFF;
	if (!(colMask & VG_GREEN))
		g = (*dst >> gShift) & 0xFF;
	if (!(colMask & VG_BLUE))
		b = (*dst >> bShift) & 0xFF;
	if (!(colMask & VG_ALPHA))
		a = (*dst >> aShift) & 0xFF;
	*dst = (r << rShift) | (g << gShift) | (b << bShift) | (a << aShift);
}


//*************************************************************
//          High level pixel conversion functions
//*************************************************************

// conversion to sRGBX8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to sRGBX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosRGBX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos888X(dstPixel, r, g, b, a, x, colMask, VG_sRGBX_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sRGBX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosRGBX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos888X(dstPixel, r, g, b, a, x, colMask, VG_sRGBX_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sRGBX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosRGBX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos888X(dstPixel, r, g, b, a, x, colMask, VG_sRGBX_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sRGBX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosRGBX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos888X(dstPixel, r, g, b, a, x, colMask, VG_sRGBX_8888);
}


// conversion to sRGBA8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to sRGBA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosRGBA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos8888(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sRGBA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosRGBA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos8888(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sRGBA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosRGBA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos8888(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sRGBA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosRGBA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos8888(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_8888);
}



// conversion to sRGBA8888Pre

/*!
	\brief Convert a single pixel from sRGBA8888 format to sRGBA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosRGBA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_8888_PRE);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sRGBA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosRGBA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sRGBA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosRGBA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sRGBA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosRGBA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a,	const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_8888_PRE);
}



// conversion to sRGB565

/*!
	\brief Convert a single pixel from sRGBA8888 format to sRGB565 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosRGB565(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a,	const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos565(dstPixel, r, g, b, a, x, colMask, VG_sRGB_565);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sRGB565 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosRGB565(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos565(dstPixel, r, g, b, a, x, colMask, VG_sRGB_565);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sRGB565 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosRGB565(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a,	const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos565(dstPixel, r, g, b, a, x, colMask, VG_sRGB_565);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sRGB565 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosRGB565(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos565(dstPixel, r, g, b, a, x, colMask, VG_sRGB_565);
}



// conversion to sRGBA5551

/*!
	\brief Convert a single pixel from sRGBA8888 format to sRGBA5551 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosRGBA5551(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos5551(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_5551);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sRGBA5551 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosRGBA5551(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos5551(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_5551);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sRGBA5551 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosRGBA5551(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos5551(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_5551);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sRGBA5551 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosRGBA5551(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos5551(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_5551);
}



// conversion to sRGBA4444

/*!
	\brief Convert a single pixel from sRGBA8888 format to sRGBA4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosRGBA4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos4444(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_4444);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sRGBA4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosRGBA4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos4444(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_4444);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sRGBA4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosRGBA4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos4444(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_4444);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sRGBA4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosRGBA4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos4444(dstPixel, r, g, b, a, x, colMask, VG_sRGBA_4444);
}



// conversion to sL8

/*!
	\brief Convert a single pixel from sRGBA8888 format to sL8 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosL8(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;
	AMuint8 lum;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)a;
	(void)x;
	(void)colMask;

	// use inverse gamma function to convert from non-linear color space to linear color space
	r = AM_GAMMA_INV_TABLE(r);
	g = AM_GAMMA_INV_TABLE(g);
	b = AM_GAMMA_INV_TABLE(b);
	// calculate luminance
	r *= 13933;	// 13933 = 0.2126 * 65536
	g *= 46871;	// 46871 = 0.7152 * 65536
	b *= 4732;	// 4732 = 0.0722 * 65536
	lum = (AMuint8)((r + g + b) >> 16);
	// use gamma function to convert from linear color space to non-linear color space
	lum = (AMuint8)(AM_GAMMA_TABLE(lum));
	// write pixel
	*dst = lum;
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sL8 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosL8(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;
	AMuint8 lum;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;
	(void)colMask;

	if (a == 0)
		*dst = 0;
	else {
		// remove alpha
		AM_UNPREMULTIPLY(r, g, b, r, g, b, a)
		// use inverse gamma function to convert from non-linear color space to linear color space
		r = AM_GAMMA_INV_TABLE(r);
		g = AM_GAMMA_INV_TABLE(g);
		b = AM_GAMMA_INV_TABLE(b);
		// calculate luminance
		r *= 13933;	// 13933 = 0.2126 * 65536
		g *= 46871;	// 46871 = 0.7152 * 65536
		b *= 4732;	// 4732 = 0.0722 * 65536
		lum = (AMuint8)((r + g + b) >> 16);
		// use gamma function to convert from linear color space to non-linear color space
		lum = (AMuint8)(AM_GAMMA_TABLE(lum));
		// write pixel
		*dst = lum;
	}
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sL8 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosL8(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;
	AMuint8 lum;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)a;
	(void)x;
	(void)colMask;

	// calculate luminance
	r *= 13933;	// 13933 = 0.2126 * 65536
	g *= 46871;	// 46871 = 0.7152 * 65536
	b *= 4732;	// 4732 = 0.0722 * 65536
	lum = (AMuint8)((r + g + b) >> 16);
	// use gamma function to convert from linear color space to non-linear color space
	lum = (AMuint8)(AM_GAMMA_TABLE(lum));
	// write pixel
	*dst = lum;
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sL8 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosL8(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;
	AMuint8 lum;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;
	(void)colMask;

	if (a == 0)
		*dst = 0;
	else {
		// remove alpha
		AM_UNPREMULTIPLY(r, g, b, r, g, b, a)
		// calculate luminance
		r *= 13933;	// 13933 = 0.2126 * 65536
		g *= 46871;	// 46871 = 0.7152 * 65536
		b *= 4732;	// 4732 = 0.0722 * 65536
		lum = (AMuint8)((r + g + b) >> 16);
		// use gamma function to convert from linear color space to non-linear color space
		lum = (AMuint8)(AM_GAMMA_TABLE(lum));
		// write pixel
		*dst = lum;
	}
}



// conversion to lRGBX8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to lRGBX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolRGBX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tol888X(dstPixel, r, g, b, a, x, colMask, VG_lRGBX_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lRGBX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolRGBX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTol888X(dstPixel, r, g, b, a, x, colMask, VG_lRGBX_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lRGBX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolRGBX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tol888X(dstPixel, r, g, b, a, x, colMask, VG_lRGBX_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lRGBX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolRGBX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTol888X(dstPixel, r, g, b, a, x, colMask, VG_lRGBX_8888);
}



// conversion to lRGBA8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to lRGBA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolRGBA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tol8888(dstPixel, r, g, b, a, x, colMask, VG_lRGBA_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lRGBA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolRGBA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTol8888(dstPixel, r, g, b, a, x, colMask, VG_lRGBA_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lRGBA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolRGBA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tol8888(dstPixel, r, g, b, a, x, colMask, VG_lRGBA_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lRGBA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolRGBA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTol8888(dstPixel, r, g, b, a, x, colMask, VG_lRGBA_8888);
}



// conversion to lRGBA8888Pre

/*!
	\brief Convert a single pixel from sRGBA8888 format to lRGBA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolRGBA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lRGBA_8888_PRE);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lRGBA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolRGBA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a,	const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lRGBA_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lRGBA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolRGBA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lRGBA_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lRGBA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolRGBA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a,	const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lRGBA_8888_PRE);
}



// conversion to lL8

/*!
	\brief Convert a single pixel from sRGBA8888 format to lL8 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolL8(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;
	AMuint8 lum;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)a;
	(void)x;
	(void)colMask;

	// use inverse gamma function to convert from non-linear color space to linear color space
	r = AM_GAMMA_INV_TABLE(r);
	g = AM_GAMMA_INV_TABLE(g);
	b = AM_GAMMA_INV_TABLE(b);
	// calculate luminance
	r *= 13933;	// 13933 = 0.2126 * 65536
	g *= 46871;	// 46871 = 0.7152 * 65536
	b *= 4732;	// 4732 = 0.0722 * 65536
	lum = (AMuint8)((r + g + b) >> 16);
	// write pixel
	*dst = lum;
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lL8 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolL8(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;
	AMuint8 lum;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;
	(void)colMask;

	if (a == 0)
		*dst = 0;
	else {
		// remove alpha
		AM_UNPREMULTIPLY(r, g, b, r, g, b, a)
		// use inverse gamma function to convert from non-linear color space to linear color space
		r = AM_GAMMA_INV_TABLE(r);
		g = AM_GAMMA_INV_TABLE(g);
		b = AM_GAMMA_INV_TABLE(b);
		// calculate luminance
		r *= 13933;	// 13933 = 0.2126 * 65536
		g *= 46871;	// 46871 = 0.7152 * 65536
		b *= 4732;	// 4732 = 0.0722 * 65536
		lum = (AMuint8)((r + g + b) >> 16);
		// write pixel
		*dst = lum;
	}
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lL8 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolL8(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;
	AMuint8 lum;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)a;
	(void)x;
	(void)colMask;

	// calculate luminance
	r *= 13933;	// 13933 = 0.2126 * 65536
	g *= 46871;	// 46871 = 0.7152 * 65536
	b *= 4732;	// 4732 = 0.0722 * 65536
	lum = (AMuint8)((r + g + b) >> 16);
	// write pixel
	*dst = lum;
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lL8 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolL8(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;
	AMuint8 lum;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)x;
	(void)colMask;

	if (a == 0)
		*dst = 0;
	else {
		// remove alpha
		AM_UNPREMULTIPLY(r, g, b, r, g, b, a)
		// calculate luminance
		r *= 13933;	// 13933 = 0.2126 * 65536
		g *= 46871;	// 46871 = 0.7152 * 65536
		b *= 4732;	// 4732 = 0.0722 * 65536
		lum = (AMuint8)((r + g + b) >> 16);
		// write pixel
		*dst = lum;
	}
}



// conversion to A8

/*!
	\brief Convert a single pixel from sRGBA8888 format to A8 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888ToA8(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)r;
	(void)g;
	(void)b;
	(void)x;

	if (colMask & VG_ALPHA)
		// write pixel
		*dst = (AMuint8)(a);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to A8 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreToA8(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)r;
	(void)g;
	(void)b;
	(void)x;

	if (colMask & VG_ALPHA)
		// write pixel
		*dst = (AMuint8)(a);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to A8 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888ToA8(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)r;
	(void)g;
	(void)b;
	(void)x;

	if (colMask & VG_ALPHA)
		// write pixel
		*dst = (AMuint8)(a);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to A8 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreToA8(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)r;
	(void)g;
	(void)b;
	(void)x;

	if (colMask & VG_ALPHA)
		// write pixel
		*dst = (AMuint8)(a);
}



// conversion to BW1

/*!
	\brief Convert a single pixel from sRGBA8888 format to BW1 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888ToBW1(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;
	AMuint8 lum, mask, i;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)a;
	(void)colMask;

	// use inverse gamma function to convert from non-linear color space to linear color space
	r = AM_GAMMA_INV_TABLE(r);
	g = AM_GAMMA_INV_TABLE(g);
	b = AM_GAMMA_INV_TABLE(b);
	// calculate luminance
	r *= 13933;	// 13933 = 0.2126 * 65536
	g *= 46871;	// 46871 = 0.7152 * 65536
	b *= 4732;	// 4732 = 0.0722 * 65536
	lum = (AMuint8)((r + g + b) >> 16);
	// write pixel
	i = (AMuint8)(x & 7);
	mask = 1 << i;
	if (lum < 128) {
		mask = 255 - mask;
		*dst &= mask;
	}
	else
		*dst |= mask;
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to BW1 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreToBW1(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;
	AMuint8 lum, mask, i;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)colMask;

	if (a == 0) {
		// write pixel
		i = (AMuint8)(x & 7);
		mask = 1 << i;
		mask = 255 - mask;
		*dst &= mask;
	}
	else {
		// remove alpha
		AM_UNPREMULTIPLY(r, g, b, r, g, b, a)
		// use inverse gamma function to convert from non-linear color space to linear color space
		r = AM_GAMMA_INV_TABLE(r);
		g = AM_GAMMA_INV_TABLE(g);
		b = AM_GAMMA_INV_TABLE(b);
		// calculate luminance
		r *= 13933;	// 13933 = 0.2126 * 65536
		g *= 46871;	// 46871 = 0.7152 * 65536
		b *= 4732;	// 4732 = 0.0722 * 65536
		lum = (AMuint8)((r + g + b) >> 16);
		// write pixel
		i = (AMuint8)(x & 7);
		mask = 1 << i;
		if (lum < 128) {
			mask = 255 - mask;
			*dst &= mask;
		}
		else
			*dst |= mask;
	}
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to BW1 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888ToBW1(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;
	AMuint8 lum, mask, i;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)a;
	(void)colMask;

	// calculate luminance
	r *= 13933;	// 13933 = 0.2126 * 65536
	g *= 46871;	// 46871 = 0.7152 * 65536
	b *= 4732;	// 4732 = 0.0722 * 65536
	lum = (AMuint8)((r + g + b) >> 16);
	// write pixel
	i = (AMuint8)(x & 7);
	mask = 1 << i;
	if (lum < 128) {
		mask = 255 - mask;
		*dst &= mask;
	}
	else
		*dst |= mask;
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to BW1 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreToBW1(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;
	AMuint8 lum, mask, i;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)colMask;

	if (a == 0) {
		// write pixel
		i = (AMuint8)(x & 7);
		mask = 1 << i;
		mask = 255 - mask;
		*dst &= mask;
	}
	else {
		// remove alpha
		AM_UNPREMULTIPLY(r, g, b, r, g, b, a)
		// calculate luminance
		r *= 13933;	// 13933 = 0.2126 * 65536
		g *= 46871;	// 46871 = 0.7152 * 65536
		b *= 4732;	// 4732 = 0.0722 * 65536
		lum = (AMuint8)((r + g + b) >> 16);
		// write pixel
		i = (AMuint8)(x & 7);
		mask = 1 << i;
		if (lum < 128) {
			mask = 255 - mask;
			*dst &= mask;
		}
		else
			*dst |= mask;
	}
}

#if (AM_OPENVG_VERSION >= 110)

// conversion to A1

/*!
	\brief Convert a single pixel from sRGBA8888 format to A1 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888ToA1(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)r;
	(void)g;
	(void)b;

	if (colMask & VG_ALPHA) {
		// write pixel
		AMuint8 mask = 1 << (x & 7);
		if ((a >> 7) != 0)
			*dst |= mask;
		else
			*dst &= mask ^ 0xFF;
	}
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to A1 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreToA1(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)r;
	(void)g;
	(void)b;

	if (colMask & VG_ALPHA) {
		// write pixel
		AMuint8 mask = 1 << (x & 7);
		if ((a >> 7) != 0)
			*dst |= mask;
		else
			*dst &= mask ^ 0xFF;
	}
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to A1 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888ToA1(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)r;
	(void)g;
	(void)b;

	if (colMask & VG_ALPHA) {
		// write pixel
		AMuint8 mask = 1 << (x & 7);
		if ((a >> 7) != 0)
			*dst |= mask;
		else
			*dst &= mask ^ 0xFF;
	}
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to A1 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreToA1(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)r;
	(void)g;
	(void)b;

	if (colMask & VG_ALPHA) {
		// write pixel
		AMuint8 mask = 1 << (x & 7);
		if ((a >> 7) != 0)
			*dst |= mask;
		else
			*dst &= mask ^ 0xFF;
	}
}

/*!
	\brief Convert a single pixel from sRGBA8888 format to A4 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888ToA4(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)r;
	(void)g;
	(void)b;

	if (colMask & VG_ALPHA) {

		AMuint8 a4 = am8To4BitTable[a];
		AMuint8 d = *dst;

		// write pixel
		*dst = (x & 1) ? (d & 0x0F) | (a4 << 4) : (d & 0xF0) | (a4);
	}
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to A4 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreToA4(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)r;
	(void)g;
	(void)b;

	if (colMask & VG_ALPHA) {

		AMuint8 a4 = am8To4BitTable[a];
		AMuint8 d = *dst;

		// write pixel
		*dst = (x & 1) ? (d & 0x0F) | (a4 << 4) : (d & 0xF0) | (a4);
	}
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to A4 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888ToA4(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);

	(void)r;
	(void)g;
	(void)b;

	if (colMask & VG_ALPHA) {

		AMuint8 a4 = am8To4BitTable[a];
		AMuint8 d = *dst;

		// write pixel
		*dst = (x & 1) ? (d & 0x0F) | (a4 << 4) : (d & 0xF0) | (a4);
	}
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to A4 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreToA4(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	AMuint8 *dst = (AMuint8 *)dstPixel;

	AM_ASSERT(dstPixel);
	AM_ASSERT(r <= 0xFF && g <= 0xFF && b <= 0xFF && a <= 0xFF);
	AM_ASSERT(r <= a && g <= a && b <= a);

	(void)r;
	(void)g;
	(void)b;

	if (colMask & VG_ALPHA) {

		AMuint8 a4 = am8To4BitTable[a];
		AMuint8 d = *dst;

		// write pixel
		*dst = (x & 1) ? (d & 0x0F) | (a4 << 4) : (d & 0xF0) | (a4);
	}
}
#endif

// {A,X}RGB channel ordering
// conversion to sXRGB8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to sXRGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosXRGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos888X(dstPixel, r, g, b, a, x, colMask, VG_sXRGB_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sXRGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosXRGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {
	
	amPxlConvert_s8888PreTos888X(dstPixel, r, g, b, a, x, colMask, VG_sXRGB_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sXRGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosXRGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos888X(dstPixel, r, g, b, a, x, colMask, VG_sXRGB_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sXRGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosXRGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos888X(dstPixel, r, g, b, a, x, colMask, VG_sXRGB_8888);
}



// conversion to sARGB8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to sARGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosARGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos8888(dstPixel, r, g, b, a, x, colMask, VG_sARGB_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sARGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosARGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos8888(dstPixel, r, g, b, a, x, colMask, VG_sARGB_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sARGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosARGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos8888(dstPixel, r, g, b, a, x, colMask, VG_sARGB_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sARGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosARGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos8888(dstPixel, r, g, b, a, x, colMask, VG_sARGB_8888);
}



// conversion to sARGB8888Pre

/*!
	\brief Convert a single pixel from sRGBA8888 format to sARGB8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosARGB8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sARGB_8888_PRE);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sARGB8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosARGB8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sARGB_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sARGB8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosARGB8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sARGB_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sARGB8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosARGB8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sARGB_8888_PRE);
}



// conversion to sARGB1555

/*!
	\brief Convert a single pixel from sRGBA8888 format to sARGB1555 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosARGB1555(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos5551(dstPixel, r, g, b, a, x, colMask, VG_sARGB_1555);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sARGB1555 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosARGB1555(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos5551(dstPixel, r, g, b, a, x, colMask, VG_sARGB_1555);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sARGB1555 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosARGB1555(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos5551(dstPixel, r, g, b, a, x, colMask, VG_sARGB_1555);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sARGB1555 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosARGB1555(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos5551(dstPixel, r, g, b, a, x, colMask, VG_sARGB_1555);
}



// conversion to sARGB4444

/*!
	\brief Convert a single pixel from sRGBA8888 format to sARGB4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosARGB4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos4444(dstPixel, r, g, b, a, x, colMask, VG_sARGB_4444);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sARGB4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosARGB4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos4444(dstPixel, r, g, b, a, x, colMask, VG_sARGB_4444);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sARGB4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosARGB4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos4444(dstPixel, r, g, b, a, x, colMask, VG_sARGB_4444);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sARGB4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosARGB4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos4444(dstPixel, r, g, b, a, x, colMask, VG_sARGB_4444);
}



// conversion to lXRGB8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to lXRGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolXRGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tol888X(dstPixel, r, g, b, a, x, colMask, VG_lXRGB_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lXRGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolXRGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTol888X(dstPixel, r, g, b, a, x, colMask, VG_lXRGB_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lXRGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolXRGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tol888X(dstPixel, r, g, b, a, x, colMask, VG_lXRGB_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lXRGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolXRGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTol888X(dstPixel, r, g, b, a, x, colMask, VG_lXRGB_8888);
}



// conversion to lARGB8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to lARGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolARGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tol8888(dstPixel, r, g, b, a, x, colMask, VG_lARGB_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lARGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolARGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTol8888(dstPixel, r, g, b, a, x, colMask, VG_lARGB_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lARGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolARGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tol8888(dstPixel, r, g, b, a, x, colMask, VG_lARGB_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lARGB8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolARGB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTol8888(dstPixel, r, g, b, a, x, colMask, VG_lARGB_8888);
}



// conversion to lARGB8888Pre

/*!
	\brief Convert a single pixel from sRGBA8888 format to lARGB8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolARGB8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lARGB_8888_PRE);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lARGB8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolARGB8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lARGB_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lARGB8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolARGB8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lARGB_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lARGB8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolARGB8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lARGB_8888_PRE);
}



// BGR{A,X} channel ordering

// conversion to sBGRX8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to sBGRX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosBGRX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos888X(dstPixel, r, g, b, a, x, colMask, VG_sBGRX_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sBGRX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosBGRX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos888X(dstPixel, r, g, b, a, x, colMask, VG_sBGRX_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sBGRX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosBGRX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos888X(dstPixel, r, g, b, a, x, colMask, VG_sBGRX_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sBGRX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosBGRX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos888X(dstPixel, r, g, b, a, x, colMask, VG_sBGRX_8888);
}



// conversion to sBGRA8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to sBGRA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosBGRA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos8888(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sBGRA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosBGRA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos8888(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sBGRA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosBGRA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos8888(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sBGRA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosBGRA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos8888(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_8888);
}



// conversion to sBGRA8888Pre

/*!
	\brief Convert a single pixel from sRGBA8888 format to sBGRA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosBGRA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_8888_PRE);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sBGRA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosBGRA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sBGRA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosBGRA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sBGRA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosBGRA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_8888_PRE);
}



// conversion to sBGR565

/*!
	\brief Convert a single pixel from sRGBA8888 format to sBGR565 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosBGR565(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos565(dstPixel, r, g, b, a, x, colMask, VG_sBGR_565);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sBGR565 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosBGR565(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos565(dstPixel, r, g, b, a, x, colMask, VG_sBGR_565);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sBGR565 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosBGR565(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos565(dstPixel, r, g, b, a, x, colMask, VG_sBGR_565);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sBGR565 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosBGR565(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos565(dstPixel, r, g, b, a, x, colMask, VG_sBGR_565);
}



// conversion to sBGRA5551

/*!
	\brief Convert a single pixel from sRGBA8888 format to sBGRA5551 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosBGRA5551(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos5551(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_5551);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sBGRA5551 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosBGRA5551(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos5551(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_5551);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sBGRA5551 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosBGRA5551(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos5551(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_5551);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sBGRA5551 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosBGRA5551(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos5551(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_5551);
}



// conversion to sBGRA4444

/*!
	\brief Convert a single pixel from sRGBA8888 format to sBGRA4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosBGRA4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos4444(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_4444);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sBGRA4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosBGRA4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos4444(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_4444);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sBGRA4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosBGRA4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos4444(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_4444);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sBGRA4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosBGRA4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos4444(dstPixel, r, g, b, a, x, colMask, VG_sBGRA_4444);
}



// conversion to lBGRX8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to lBGRX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolBGRX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tol888X(dstPixel, r, g, b, a, x, colMask, VG_lBGRX_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lBGRX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolBGRX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTol888X(dstPixel, r, g, b, a, x, colMask, VG_lBGRX_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lBGRX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolBGRX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {
	
	amPxlConvert_l8888Tol888X(dstPixel, r, g, b, a, x, colMask, VG_lBGRX_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lBGRX8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolBGRX8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTol888X(dstPixel, r, g, b, a, x, colMask, VG_lBGRX_8888);
}



// conversion to lBGRA8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to lBGRA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolBGRA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tol8888(dstPixel, r, g, b, a, x, colMask, VG_lBGRA_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lBGRA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolBGRA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTol8888(dstPixel, r, g, b, a, x, colMask, VG_lBGRA_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lBGRA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolBGRA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tol8888(dstPixel, r, g, b, a, x, colMask, VG_lBGRA_8888);
}


/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lBGRA8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolBGRA8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTol8888(dstPixel, r, g, b, a, x, colMask, VG_lBGRA_8888);
}



// conversion to lBGRA8888Pre

/*!
	\brief Convert a single pixel from sRGBA8888 format to lBGRA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolBGRA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lBGRA_8888_PRE);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lBGRA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolBGRA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lBGRA_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lBGRA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolBGRA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lBGRA_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lBGRA8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolBGRA8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lBGRA_8888_PRE);
}



// {A,X}BGR channel ordering

// conversion to sXBGR8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to sXBGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosXBGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos888X(dstPixel, r, g, b, a, x, colMask, VG_sXBGR_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sXBGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosXBGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos888X(dstPixel, r, g, b, a, x, colMask, VG_sXBGR_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sXBGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosXBGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos888X(dstPixel, r, g, b, a, x, colMask, VG_sXBGR_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sXBGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosBGRB8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos888X(dstPixel, r, g, b, a, x, colMask, VG_sXBGR_8888);
}



// conversion to sABGR8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to sABGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosABGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos8888(dstPixel, r, g, b, a, x, colMask, VG_sABGR_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sABGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosABGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos8888(dstPixel, r, g, b, a, x, colMask, VG_sABGR_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sABGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosABGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos8888(dstPixel, r, g, b, a, x, colMask, VG_sABGR_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sABGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosABGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos8888(dstPixel, r, g, b, a, x, colMask, VG_sABGR_8888);
}



// conversion to sABGR8888Pre

/*!
	\brief Convert a single pixel from sRGBA8888 format to sABGR8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosABGR8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sABGR_8888_PRE);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sABGR8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosABGR8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sABGR_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sABGR8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosABGR8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sABGR_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sABGR8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosABGR8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos8888Pre(dstPixel, r, g, b, a, x, colMask, VG_sABGR_8888_PRE);
}



// conversion to sABGR1555

/*!
	\brief Convert a single pixel from sRGBA8888 format to sABGR1555 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosABGR1555(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos5551(dstPixel, r, g, b, a, x, colMask, VG_sABGR_1555);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sABGR1555 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosABGR1555(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos5551(dstPixel, r, g, b, a, x, colMask, VG_sABGR_1555);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sABGR1555 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosABGR1555(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos5551(dstPixel, r, g, b, a, x, colMask, VG_sABGR_1555);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sABGR1555 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosABGR1555(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos5551(dstPixel, r, g, b, a, x, colMask, VG_sABGR_1555);
}



// conversion to sABGR4444

/*!
	\brief Convert a single pixel from sRGBA8888 format to sABGR4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TosABGR4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tos4444(dstPixel, r, g, b, a, x, colMask, VG_sABGR_4444);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to sABGR4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTosABGR4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTos4444(dstPixel, r, g, b, a, x, colMask, VG_sABGR_4444);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to sABGR4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TosABGR4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tos4444(dstPixel, r, g, b, a, x, colMask, VG_sABGR_4444);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to sABGR4444 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTosABGR4444(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTos4444(dstPixel, r, g, b, a, x, colMask, VG_sABGR_4444);
}



// conversion to lXBGR8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to lXBGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolXBGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tol888X(dstPixel, r, g, b, a, x, colMask, VG_lXBGR_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lXBGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolXBGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTol888X(dstPixel, r, g, b, a, x, colMask, VG_lXBGR_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lXBGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolXBGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tol888X(dstPixel, r, g, b, a, x, colMask, VG_lXBGR_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lXBGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolXBGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTol888X(dstPixel, r, g, b, a, x, colMask, VG_lXBGR_8888);
}



// conversion to lABGR8888

/*!
	\brief Convert a single pixel from sRGBA8888 format to lABGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolABGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tol8888(dstPixel, r, g, b, a, x, colMask, VG_lABGR_8888);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lABGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolABGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTol8888(dstPixel, r, g, b, a, x, colMask, VG_lABGR_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lABGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolABGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tol8888(dstPixel, r, g, b, a, x, colMask, VG_lABGR_8888);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lABGR8888 format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolABGR8888(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTol8888(dstPixel, r, g, b, a, x, colMask, VG_lABGR_8888);
}



// conversion to lABGR8888Pre

/*!
	\brief Convert a single pixel from sRGBA8888 format to lABGR8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_sRGBA8888TolABGR8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888Tol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lABGR_8888_PRE);
}

/*!
	\brief Convert a single pixel from sRGBA8888Pre format to lABGR8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_sRGBA8888PreTolABGR8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_s8888PreTol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lABGR_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888 format to lABGR8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b, a <= 0xFF.
*/
void amPxlConvert_lRGBA8888TolABGR8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888Tol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lABGR_8888_PRE);
}

/*!
	\brief Convert a single pixel from lRGBA8888Pre format to lABGR8888Pre format.
	\param dstPixel pointer to the output pixel.
	\param r red component of the source pixel.
	\param g green component of the source pixel.
	\param b blue component of the source pixel.
	\param a alpha component of the source pixel.
	\param x x-coordinate used to dither.
	\param colMask bitfield color mask.
	\pre 0 <= r, g, b <= a <= 0xFF.
*/
void amPxlConvert_lRGBA8888PreTolABGR8888Pre(void *dstPixel, AMuint32 r, AMuint32 g, AMuint32 b, AMuint32 a, const AMuint32 x, const VGbitfield colMask) {

	amPxlConvert_l8888PreTol8888Pre(dstPixel, r, g, b, a, x, colMask, VG_lABGR_8888_PRE);
}

/*!
	\brief Initialize pixel conversion table.
	\param _context pointer to a AMContext structure, containing pixel conversion table.
*/
void amPxlConversionTableInit(void *_context) {

	AMContext *context = (AMContext *)_context;
	AMuint32 i;

	AM_ASSERT(context);

	// row = destination image format, col = source image format
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBX_8888)		][0] = amPxlConvert_sRGBA8888TosRGBX8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBX_8888)		][1] = amPxlConvert_sRGBA8888PreTosRGBX8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBX_8888)		][2] = amPxlConvert_lRGBA8888TosRGBX8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBX_8888)		][3] = amPxlConvert_lRGBA8888PreTosRGBX8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_8888)		][0] = amPxlConvert_sRGBA8888TosRGBA8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_8888)		][1] = amPxlConvert_sRGBA8888PreTosRGBA8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_8888)		][2] = amPxlConvert_lRGBA8888TosRGBA8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_8888)		][3] = amPxlConvert_lRGBA8888PreTosRGBA8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_8888_PRE)	][0] = amPxlConvert_sRGBA8888TosRGBA8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_8888_PRE)	][1] = amPxlConvert_sRGBA8888PreTosRGBA8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_8888_PRE)	][2] = amPxlConvert_lRGBA8888TosRGBA8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_8888_PRE)	][3] = amPxlConvert_lRGBA8888PreTosRGBA8888Pre;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGB_565)		][0] = amPxlConvert_sRGBA8888TosRGB565;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGB_565)		][1] = amPxlConvert_sRGBA8888PreTosRGB565;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGB_565)		][2] = amPxlConvert_lRGBA8888TosRGB565;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGB_565)		][3] = amPxlConvert_lRGBA8888PreTosRGB565;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_5551)		][0] = amPxlConvert_sRGBA8888TosRGBA5551;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_5551)		][1] = amPxlConvert_sRGBA8888PreTosRGBA5551;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_5551)		][2] = amPxlConvert_lRGBA8888TosRGBA5551;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_5551)		][3] = amPxlConvert_lRGBA8888PreTosRGBA5551;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_4444)		][0] = amPxlConvert_sRGBA8888TosRGBA4444;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_4444)		][1] = amPxlConvert_sRGBA8888PreTosRGBA4444;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_4444)		][2] = amPxlConvert_lRGBA8888TosRGBA4444;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sRGBA_4444)		][3] = amPxlConvert_lRGBA8888PreTosRGBA4444;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sL_8)			][0] = amPxlConvert_sRGBA8888TosL8;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sL_8)			][1] = amPxlConvert_sRGBA8888PreTosL8;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sL_8)			][2] = amPxlConvert_lRGBA8888TosL8;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sL_8)			][3] = amPxlConvert_lRGBA8888PreTosL8;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lRGBX_8888)		][0] = amPxlConvert_sRGBA8888TolRGBX8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lRGBX_8888)		][1] = amPxlConvert_sRGBA8888PreTolRGBX8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lRGBX_8888)		][2] = amPxlConvert_lRGBA8888TolRGBX8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lRGBX_8888)		][3] = amPxlConvert_lRGBA8888PreTolRGBX8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lRGBA_8888)		][0] = amPxlConvert_sRGBA8888TolRGBA8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lRGBA_8888)		][1] = amPxlConvert_sRGBA8888PreTolRGBA8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lRGBA_8888)		][2] = amPxlConvert_lRGBA8888TolRGBA8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lRGBA_8888)		][3] = amPxlConvert_lRGBA8888PreTolRGBA8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lRGBA_8888_PRE)	][0] = amPxlConvert_sRGBA8888TolRGBA8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lRGBA_8888_PRE)	][1] = amPxlConvert_sRGBA8888PreTolRGBA8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lRGBA_8888_PRE)	][2] = amPxlConvert_lRGBA8888TolRGBA8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lRGBA_8888_PRE)	][3] = amPxlConvert_lRGBA8888PreTolRGBA8888Pre;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lL_8)			][0] = amPxlConvert_sRGBA8888TolL8;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lL_8)			][1] = amPxlConvert_sRGBA8888PreTolL8;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lL_8)			][2] = amPxlConvert_lRGBA8888TolL8;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lL_8)			][3] = amPxlConvert_lRGBA8888PreTolL8;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_A_8)				][0] = amPxlConvert_sRGBA8888ToA8;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_A_8)				][1] = amPxlConvert_sRGBA8888PreToA8;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_A_8)				][2] = amPxlConvert_lRGBA8888ToA8;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_A_8)				][3] = amPxlConvert_lRGBA8888PreToA8;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_BW_1)			][0] = amPxlConvert_sRGBA8888ToBW1;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_BW_1)			][1] = amPxlConvert_sRGBA8888PreToBW1;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_BW_1)			][2] = amPxlConvert_lRGBA8888ToBW1;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_BW_1)			][3] = amPxlConvert_lRGBA8888PreToBW1;

#if (AM_OPENVG_VERSION >= 110)
	context->pxlConverters[AM_FMT_GET_INDEX(VG_A_1)				][0] = amPxlConvert_sRGBA8888ToA1;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_A_1)				][1] = amPxlConvert_sRGBA8888PreToA1;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_A_1)				][2] = amPxlConvert_lRGBA8888ToA1;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_A_1)				][3] = amPxlConvert_lRGBA8888PreToA1;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_A_4)				][0] = amPxlConvert_sRGBA8888ToA4;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_A_4)				][1] = amPxlConvert_sRGBA8888PreToA4;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_A_4)				][2] = amPxlConvert_lRGBA8888ToA4;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_A_4)				][3] = amPxlConvert_lRGBA8888PreToA4;
#endif

	// fill table with NULL pointers
	for (i = AM_LAST_IMAGE_FORMAT + 1; i < AM_IMAGE_FORMATS_COUNT; ++i) {
		context->pxlConverters[i][0] = NULL;
		context->pxlConverters[i][1] = NULL;
		context->pxlConverters[i][2] = NULL;
		context->pxlConverters[i][3] = NULL;
	}
	// {A,X}RGB channel ordering
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sXRGB_8888)		][0] = amPxlConvert_sRGBA8888TosXRGB8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sXRGB_8888)		][1] = amPxlConvert_sRGBA8888PreTosXRGB8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sXRGB_8888)		][2] = amPxlConvert_lRGBA8888TosXRGB8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sXRGB_8888)		][3] = amPxlConvert_lRGBA8888PreTosXRGB8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_8888)		][0] = amPxlConvert_sRGBA8888TosARGB8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_8888)		][1] = amPxlConvert_sRGBA8888PreTosARGB8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_8888)		][2] = amPxlConvert_lRGBA8888TosARGB8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_8888)		][3] = amPxlConvert_lRGBA8888PreTosARGB8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_8888_PRE)	][0] = amPxlConvert_sRGBA8888TosARGB8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_8888_PRE)	][1] = amPxlConvert_sRGBA8888PreTosARGB8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_8888_PRE)	][2] = amPxlConvert_lRGBA8888TosARGB8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_8888_PRE)	][3] = amPxlConvert_lRGBA8888PreTosARGB8888Pre;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_1555)		][0] = amPxlConvert_sRGBA8888TosARGB1555;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_1555)		][1] = amPxlConvert_sRGBA8888PreTosARGB1555;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_1555)		][2] = amPxlConvert_lRGBA8888TosARGB1555;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_1555)		][3] = amPxlConvert_lRGBA8888PreTosARGB1555;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_4444)		][0] = amPxlConvert_sRGBA8888TosARGB4444;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_4444)		][1] = amPxlConvert_sRGBA8888PreTosARGB4444;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_4444)		][2] = amPxlConvert_lRGBA8888TosARGB4444;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sARGB_4444)		][3] = amPxlConvert_lRGBA8888PreTosARGB4444;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lXRGB_8888)		][0] = amPxlConvert_sRGBA8888TolXRGB8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lXRGB_8888)		][1] = amPxlConvert_sRGBA8888PreTolXRGB8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lXRGB_8888)		][2] = amPxlConvert_lRGBA8888TolXRGB8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lXRGB_8888)		][3] = amPxlConvert_lRGBA8888PreTolXRGB8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lARGB_8888)		][0] = amPxlConvert_sRGBA8888TolARGB8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lARGB_8888)		][1] = amPxlConvert_sRGBA8888PreTolARGB8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lARGB_8888)		][2] = amPxlConvert_lRGBA8888TolARGB8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lARGB_8888)		][3] = amPxlConvert_lRGBA8888PreTolARGB8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lARGB_8888_PRE)	][0] = amPxlConvert_sRGBA8888TolARGB8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lARGB_8888_PRE)	][1] = amPxlConvert_sRGBA8888PreTolARGB8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lARGB_8888_PRE)	][2] = amPxlConvert_lRGBA8888TolARGB8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lARGB_8888_PRE)	][3] = amPxlConvert_lRGBA8888PreTolARGB8888Pre;

	// BGR{A,X} channel ordering
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRX_8888)		][0] = amPxlConvert_sRGBA8888TosBGRX8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRX_8888)		][1] = amPxlConvert_sRGBA8888PreTosBGRX8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRX_8888)		][2] = amPxlConvert_lRGBA8888TosBGRX8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRX_8888)		][3] = amPxlConvert_lRGBA8888PreTosBGRX8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_8888)		][0] = amPxlConvert_sRGBA8888TosBGRA8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_8888)		][1] = amPxlConvert_sRGBA8888PreTosBGRA8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_8888)		][2] = amPxlConvert_lRGBA8888TosBGRA8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_8888)		][3] = amPxlConvert_lRGBA8888PreTosBGRA8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_8888_PRE)	][0] = amPxlConvert_sRGBA8888TosBGRA8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_8888_PRE)	][1] = amPxlConvert_sRGBA8888PreTosBGRA8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_8888_PRE)	][2] = amPxlConvert_lRGBA8888TosBGRA8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_8888_PRE)	][3] = amPxlConvert_lRGBA8888PreTosBGRA8888Pre;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGR_565)		][0] = amPxlConvert_sRGBA8888TosBGR565;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGR_565)		][1] = amPxlConvert_sRGBA8888PreTosBGR565;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGR_565)		][2] = amPxlConvert_lRGBA8888TosBGR565;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGR_565)		][3] = amPxlConvert_lRGBA8888PreTosBGR565;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_5551)		][0] = amPxlConvert_sRGBA8888TosBGRA5551;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_5551)		][1] = amPxlConvert_sRGBA8888PreTosBGRA5551;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_5551)		][2] = amPxlConvert_lRGBA8888TosBGRA5551;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_5551)		][3] = amPxlConvert_lRGBA8888PreTosBGRA5551;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_4444)		][0] = amPxlConvert_sRGBA8888TosBGRA4444;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_4444)		][1] = amPxlConvert_sRGBA8888PreTosBGRA4444;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_4444)		][2] = amPxlConvert_lRGBA8888TosBGRA4444;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sBGRA_4444)		][3] = amPxlConvert_lRGBA8888PreTosBGRA4444;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lBGRX_8888)		][0] = amPxlConvert_sRGBA8888TolBGRX8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lBGRX_8888)		][1] = amPxlConvert_sRGBA8888PreTolBGRX8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lBGRX_8888)		][2] = amPxlConvert_lRGBA8888TolBGRX8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lBGRX_8888)		][3] = amPxlConvert_lRGBA8888PreTolBGRX8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lBGRA_8888)		][0] = amPxlConvert_sRGBA8888TolBGRA8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lBGRA_8888)		][1] = amPxlConvert_sRGBA8888PreTolBGRA8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lBGRA_8888)		][2] = amPxlConvert_lRGBA8888TolBGRA8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lBGRA_8888)		][3] = amPxlConvert_lRGBA8888PreTolBGRA8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lBGRA_8888_PRE)	][0] = amPxlConvert_sRGBA8888TolBGRA8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lBGRA_8888_PRE)	][1] = amPxlConvert_sRGBA8888PreTolBGRA8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lBGRA_8888_PRE)	][2] = amPxlConvert_lRGBA8888TolBGRA8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lBGRA_8888_PRE)	][3] = amPxlConvert_lRGBA8888PreTolBGRA8888Pre;

	// {A,X}BGR channel ordering
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sXBGR_8888)		][0] = amPxlConvert_sRGBA8888TosXBGR8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sXBGR_8888)		][1] = amPxlConvert_sRGBA8888PreTosXBGR8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sXBGR_8888)		][2] = amPxlConvert_lRGBA8888TosXBGR8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sXBGR_8888)		][3] = amPxlConvert_lRGBA8888PreTosBGRB8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_8888)		][0] = amPxlConvert_sRGBA8888TosABGR8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_8888)		][1] = amPxlConvert_sRGBA8888PreTosABGR8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_8888)		][2] = amPxlConvert_lRGBA8888TosABGR8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_8888)		][3] = amPxlConvert_lRGBA8888PreTosABGR8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_8888_PRE)	][0] = amPxlConvert_sRGBA8888TosABGR8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_8888_PRE)	][1] = amPxlConvert_sRGBA8888PreTosABGR8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_8888_PRE)	][2] = amPxlConvert_lRGBA8888TosABGR8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_8888_PRE)	][3] = amPxlConvert_lRGBA8888PreTosABGR8888Pre;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_1555)		][0] = amPxlConvert_sRGBA8888TosABGR1555;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_1555)		][1] = amPxlConvert_sRGBA8888PreTosABGR1555;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_1555)		][2] = amPxlConvert_lRGBA8888TosABGR1555;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_1555)		][3] = amPxlConvert_lRGBA8888PreTosABGR1555;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_4444)		][0] = amPxlConvert_sRGBA8888TosABGR4444;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_4444)		][1] = amPxlConvert_sRGBA8888PreTosABGR4444;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_4444)		][2] = amPxlConvert_lRGBA8888TosABGR4444;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_sABGR_4444)		][3] = amPxlConvert_lRGBA8888PreTosABGR4444;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lXBGR_8888)		][0] = amPxlConvert_sRGBA8888TolXBGR8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lXBGR_8888)		][1] = amPxlConvert_sRGBA8888PreTolXBGR8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lXBGR_8888)		][2] = amPxlConvert_lRGBA8888TolXBGR8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lXBGR_8888)		][3] = amPxlConvert_lRGBA8888PreTolXBGR8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lABGR_8888)		][0] = amPxlConvert_sRGBA8888TolABGR8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lABGR_8888)		][1] = amPxlConvert_sRGBA8888PreTolABGR8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lABGR_8888)		][2] = amPxlConvert_lRGBA8888TolABGR8888;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lABGR_8888)		][3] = amPxlConvert_lRGBA8888PreTolABGR8888;

	context->pxlConverters[AM_FMT_GET_INDEX(VG_lABGR_8888_PRE)	][0] = amPxlConvert_sRGBA8888TolABGR8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lABGR_8888_PRE)	][1] = amPxlConvert_sRGBA8888PreTolABGR8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lABGR_8888_PRE)	][2] = amPxlConvert_lRGBA8888TolABGR8888Pre;
	context->pxlConverters[AM_FMT_GET_INDEX(VG_lABGR_8888_PRE)	][3] = amPxlConvert_lRGBA8888PreTolABGR8888Pre;
}

// *********************************************************************
//                        Private implementations
// *********************************************************************

/*!
	\brief This function computes a linear combination, according to the given matrix, of color and alpha
	values from the source image at each pixel.
	\param dst destination image.
	\param src source image.
	\param matrix input color matrix.
	\param filterLinear if AM_TRUE, apply the filter in the linear color space.
	\param filterPreMultiplied if AM_TRUE, apply the filter in the premultiplied color space.
	\param filterChannels specifies which destination channels are to be written.
	\param context input context containing the pixel converters table.
*/
void amFltColorMatrix(AMImage *dst,
					  const AMImage *src,
					  const AMfloat *matrix,
					  const AMbool filterLinear,
					  const AMbool filterPreMultiplied,
					  const VGbitfield filterChannels,
					  const AMContext *context) {
	
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dst->format);
	AMuint32 dstBits = pxlFormatTable[dstIdx][FMT_BITS];
	AMuint32 rightShift;
	AMint32 m[4][5], width, height, i, j;
	VGImageFormat filterSpaceFormat;
	AMPixel32ConverterFunction converter;
	AMImageSampler sampler;
	AMImageSamplerParams samplerParams;
	AMuint32 dstBytesPerPixel;
	AMuint8 *dst8;
	AMfloat tmpf;

	// we accept matrix entries between -511 and 511 (typical effects use entries in the [-1; 1] range)
	#define LOW_BOUND -511.0f
	#define HIGH_BOUND 511.0f
	// we represent each fixed point matrix entry with 11 bit precision
	#define FIXED_PRECISION_F 2048.0f
	#define FIXED_PRECISION_BITS 11

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(matrix);
	AM_ASSERT(context);

	tmpf = AM_CLAMP(matrix[0], LOW_BOUND, HIGH_BOUND);
	m[0][0] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[1], LOW_BOUND, HIGH_BOUND);
	m[1][0] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[2], LOW_BOUND, HIGH_BOUND);
	m[2][0] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[3], LOW_BOUND, HIGH_BOUND);
	m[3][0] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[4], LOW_BOUND, HIGH_BOUND);
	m[0][1] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[5], LOW_BOUND, HIGH_BOUND);
	m[1][1] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[6], LOW_BOUND, HIGH_BOUND);
	m[2][1] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[7], LOW_BOUND, HIGH_BOUND);
	m[3][1] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[8], LOW_BOUND, HIGH_BOUND);
	m[0][2] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[9], LOW_BOUND, HIGH_BOUND);
	m[1][2] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[10], LOW_BOUND, HIGH_BOUND);
	m[2][2] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[11], LOW_BOUND, HIGH_BOUND);
	m[3][2] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[12], LOW_BOUND, HIGH_BOUND);
	m[0][3] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[13], LOW_BOUND, HIGH_BOUND);
	m[1][3] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[14], LOW_BOUND, HIGH_BOUND);
	m[2][3] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[15], LOW_BOUND, HIGH_BOUND);
	m[3][3] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[16], LOW_BOUND, HIGH_BOUND);
	m[0][4] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[17], LOW_BOUND, HIGH_BOUND);
	m[1][4] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[18], LOW_BOUND, HIGH_BOUND);
	m[2][4] = (AMint32)(FIXED_PRECISION_F * tmpf);
	tmpf = AM_CLAMP(matrix[19], LOW_BOUND, HIGH_BOUND);
	m[3][4] = (AMint32)(FIXED_PRECISION_F * tmpf);

	// find valid width and height
	width = AM_MIN(src->width, dst->width);
	height = AM_MIN(src->height, dst->height);

	if (filterLinear) {
		if (filterPreMultiplied) {
			filterSpaceFormat = VG_lRGBA_8888_PRE;
			converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][3];
		}
		else {
			filterSpaceFormat = VG_lRGBA_8888;
			converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][2];
		}
	}
	else {
		if (filterPreMultiplied) {
			filterSpaceFormat = VG_sRGBA_8888_PRE;
			converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][1];
		}
		else {
			filterSpaceFormat = VG_sRGBA_8888;
			converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][0];
		}
	}
	AM_ASSERT(converter);
	// extract pixel sampler
	sampler = amImageSamplerGet(src->format);
	AM_ASSERT(sampler);
	samplerParams.image = src;
	samplerParams.srcIdx = AM_FMT_GET_INDEX(src->format);
	samplerParams.tilingMode = VG_TILE_PAD;
	samplerParams.dstIdx = AM_FMT_GET_INDEX(filterSpaceFormat);
	samplerParams.bilinear = AM_FALSE;
#if (AM_OPENVG_VERSION >= 110)
	samplerParams.colorTransformation = NULL;
#endif

	switch (dstBits) {
		case 1:
			rightShift = 3;
			break;
		case 4:
			rightShift = 1;
			break;
		default:
			rightShift = 0;
			break;
	}

	// apply filter and write onto destination
	dst8 = (AMuint8 *)dst->pixels;
	dstBytesPerPixel = amImageBytesPerPixel(dst->format);

	for (i = 0; i < height; ++i) {

		samplerParams.y = i << 16;

		for (j = 0; j < width; ++j) {

			AMint32 rgbClamp, rTmp, gTmp, bTmp, aTmp, r, g, b, a;
			AMuint32 rgba32;

			samplerParams.x = j << 16;
			rgba32 = sampler(&samplerParams);
			rTmp = (AMint32)(rgba32 >> 24);
			gTmp = (AMint32)((rgba32 >> 16) & 0xFF);
			bTmp = (AMint32)((rgba32 >> 8) & 0xFF);
			aTmp = (AMint32)(rgba32 & 0xFF);
			// apply color matrix
			r = (m[0][0] * rTmp + m[0][1] * gTmp + m[0][2] * bTmp + m[0][3] * aTmp + m[0][4]) >> FIXED_PRECISION_BITS;
			g = (m[1][0] * rTmp + m[1][1] * gTmp + m[1][2] * bTmp + m[1][3] * aTmp + m[1][4]) >> FIXED_PRECISION_BITS;
			b = (m[2][0] * rTmp + m[2][1] * gTmp + m[2][2] * bTmp + m[2][3] * aTmp + m[2][4]) >> FIXED_PRECISION_BITS;
			a = (m[3][0] * rTmp + m[3][1] * gTmp + m[3][2] * bTmp + m[3][3] * aTmp + m[3][4]) >> FIXED_PRECISION_BITS;
			a = AM_CLAMP(a, 0, 255);
			// take care of premultiplied formats
			rgbClamp = filterPreMultiplied ? a : 0xFF;
			r = AM_CLAMP(r, 0, rgbClamp);
			g = AM_CLAMP(g, 0, rgbClamp);
			b = AM_CLAMP(b, 0, rgbClamp);

			if (dstBits > 4)
				converter((void *)&dst8[j * dstBytesPerPixel], r, g, b, a, 0, filterChannels);
			else
				converter((void *)&dst8[j >> rightShift], r, g, b, a, j, filterChannels);
		}
		dst8 += dst->dataStride;
	}

#if defined(AM_GLE) || defined(AM_GLS)
	// invalidate image textures
	amImageTexturesInvalidate(dst, context);
#endif

	#undef LOW_BOUND
	#undef HIGH_BOUND
	#undef FIXED_PRECISION_F
	#undef FIXED_PRECISION_BITS
}

// auxiliary structure exclusively used to pass parameters to amGetImage128Pixel function
typedef struct _AMImage128Sample {
	AMint32 *pixels;
	AMint32 width;
	AMint32 height;
	AMbool pow2;
	AMuint32 widthShift;
	AMuint32 heightShift;
	VGTilingMode tileMode;
	AMint32 tileColorR;
	AMint32 tileColorG;
	AMint32 tileColorB;
	AMint32 tileColorA;
} AMImage128Sample;

// used by separable convolve filter only
void amGetImage128Pixel(AMint32 *rTmp,
						AMint32 *gTmp,
						AMint32 *bTmp,
						AMint32 *aTmp,
						const AMImage128Sample *image,
						AMint32 x,
						AMint32 y) {

	AMint32 cycles;
	const AMint32 *pixels;

	AM_ASSERT(rTmp);
	AM_ASSERT(gTmp);
	AM_ASSERT(bTmp);
	AM_ASSERT(aTmp);
	AM_ASSERT(image);
	AM_ASSERT(image->width > 0 && image->height > 0);

	//  0 <= x < width and 0 <= y < height
	if ((((AMuint32)(x)) < (AMuint32)image->width) && (((AMuint32)(y)) < (AMuint32)image->height)) {
		pixels = image->pixels + 4 * (y * image->width + x);
		*rTmp = *pixels++;
		*gTmp = *pixels++;
		*bTmp = *pixels++;
		*aTmp = *pixels;
	}
	else {
		switch (image->tileMode) {

			case VG_TILE_FILL:
				*rTmp = image->tileColorR;
				*gTmp = image->tileColorG;
				*bTmp = image->tileColorB;
				*aTmp = image->tileColorA;
				break;

			case VG_TILE_PAD:
				x = AM_CLAMP(x, 0, image->width - 1);
				y = AM_CLAMP(y, 0, image->height - 1);
				pixels = image->pixels + 4 * (y * image->width + x);
				*rTmp = *pixels++;
				*gTmp = *pixels++;
				*bTmp = *pixels++;
				*aTmp = *pixels;
				break;

			case VG_TILE_REPEAT:
				if (image->pow2) {
					x &= image->width - 1;
					y &= image->height - 1;
					pixels = image->pixels + 4 * ((y << image->widthShift) + x);
				}
				else {
					// x < 0 or x >= width
					if (((AMuint32)(x)) >= (AMuint32)image->width) {
						x = x % image->width;
						if (x < 0) x += image->width;
					}
					// y < 0 or y >= height
					if (((AMuint32)(y)) >= (AMuint32)image->height) {
						y = y % image->height;
						if (y < 0) y += image->height;
					}
					pixels = image->pixels + 4 * (y * image->width + x);
				}
				*rTmp = *pixels++;
				*gTmp = *pixels++;
				*bTmp = *pixels++;
				*aTmp = *pixels;
				break;

			case VG_TILE_REFLECT:
				if (image->pow2) {
					if (((AMuint32)(x)) >= (AMuint32)image->width) {
						cycles = (x < 0) ? (((-x - 1) >> image->widthShift) + 1) : (x >> image->widthShift);
						x &= image->width - 1;
						if (cycles & 1) x = image->width - 1 - x;
					}
					if (((AMuint32)(y)) >= (AMuint32)image->height) {
						cycles = (y < 0) ? (((-y - 1) >> image->heightShift) + 1) : (y >> image->heightShift);
						y &= image->height - 1;
						if (cycles & 1) y = image->height - 1 - y;
					}
					pixels = image->pixels + 4 * ((y << image->widthShift) + x);
				}
				else {
					// x < 0 or x >= width
					if (((AMuint32)(x)) >= (AMuint32)image->width) {
						cycles = (x < 0) ? (((-x - 1) / image->width) + 1) : (x / image->width);
						x = x % image->width;
						if (cycles & 1) {
							x = image->width - 1 - x;
							if (x >= image->width) x -= image->width;
						}
						else {
							if (x < 0) x += image->width;
						}
					}
					// y < 0 or y >= height
					if (((AMuint32)(y)) >= (AMuint32)image->height) {
						cycles = (y < 0) ? (((-y - 1) / image->height) + 1) : (y / image->height);
						y = y % image->height;
						if (cycles & 1) {
							y = image->height - 1 - y;
							if (y >= image->height) y -= image->height;
						}
						else {
							if (y < 0) y += image->height;
						}
					}
					pixels = image->pixels + 4 * (y * image->width + x);
				}
				*rTmp = *pixels++;
				*gTmp = *pixels++;
				*bTmp = *pixels++;
				*aTmp = *pixels;
				break;
			default:
				AM_ASSERT(0 == 1);
				break;
		}
	}
}

/*!
	\brief This function applies a user-supplied convolution kernel to a source image.
	\param dst destination image.
	\param src source image.
	\param kernelWidth width of the convolution kernel.
	\param kernelHeight height of the convolution kernel.
	\param shiftX horizontal translation between source and destination images, in pixels.
	\param shiftY vertical translation between source and destination images, in pixels.
	\param kernel convolution kernel.
	\param scale scale factor to apply to the result of the convolution, at each pixel.
	\param bias value to add to the result of the convolution, at each pixel.
	\param tilingMode tiling mode to use when reading pixels from source image.
	\param sTileColor color to use for VG_TILE_FILL tiling mode, in non-linear unpremultiplied color space.
	\param filterLinear if AM_TRUE, apply the filter in the linear color space.
	\param filterPreMultiplied if AM_TRUE, apply the filter in the premultiplied color space.
	\param filterChannels specifies which destination channels are to be written.
	\param context input context containing the pixel converters table.
*/
void amFltConvolve(AMImage *dst,
				   const AMImage *src,
				   const AMint32 kernelWidth,
				   const AMint32 kernelHeight,
				   const AMint32 shiftX,
				   const AMint32 shiftY,
				   const AMint16 *kernel,
				   const AMfloat scale,
				   const AMfloat bias,
				   const VGTilingMode tilingMode,
				   const AMfloat *sTileColor,
				   const AMbool filterLinear,
				   const AMbool filterPreMultiplied,
				   const VGbitfield filterChannels,
				   const AMContext *context) {

	#define KERNEL(_i, _j) (kernel[(_i) * kernelHeight + (_j)])
	#define FIXED_PRECISION_BITS 24
	#define FIXED_PRECISION_F (65536.0f * 256.0f)

	AMuint32 dstIdx = AM_FMT_GET_INDEX(dst->format);
	AMuint32 dstBits = pxlFormatTable[dstIdx][FMT_BITS];
	AMuint32 rightShift;
	AMint32 width, height, i, j, k, w;
	VGImageFormat filterSpaceFormat;
	AMPixel32ConverterFunction converter;
	AMImageSampler sampler;
	AMImageSamplerParams samplerParams;
	AMuint32 dstBytesPerPixel, r32, g32, b32, a32, rgbClamp;
	AMint64 r, g, b, a, scaleFixed, biasFixed;
	AMuint8 *dst8;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(kernel);
	AM_ASSERT(sTileColor);
	AM_ASSERT(kernelWidth > 0 && kernelHeight > 0);

	// transform scale and bias values in fixed point 24.8
	scaleFixed = INT64_FROM_FLOAT(scale * FIXED_PRECISION_F + 0.5f);
	biasFixed = INT64_FROM_FLOAT(bias * 256.0f * FIXED_PRECISION_F + 0.5f);

	// find valid width and height
	width = AM_MIN(src->width, dst->width);
	height = AM_MIN(src->height, dst->height);

	if (filterLinear) {
		if (filterPreMultiplied) {
			filterSpaceFormat = VG_lRGBA_8888_PRE;
			converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][3];
			r32 = (AMuint32)amFloorf(amGammaInvConversion(sTileColor[AM_R]) * sTileColor[AM_A] * 255.0f + 0.5f);
			g32 = (AMuint32)amFloorf(amGammaInvConversion(sTileColor[AM_G]) * sTileColor[AM_A] * 255.0f + 0.5f);
			b32 = (AMuint32)amFloorf(amGammaInvConversion(sTileColor[AM_B]) * sTileColor[AM_A] * 255.0f + 0.5f);
		}
		else {
			filterSpaceFormat = VG_lRGBA_8888;
			converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][2];
			r32 = (AMuint32)amFloorf(amGammaInvConversion(sTileColor[AM_R]) * 255.0f + 0.5f);
			g32 = (AMuint32)amFloorf(amGammaInvConversion(sTileColor[AM_G]) * 255.0f + 0.5f);
			b32 = (AMuint32)amFloorf(amGammaInvConversion(sTileColor[AM_B]) * 255.0f + 0.5f);
		}
	}
	else {
		if (filterPreMultiplied) {
			filterSpaceFormat = VG_sRGBA_8888_PRE;
			converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][1];
			r32 = (AMuint32)amFloorf(sTileColor[AM_R] * sTileColor[AM_A] * 255.0f + 0.5f);
			g32 = (AMuint32)amFloorf(sTileColor[AM_G] * sTileColor[AM_A] * 255.0f + 0.5f);
			b32 = (AMuint32)amFloorf(sTileColor[AM_B] * sTileColor[AM_A] * 255.0f + 0.5f);
		}
		else {
			filterSpaceFormat = VG_sRGBA_8888;
			converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][0];
			r32 = (AMuint32)amFloorf(sTileColor[AM_R] * 255.0f + 0.5f);
			g32 = (AMuint32)amFloorf(sTileColor[AM_G] * 255.0f + 0.5f);
			b32 = (AMuint32)amFloorf(sTileColor[AM_B] * 255.0f + 0.5f);
		}
	}
	a32 = (AMuint32)amFloorf(sTileColor[AM_A] * 255.0f + 0.5f);
	AM_ASSERT(converter);

	// extract pixel sampler
	sampler = amImageSamplerGet(src->format);
	AM_ASSERT(sampler);
	samplerParams.image = src;
	samplerParams.srcIdx = AM_FMT_GET_INDEX(src->format);
	samplerParams.tilingMode = tilingMode;
	samplerParams.tileFillColor = (r32 << 24) | (g32 << 16) | (b32 << 8) | a32;;
	samplerParams.dstIdx = AM_FMT_GET_INDEX(filterSpaceFormat);
	samplerParams.bilinear = AM_FALSE;
#if (AM_OPENVG_VERSION >= 110)
	samplerParams.colorTransformation = NULL;
#endif

	switch (dstBits) {
		case 1:
			rightShift = 3;
			break;
		case 4:
			rightShift = 1;
			break;
		default:
			rightShift = 0;
			break;
	}

	dst8 = (AMuint8 *)dst->pixels;
	dstBytesPerPixel = amImageBytesPerPixel(dst->format);

	// according to OpenVG specifications:
	// "if the VG_FILTER_CHANNEL_MASK parameter has its VG_ALPHA bit enabled, the convolved alpha value is
	// used; otherwise, the original alpha value of each source pixel is used in the final destination
	// premultiplication step"
	if (filterChannels & VG_ALPHA) {
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; ++j) {

				INT64_ZERO_SET(r);
				INT64_ZERO_SET(g);
				INT64_ZERO_SET(b);
				INT64_ZERO_SET(a);
				for (k = 0; k < kernelHeight; ++k) {

					samplerParams.y = (i + k - shiftY) << 16;
					for (w = 0; w < kernelWidth; ++w) {

						AMint16 kernVal = KERNEL(kernelWidth - w - 1, kernelHeight - k - 1);
						AMuint32 rgba32;
						AMint32 rTmp, gTmp, bTmp, aTmp;

						samplerParams.x = (j + w - shiftX) << 16;						
						rgba32 = sampler(&samplerParams);
						rTmp = (AMint32)(rgba32 >> 24);
						gTmp = (AMint32)((rgba32 >> 16) & 0xFF);
						bTmp = (AMint32)((rgba32 >> 8) & 0xFF);
						aTmp = (AMint32)(rgba32 & 0xFF);
						r = INT64_ADD(r, INT32_INT32_MUL(rTmp, kernVal));
						g = INT64_ADD(g, INT32_INT32_MUL(gTmp, kernVal));
						b = INT64_ADD(b, INT32_INT32_MUL(bTmp, kernVal));
						a = INT64_ADD(a, INT32_INT32_MUL(aTmp, kernVal));
					}
				}

				r = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(r, scaleFixed), biasFixed), FIXED_PRECISION_BITS);
				g = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(g, scaleFixed), biasFixed), FIXED_PRECISION_BITS);
				b = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(b, scaleFixed), biasFixed), FIXED_PRECISION_BITS);
				a = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(a, scaleFixed), biasFixed), FIXED_PRECISION_BITS);
				a32 = INT64_CLAMP_ZERO_UINT8(a, 0xFF);
				// take care of premultiplied formats
				rgbClamp = filterPreMultiplied ? a32 : 0xFF;
				r32 = INT64_CLAMP_ZERO_UINT8(r, rgbClamp);
				g32 = INT64_CLAMP_ZERO_UINT8(g, rgbClamp);
				b32 = INT64_CLAMP_ZERO_UINT8(b, rgbClamp);
				// convert the pixel and go to the next one
				if (dstBits > 4)
					converter((void *)&dst8[j * dstBytesPerPixel], r32, g32, b32, a32, 0, filterChannels);
				else
					converter((void *)&dst8[j >> rightShift], r32, g32, b32, a32, j, filterChannels);
			}
			dst8 += dst->dataStride;
		}
	}
	else {
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; ++j) {
				
				INT64_ZERO_SET(r);
				INT64_ZERO_SET(g);
				INT64_ZERO_SET(b);
				for (k = 0; k < kernelHeight; ++k) {

					samplerParams.y = (i + k - shiftY) << 16;
					for (w = 0; w < kernelWidth; ++w) {
						AMint16 kernVal = KERNEL(kernelWidth - w - 1, kernelHeight - k - 1);
						AMuint32 rgba32;
						AMint32 rTmp, gTmp, bTmp;

						samplerParams.x = (j + w - shiftX) << 16;
						rgba32 = sampler(&samplerParams);
						rTmp = (AMint32)(rgba32 >> 24);
						gTmp = (AMint32)((rgba32 >> 16) & 0xFF);
						bTmp = (AMint32)((rgba32 >> 8) & 0xFF);
						r = INT64_ADD(r, INT32_INT32_MUL(rTmp, kernVal));
						g = INT64_ADD(g, INT32_INT32_MUL(gTmp, kernVal));
						b = INT64_ADD(b, INT32_INT32_MUL(bTmp, kernVal));
					}
				}
				
				r = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(r, scaleFixed), biasFixed), FIXED_PRECISION_BITS);
				g = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(g, scaleFixed), biasFixed), FIXED_PRECISION_BITS);
				b = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(b, scaleFixed), biasFixed), FIXED_PRECISION_BITS);
				// get original (unfiltered alpha value)
				samplerParams.x = j << 16;
				samplerParams.y = i << 16;
				a32 = sampler(&samplerParams) & 0xFF;
				// take care of premultiplied formats
				rgbClamp = filterPreMultiplied ? a32 : 0xFF;
				r32 = INT64_CLAMP_ZERO_UINT8(r, rgbClamp);
				g32 = INT64_CLAMP_ZERO_UINT8(g, rgbClamp);
				b32 = INT64_CLAMP_ZERO_UINT8(b, rgbClamp);
				// convert the pixel and go to the next one
				if (dstBits > 4)
					converter((void *)&dst8[j * dstBytesPerPixel], r32, g32, b32, a32, 0, filterChannels);
				else
					converter((void *)&dst8[j >> rightShift], r32, g32, b32, a32, j, filterChannels);
			}
			dst8 += dst->dataStride;
		}
	}

#if defined(AM_GLE) || defined(AM_GLS)
	// invalidate image textures
	amImageTexturesInvalidate(dst, context);
#endif

	#undef KERNEL
	#undef FIXED_PRECISION_BITS
	#undef FIXED_PRECISION_F
}

/*!
	\brief This function applies a user-supplied separable convolution kernel to a source image.
	\param dst destination image.
	\param src source image.
	\param kernelWidth width of the convolution kernel.
	\param kernelHeight height of the convolution kernel.
	\param shiftX horizontal translation between source and destination images, in pixels.
	\param shiftY vertical translation between source and destination images, in pixels.
	\param kernelX horizontal convolution kernel.
	\param kernelY vertical convolution kernel.
	\param scaleX scale factor to apply to the result of the horizontal convolution, at each pixel.
	\param scaleY scale factor to apply to the result of the vertical convolution, at each pixel.
	\param bias value to add to the result of the convolution, at each pixel.
	\param tilingMode tiling mode to use when reading pixels from source image.
	\param sTileColor color to use for VG_TILE_FILL tiling mode, in non-linear unpremultiplied color space.
	\param filterLinear if AM_TRUE, apply the filter in the linear color space.
	\param filterPreMultiplied if AM_TRUE, apply the filter in the premultiplied color space.
	\param filterChannels specifies which destination channels are to be written.
	\param context input context containing the pixel converters table.
	\return VG_OUT_OF_MEMORY_ERROR for memory allocation errors, else VG_NO_ERROR.
*/
AMbool amFltSeparableConvolve(AMImage *dst,
							  const AMImage *src,
							  const AMint32 kernelWidth,
							  const AMint32 kernelHeight,
							  const AMint32 shiftX,
							  const AMint32 shiftY,
							  const AMint16 *kernelX,
							  const AMint16 *kernelY,
							  const AMfloat scaleX,
							  const AMfloat scaleY,
							  const AMfloat bias,
							  const VGTilingMode tilingMode,
							  const AMfloat *sTileColor,
							  const AMbool filterLinear,
							  const AMbool filterPreMultiplied,
							  const VGbitfield filterChannels,
							  const AMContext *context) {

	#define FIXED_PRECISION_BITS 24
	#define FIXED_PRECISION_F (65536.0f * 256.0f)

	AMuint32 dstIdx = AM_FMT_GET_INDEX(dst->format);
	AMuint32 dstBits = pxlFormatTable[dstIdx][FMT_BITS];
	AMuint32 rightShift, r32, g32, b32, a32;
	AMint32 *dst32, width, height, i, j, k, w;
	AMImage128Sample tmpImage;
	VGImageFormat filterSpaceFormat;
	AMPixel32ConverterFunction converter;
	AMImageSampler sampler;
	AMImageSamplerParams samplerParams;
	AMuint32 dstBytesPerPixel;
	AMint64 r, g, b, a, scaleFixedX, scaleFixedY, biasFixed, tmp64;
	AMuint8 *dst8;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(kernelX);
	AM_ASSERT(kernelY);
	AM_ASSERT(sTileColor);
	AM_ASSERT(kernelWidth > 0 && kernelHeight > 0);
	AM_ASSERT(context);

	// transform scale and bias values in fixed point 24.8
	scaleFixedX = INT64_FROM_FLOAT(scaleX * FIXED_PRECISION_F + 0.5f);
	scaleFixedY = INT64_FROM_FLOAT(scaleY * FIXED_PRECISION_F + 0.5f);
	biasFixed = INT64_FROM_FLOAT(bias * 256.0f * FIXED_PRECISION_F + 0.5f);

	// find valid width and height
	width = AM_MIN(src->width, dst->width);
	height = AM_MIN(src->height, dst->height);

	tmpImage.pixels = (AMint32 *)amMalloc(width * height * 4 * sizeof(AMint32));
	if (!tmpImage.pixels)
		return AM_FALSE;

	// initialize the temporary image, according to the filter application space
	if (filterLinear) {
		if (filterPreMultiplied) {
			filterSpaceFormat = VG_lRGBA_8888_PRE;
			converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][3];
			r32 = (AMuint32)amFloorf(amGammaInvConversion(sTileColor[AM_R]) * sTileColor[AM_A] * 255.0f + 0.5f);
			g32 = (AMuint32)amFloorf(amGammaInvConversion(sTileColor[AM_G]) * sTileColor[AM_A] * 255.0f + 0.5f);
			b32 = (AMuint32)amFloorf(amGammaInvConversion(sTileColor[AM_B]) * sTileColor[AM_A] * 255.0f + 0.5f);
		}
		else {
			filterSpaceFormat = VG_lRGBA_8888;
			converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][2];
			r32 = (AMuint32)amFloorf(amGammaInvConversion(sTileColor[AM_R]) * 255.0f + 0.5f);
			g32 = (AMuint32)amFloorf(amGammaInvConversion(sTileColor[AM_G]) * 255.0f + 0.5f);
			b32 = (AMuint32)amFloorf(amGammaInvConversion(sTileColor[AM_B]) * 255.0f + 0.5f);
		}
	}
	else {
		if (filterPreMultiplied) {
			filterSpaceFormat = VG_sRGBA_8888_PRE;
			converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][1];
			r32 = (AMuint32)amFloorf(sTileColor[AM_R] * sTileColor[AM_A] * 255.0f + 0.5f);
			g32 = (AMuint32)amFloorf(sTileColor[AM_G] * sTileColor[AM_A] * 255.0f + 0.5f);
			b32 = (AMuint32)amFloorf(sTileColor[AM_B] * sTileColor[AM_A] * 255.0f + 0.5f);
		}
		else {
			filterSpaceFormat = VG_sRGBA_8888;
			converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][0];
			r32 = (AMuint32)amFloorf(sTileColor[AM_R] * 255.0f + 0.5f);
			g32 = (AMuint32)amFloorf(sTileColor[AM_G] * 255.0f + 0.5f);
			b32 = (AMuint32)amFloorf(sTileColor[AM_B] * 255.0f + 0.5f);
		}
	}
	a32 = (AMuint32)amFloorf(sTileColor[AM_A] * 255.0f + 0.5f);
	AM_ASSERT(converter);

	// extract pixel sampler
	sampler = amImageSamplerGet(src->format);
	AM_ASSERT(sampler);
	samplerParams.image = src;
	samplerParams.srcIdx = AM_FMT_GET_INDEX(src->format);
	samplerParams.tilingMode = tilingMode;
	samplerParams.tileFillColor = (r32 << 24) | (g32 << 16) | (b32 << 8) | a32;
	samplerParams.dstIdx = AM_FMT_GET_INDEX(filterSpaceFormat);
	samplerParams.bilinear = AM_FALSE;
#if (AM_OPENVG_VERSION >= 110)
	samplerParams.colorTransformation = NULL;
#endif
	
	// first apply horizontal kernel from actualSrc to tmpImage
	dst32 = tmpImage.pixels;
	for (i = 0; i < height; ++i) {

		samplerParams.y = i << 16;
		for (j = 0; j < width; ++j) {

			INT64_ZERO_SET(r);
			INT64_ZERO_SET(g);
			INT64_ZERO_SET(b);
			INT64_ZERO_SET(a);
			for (w = 0; w < kernelWidth; ++w) {
				AMint16 kernValX = kernelX[kernelWidth - w - 1];
				AMuint32 rgba32;
				AMint32 rTmp, gTmp, bTmp, aTmp;

				samplerParams.x = (j + w - shiftX) << 16;
				rgba32 = sampler(&samplerParams);
				rTmp = (AMint32)(rgba32 >> 24);
				gTmp = (AMint32)((rgba32 >> 16) & 0xFF);
				bTmp = (AMint32)((rgba32 >> 8) & 0xFF);
				aTmp = (AMint32)(rgba32 & 0xFF);
				r = INT64_ADD(r, INT32_INT32_MUL(rTmp, kernValX));
				g = INT64_ADD(g, INT32_INT32_MUL(gTmp, kernValX));
				b = INT64_ADD(b, INT32_INT32_MUL(bTmp, kernValX));
				a = INT64_ADD(a, INT32_INT32_MUL(aTmp, kernValX));
			}
			// apply scaling for horizontal kernel
			r = INT64_RSHIFT(INT64_INT64_MUL(r, scaleFixedX), FIXED_PRECISION_BITS);
			g = INT64_RSHIFT(INT64_INT64_MUL(g, scaleFixedX), FIXED_PRECISION_BITS);
			b = INT64_RSHIFT(INT64_INT64_MUL(b, scaleFixedX), FIXED_PRECISION_BITS);
			a = INT64_RSHIFT(INT64_INT64_MUL(a, scaleFixedX), FIXED_PRECISION_BITS);
			// write the filtered pixel
			*dst32++ = INT64_INT32_CAST(r);
			*dst32++ = INT64_INT32_CAST(g);
			*dst32++ = INT64_INT32_CAST(b);
			*dst32++ = INT64_INT32_CAST(a);
		}
	}

	// we must take care of apply horizontal filter to tile color!
	INT64_ZERO_SET(r);
	INT64_ZERO_SET(g);
	INT64_ZERO_SET(b);
	INT64_ZERO_SET(a);
	for (w = 0; w < kernelWidth; ++w) {

		AMint16 kernValX = kernelX[kernelWidth - w - 1];

		r = INT64_ADD(r, INT32_INT32_MUL(r32, kernValX));
		g = INT64_ADD(g, INT32_INT32_MUL(g32, kernValX));
		b = INT64_ADD(b, INT32_INT32_MUL(b32, kernValX));
		a = INT64_ADD(a, INT32_INT32_MUL(a32, kernValX));
	}
	// fill tmpImage fields
	tmpImage.width = width;
	tmpImage.height = height;
	tmpImage.pow2 = amPow2Check(width) && amPow2Check(height);
	if (tmpImage.pow2) {
		tmpImage.widthShift = amPow2Shift(width);
		tmpImage.heightShift = amPow2Shift(height);
	}
	tmpImage.tileMode = tilingMode;

	tmp64 = INT64_RSHIFT(INT64_INT64_MUL(r, scaleFixedX), FIXED_PRECISION_BITS);
	tmpImage.tileColorR = INT64_INT32_CAST(tmp64);
	tmp64 = INT64_RSHIFT(INT64_INT64_MUL(g, scaleFixedX), FIXED_PRECISION_BITS);
	tmpImage.tileColorG = INT64_INT32_CAST(tmp64);
	tmp64 = INT64_RSHIFT(INT64_INT64_MUL(b, scaleFixedX), FIXED_PRECISION_BITS);
	tmpImage.tileColorB = INT64_INT32_CAST(tmp64);
	tmp64 = INT64_RSHIFT(INT64_INT64_MUL(a, scaleFixedX), FIXED_PRECISION_BITS);
	tmpImage.tileColorA = INT64_INT32_CAST(tmp64);

	switch (dstBits) {
		case 1:
			rightShift = 3;
			break;
		case 4:
			rightShift = 1;
			break;
		default:
			rightShift = 0;
			break;
	}

	// now apply vertical kernel from tmpImage to dst
	dst8 = (AMuint8 *)dst->pixels;
	dstBytesPerPixel = amImageBytesPerPixel(dst->format);

	// according to OpenVG specifications:
	// "if the VG_FILTER_CHANNEL_MASK parameter has its VG_ALPHA bit enabled, the convolved alpha value is
	// used; otherwise, the original alpha value of each source pixel is used in the final destination
	// premultiplication step"
	if (filterChannels & VG_ALPHA) {
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; ++j) {

				AMuint32 rgbClamp;

				INT64_ZERO_SET(r);
				INT64_ZERO_SET(g);
				INT64_ZERO_SET(b);
				INT64_ZERO_SET(a);
				for (k = 0; k < kernelHeight; ++k) {

					AMint32 rTmp, gTmp, bTmp, aTmp;
					AMint16 kernValY = kernelY[kernelHeight - k - 1];

					amGetImage128Pixel(&rTmp, &gTmp, &bTmp, &aTmp, &tmpImage, j, i + k - shiftY);
					r = INT64_ADD(r, INT32_INT32_MUL(rTmp, kernValY));
					g = INT64_ADD(g, INT32_INT32_MUL(gTmp, kernValY));
					b = INT64_ADD(b, INT32_INT32_MUL(bTmp, kernValY));
					a = INT64_ADD(a, INT32_INT32_MUL(aTmp, kernValY));
				}
				
				r = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(r, scaleFixedY), biasFixed), FIXED_PRECISION_BITS);
				g = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(g, scaleFixedY), biasFixed), FIXED_PRECISION_BITS);
				b = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(b, scaleFixedY), biasFixed), FIXED_PRECISION_BITS);
				a = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(a, scaleFixedY), biasFixed), FIXED_PRECISION_BITS);
				a32 = INT64_CLAMP_ZERO_UINT8(a, 0xFF);
				// take care of premultiplied formats
				rgbClamp = filterPreMultiplied ? a32 : 0xFF;
				r32 = INT64_CLAMP_ZERO_UINT8(r, rgbClamp);
				g32 = INT64_CLAMP_ZERO_UINT8(g, rgbClamp);
				b32 = INT64_CLAMP_ZERO_UINT8(b, rgbClamp);
				// convert the pixel and go to the next one
				if (dstBits > 4)
					converter((void *)&dst8[j * dstBytesPerPixel], r32, g32, b32, a32, 0, filterChannels);
				else
					converter((void *)&dst8[j >> rightShift], r32, g32, b32, a32, j, filterChannels);
			}
			dst8 += dst->dataStride;
		}
	}
	else {
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; ++j) {

				AMuint32 rgbClamp;

				INT64_ZERO_SET(r);
				INT64_ZERO_SET(g);
				INT64_ZERO_SET(b);
				for (k = 0; k < kernelHeight; ++k) {

					AMint32 rTmp, gTmp, bTmp, aTmp;
					AMint16 kernValY = kernelY[kernelHeight - k - 1];

					amGetImage128Pixel(&rTmp, &gTmp, &bTmp, &aTmp, &tmpImage, j, i + k - shiftY);
					r = INT64_ADD(r, INT32_INT32_MUL(rTmp, kernValY));
					g = INT64_ADD(g, INT32_INT32_MUL(gTmp, kernValY));
					b = INT64_ADD(b, INT32_INT32_MUL(bTmp, kernValY));
				}
				r = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(r, scaleFixedY), biasFixed), FIXED_PRECISION_BITS);
				g = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(g, scaleFixedY), biasFixed), FIXED_PRECISION_BITS);
				b = INT64_RSHIFT(INT64_ADD(INT64_INT64_MUL(b, scaleFixedY), biasFixed), FIXED_PRECISION_BITS);

				// get original (unfiltered alpha value)
				samplerParams.x = j << 16;
				samplerParams.y = (i + k - shiftY) << 16;
				a32 = sampler(&samplerParams) & 0xFF;
				// take care of premultiplied formats
				rgbClamp = filterPreMultiplied ? a32 : 0xFF;
				r32 = INT64_CLAMP_ZERO_UINT8(r, rgbClamp);
				g32 = INT64_CLAMP_ZERO_UINT8(g, rgbClamp);
				b32 = INT64_CLAMP_ZERO_UINT8(b, rgbClamp);
				// convert the pixel
				if (dstBits > 4)
					converter((void *)&dst8[j * dstBytesPerPixel], r32, g32, b32, a32, 0, filterChannels);
				else
					converter((void *)&dst8[j >> rightShift], r32, g32, b32, a32, j, filterChannels);
			}
			dst8 += dst->dataStride;
		}
	}

	amFree(tmpImage.pixels);
#if defined(AM_GLE) || defined(AM_GLS)
	// invalidate image textures
	amImageTexturesInvalidate(dst, context);
#endif
	return AM_TRUE;

	#undef FIXED_PRECISION_BITS
	#undef FIXED_PRECISION_F
}

/*!
	\brief This function computes the convolution of a source image with a separable kernel defined in each
	dimension by the Gaussian function.
	\param dst destination image.
	\param src source image.
	\param stdDeviationX horizontal standard deviation.
	\param stdDeviationY vertical standard deviation.
	\param tilingMode tiling mode to use when reading pixels from source image.
	\param sTileColor color to use for VG_TILE_FILL tiling mode, in non-linear unpremultiplied color space.
	\param filterLinear if AM_TRUE, apply the filter in the linear color space.
	\param filterPreMultiplied if AM_TRUE, apply the filter in the premultiplied color space.
	\param filterChannels specifies which destination channels are to be written.
	\param context input context containing the pixel converters table.
	\return VG_OUT_OF_MEMORY_ERROR for memory allocation errors, else VG_NO_ERROR.
*/
AMbool amFltGaussianBlur(AMImage *dst,
						 const AMImage *src,
						 const AMfloat stdDeviationX,
						 const AMfloat stdDeviationY,
						 const VGTilingMode tilingMode,
						 const AMfloat *sTileColor,
						 const AMbool filterLinear,
						 const AMbool filterPreMultiplied,
						 const VGbitfield filterChannels,
						 AMContext *context) {

	#define FIXED_SCALE 16384.0f
	#define AM_MIN_STD_DEVIATION 0.229f

	AMfloat gx, gy, stdx2, stdy2, sumX, sumY, w;
	AMint32 i, orderX, orderY, maxOrder;

	AM_ASSERT(src);
	AM_ASSERT(dst);
	AM_ASSERT(sTileColor);
	AM_ASSERT(stdDeviationX > 0.0f && stdDeviationY > 0.0f);
	AM_ASSERT(context);

	// calculate max permitted order for Gaussian kernels
	maxOrder = 6 * context->maxGaussianStdDeviation + 1;

	orderX = (AMint32)(6.0f * stdDeviationX + 1.0f);
	if (orderX <= 1 || stdDeviationX < AM_MIN_STD_DEVIATION)
		orderX = 1;
	else {
		if ((orderX & 1) == 0)
			orderX++;
		orderX = AM_MIN(orderX, maxOrder);
	}

	orderY = (AMint32)(6.0f * stdDeviationY + 1.0f);
	if (orderY <= 1 || stdDeviationY < AM_MIN_STD_DEVIATION)
		orderY = 1;
	else {
		if ((orderY & 1) == 0)
			orderY++;
		orderY = AM_MIN(orderY, maxOrder);
	}
	AM_ASSERT((orderX & 1) == 1);
	AM_ASSERT((orderY & 1) == 1);

	stdx2 = stdDeviationX * stdDeviationX;
	stdy2 = stdDeviationY * stdDeviationY;

	// Gaussian filter
	if (orderX == 1) {
		context->gaussianKernelX[0] = 1;
		sumX = 1.0f;
	}
	else {
		sumX = 0.0f;
		w = 1.0f / amSqrtf(AM_2PI * stdx2);
		for (i = -(orderX >> 1); i <= (orderX >> 1); ++i) {
			
			AMfloat i2Float = (AMfloat)i * (AMfloat)i;

			gx = w * amExpf(-i2Float / (2.0f * stdx2));
			AM_ASSERT(i + (orderX >> 1) >= 0);
			AM_ASSERT(i + (orderX >> 1) < maxOrder);
			context->gaussianKernelX[i + (orderX >> 1)] = (AMint16)(gx * FIXED_SCALE);
			sumX += (AMfloat)context->gaussianKernelX[i + (orderX >> 1)];
		}
	}
	if (orderY == 1) {
		context->gaussianKernelY[0] = 1;
		sumY = 1.0f;
	}
	else {
		sumY = 0.0f;
		w = 1.0f / amSqrtf(AM_2PI * stdy2);
		for (i = -(orderY >> 1); i <= (orderY >> 1); ++i) {

			AMfloat i2Float = (AMfloat)i * (AMfloat)i;

			gy = w * amExpf(-i2Float / (2.0f * stdy2));
			AM_ASSERT(i + (orderY >> 1) >= 0);
			AM_ASSERT(i + (orderY >> 1) < maxOrder);
			context->gaussianKernelY[i + (orderY >> 1)] = (AMint16)(gy * FIXED_SCALE);
			sumY += (AMfloat)context->gaussianKernelY[i + (orderY >> 1)];
		}
	}

	return amFltSeparableConvolve(dst, src, orderX, orderY, orderX >> 1, orderY >> 1, context->gaussianKernelX, context->gaussianKernelY,
								  1.0f / sumX, 1.0f / sumY, 0.0f, tilingMode, sTileColor,
								  filterLinear, filterPreMultiplied, filterChannels, context);
	#undef FIXED_SCALE
	#undef AM_MIN_STD_DEVIATION
}

/*!
	\brief This function passes each image channel of the source image through a separate lookup table.
	Each channel of the source pixel is used as an index into the lookup table for that channel.
	Each LUT parameter should contain 256 unsigned bytes entries.
	\param dst destination image.
	\param src source image.
	\param redLUT red component lookup table.
	\param greenLUT green component lookup table.
	\param blueLUT blue component lookup table.
	\param alphaLUT alpha component lookup table.
	\param outputLinear AM_TRUE if lookup values are in linear color space, else AM_FALSE.
	\param outputPremultiplied AM_TRUE if lookup values are in premultiplied color space, else AM_FALSE.
	\param filterLinear if AM_TRUE, apply the filter in the linear color space.
	\param filterPreMultiplied if AM_TRUE, apply the filter in the premultiplied color space.
	\param filterChannels specifies which destination channels are to be written.
	\param context input context containing the pixel converters table.
*/
void amFltLookup(AMImage *dst,
				 const AMImage *src,
				 const AMuint8 *redLUT,
				 const AMuint8 *greenLUT,
				 const AMuint8 *blueLUT,
				 const AMuint8 *alphaLUT,
				 const AMbool outputLinear,
				 const AMbool outputPremultiplied,
				 const AMbool filterLinear,
				 const AMbool filterPreMultiplied,
				 const VGbitfield filterChannels,
				 const AMContext *context) {

	AMuint32 dstIdx = AM_FMT_GET_INDEX(dst->format);
	AMuint32 dstBits = pxlFormatTable[dstIdx][FMT_BITS];
	AMuint32 rightShift;
	AMint32 width, height, i, j, lutDataFormat;
	VGImageFormat filterSpaceFormat;
	AMPixel32ConverterFunction converter;
	AMImageSampler sampler;
	AMImageSamplerParams samplerParams;
	AMuint8 *dst8;
	AMuint32 dstBytesPerPixel;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(redLUT);
	AM_ASSERT(greenLUT);
	AM_ASSERT(blueLUT);
	AM_ASSERT(alphaLUT);
	AM_ASSERT(context);

	// according to outputLinear and outputPremultiplied, derive index of 'source' values
	if (outputLinear)
		lutDataFormat = outputPremultiplied ? 3 : 2;
	else
		lutDataFormat = outputPremultiplied ? 1 : 0;

	// find valid width and height
	width = AM_MIN(src->width, dst->width);
	height = AM_MIN(src->height, dst->height);

	if (filterLinear)
		filterSpaceFormat = (filterPreMultiplied) ? VG_lRGBA_8888_PRE : VG_lRGBA_8888;
	else
		filterSpaceFormat = (filterPreMultiplied) ? VG_sRGBA_8888_PRE : VG_sRGBA_8888;

	// extract pixel converter function
	converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][lutDataFormat];
	AM_ASSERT(converter);

	// extract pixel sampler
	sampler = amImageSamplerGet(src->format);
	AM_ASSERT(sampler);
	samplerParams.image = src;
	samplerParams.srcIdx = AM_FMT_GET_INDEX(src->format);
	samplerParams.tilingMode = VG_TILE_PAD;
	samplerParams.dstIdx = AM_FMT_GET_INDEX(filterSpaceFormat);
	samplerParams.bilinear = AM_FALSE;
#if (AM_OPENVG_VERSION >= 110)
	samplerParams.colorTransformation = NULL;
#endif

	switch (dstBits) {
		case 1:
			rightShift = 3;
			break;
		case 4:
			rightShift = 1;
			break;
		default:
			rightShift = 0;
			break;
	}

	dst8 = (AMuint8 *)dst->pixels;
	dstBytesPerPixel = amImageBytesPerPixel(dst->format);

	for (i = 0; i < height; ++i) {
		
		samplerParams.y = i << 16;
		for (j = 0; j < width; ++j) {

			AMuint32 rgbClamp, rgba32, r, g, b, a;

			samplerParams.x = j << 16;	
			rgba32 = sampler(&samplerParams);
			r = rgba32 >> 24;
			g = (rgba32 >> 16) & 0xFF;
			b = (rgba32 >> 8) & 0xFF;
			a = rgba32 & 0xFF;

			r = (AMuint32)redLUT[r];
			g = (AMuint32)greenLUT[g];
			b = (AMuint32)blueLUT[b];
			a = (AMuint32)alphaLUT[a];
			// take care of premultiplied formats
			rgbClamp = outputPremultiplied ? a : 0xFF;
			r = AM_MIN(r, rgbClamp);
			g = AM_MIN(g, rgbClamp);
			b = AM_MIN(b, rgbClamp);

			if (dstBits > 4)
				converter((void *)&dst8[j * dstBytesPerPixel], r, g, b, a, 0, filterChannels);
			else
				converter((void *)&dst8[j >> rightShift], r, g, b, a, j, filterChannels);
		}
		dst8 += dst->dataStride;
	}

#if defined(AM_GLE) || defined(AM_GLS)
	// invalidate image textures
	amImageTexturesInvalidate(dst, context);
#endif
}

/*!
	\brief This function passes a single image channel of the source image, selected by the sourceChannel
	parameter, through a combined lookup table that produces whole pixel values.
	\param dst destination image.
	\param src source image.
	\param lookupTable it contains 256 4-byte aligned entries in an	RGBA_8888 pixel format, which is
	interpreted as lRGBA_8888, lRGBA_8888_PRE, sRGBA_8888, or sRGBA_8888_PRE, depending on the
	values of outputLinear and outputPremultiplied.
	\param sourceChannel an index into the lookup table. If the source image is in a single-channel
	grayscale or alpha-only format, this parameter is ignored and the single channel is used.
	\param outputLinear AM_TRUE if lookup values are in linear color space, else AM_FALSE.
	\param outputPremultiplied AM_TRUE if lookup values are in premultiplied color space, else AM_FALSE.
	\param filterLinear if AM_TRUE, apply the filter in the linear color space.
	\param filterPreMultiplied if AM_TRUE, apply the filter in the premultiplied color space.
	\param filterChannels specifies which destination channels are to be written.
	\param context input context containing the pixel converters table.
*/
void amFltLookupSingle(AMImage *dst,
					   const AMImage *src,
					   const AMuint32 *lookupTable,
					   const VGImageChannel sourceChannel,
					   const AMbool outputLinear,
					   const AMbool outputPremultiplied,
					   const AMbool filterLinear,
					   const AMbool filterPreMultiplied,
					   const VGbitfield filterChannels,
					   const AMContext *context) {

	AMuint32 srcIdx = AM_FMT_GET_INDEX(src->format);
	AMuint32 srcBits = pxlFormatTable[srcIdx][FMT_BITS];
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dst->format);
	AMuint32 dstBits = pxlFormatTable[dstIdx][FMT_BITS];
	AMuint32 rightShift;
	AMint32 width, height, i, j, lutDataFormat;
	AMPixel32ConverterFunction converter;
	AMuint32 dstBytesPerPixel;
	AMuint8 *dst8;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(lookupTable);
	AM_ASSERT(context);

	// according to outputLinear and outputPremultiplied, derive index of 'source' values
	if (outputLinear)
		lutDataFormat = outputPremultiplied ? 3 : 2;
	else
		lutDataFormat = outputPremultiplied ? 1 : 0;

	converter = context->pxlConverters[AM_FMT_GET_INDEX(dst->format)][lutDataFormat];
	AM_ASSERT(converter);
	dst8 = (AMuint8 *)dst->pixels;
	dstBytesPerPixel = amImageBytesPerPixel(dst->format);

	// find valid width and height
	width = AM_MIN(src->width, dst->width);
	height = AM_MIN(src->height, dst->height);

	switch (dstBits) {
		case 1:
			rightShift = 3;
			break;
		case 4:
			rightShift = 1;
			break;
		default:
			rightShift = 0;
			break;
	}

	// multi channel color source image (i.e. src->format not sL8, lL8, A8, A4, A1, BW1)
	if (srcBits > 8) {

		VGImageFormat filterSpaceFormat;
		AMImageSampler sampler;
		AMImageSamplerParams samplerParams;
		AMuint32 srcShift;

		if (filterLinear)
			filterSpaceFormat = (filterPreMultiplied) ? VG_lRGBA_8888_PRE : VG_lRGBA_8888;
		else
			filterSpaceFormat = (filterPreMultiplied) ? VG_sRGBA_8888_PRE : VG_sRGBA_8888;

		switch (sourceChannel) {
			case VG_RED:
				srcShift = 24;
				break;
			case VG_GREEN:
				srcShift = 16;
				break;
			case VG_BLUE:
				srcShift = 8;
				break;
			default:
				srcShift = 0;
				break;
		}

		// extract pixel sampler
		sampler = amImageSamplerGet(src->format);
		AM_ASSERT(sampler);
		samplerParams.image = src;
		samplerParams.srcIdx = AM_FMT_GET_INDEX(src->format);
		samplerParams.tilingMode = VG_TILE_PAD;
		samplerParams.dstIdx = AM_FMT_GET_INDEX(filterSpaceFormat);
		samplerParams.bilinear = AM_FALSE;
	#if (AM_OPENVG_VERSION >= 110)
		samplerParams.colorTransformation = NULL;
	#endif

		// source is multi channel color image, destination image is aligned to 8bit
		for (i = 0; i < height; ++i) {
			samplerParams.y = i << 16;
			for (j = 0; j < width; ++j) {

				AMuint32 rgbClamp, r, g, b, a, rgba32;
				
				samplerParams.x = j << 16;
				rgba32 = sampler(&samplerParams);
				rgba32 = lookupTable[(rgba32 >> srcShift) & 0xFF];
				r = rgba32 >> 24;
				g = (rgba32 >> 16) & 0xFF;
				b = (rgba32 >> 8) & 0xFF;
				a = rgba32 & 0xFF;
				// take care of premultiplied formats
				rgbClamp = outputPremultiplied ? a : 0xFF;
				r = AM_MIN(r, rgbClamp);
				g = AM_MIN(g, rgbClamp);
				b = AM_MIN(b, rgbClamp);

				if (dstBits > 4)
					converter((void *)&dst8[j * dstBytesPerPixel], r, g, b, a, 0, filterChannels);
				else
					converter((void *)&dst8[j >> rightShift], r, g, b, a, j, filterChannels);
			}
			dst8 += dst->dataStride;
		}
	}
	// source is a single channel image
	else {
		AMuint8 *src8 = (AMuint8 *)src->pixels;
		
		// src is aligned to 8bit (sL8, lL8, A8)
		if (srcBits == 8) {

			for (i = 0; i < height; ++i) {
				for (j = 0; j < width; ++j) {

					AMuint32 rgba32, r, g, b, a, rgbClamp;

					// s --> l
					if (filterLinear && src->format == VG_sL_8)
						rgba32 = lookupTable[AM_GAMMA_INV_TABLE(src8[j])];
					else
					// l --> s
					if (!filterLinear && src->format == VG_lL_8)
						rgba32 = lookupTable[AM_GAMMA_TABLE(src8[j])];
					else
						rgba32 = lookupTable[src8[j]];

					r = rgba32 >> 24;
					g = (rgba32 >> 16) & 0xFF;
					b = (rgba32 >> 8) & 0xFF;
					a = rgba32 & 0xFF;
					// take care of premultiplied formats
					rgbClamp = outputPremultiplied ? a : 0xFF;

					r = AM_MIN(r, rgbClamp);
					g = AM_MIN(g, rgbClamp);
					b = AM_MIN(b, rgbClamp);

					if (dstBits > 4)
						converter((void *)&dst8[j * dstBytesPerPixel], r, g, b, a, 0, filterChannels);
					else
						converter((void *)&dst8[j >> rightShift], r, g, b, a, j, filterChannels);
				}
				src8 += src->dataStride;
				dst8 += dst->dataStride;
			}
		}
		else
	#if (AM_OPENVG_VERSION >= 110)
		// src is 4bit image (A4)
		if (srcBits == 4) {
			for (i = 0; i < height; ++i) {
				for (j = 0; j < width; ++j) {

					AMuint32 rgbClamp, r, g, b, a, rgba32;
					
					a = src8[j >> 1];
					a = ((a >> ((j & 1) << 2)) & 0x0F) * 17;
					AM_ASSERT(a <= 255);

					rgba32 = lookupTable[a];
					r = rgba32 >> 24;
					g = (rgba32 >> 16) & 0xFF;
					b = (rgba32 >> 8) & 0xFF;
					a = rgba32 & 0xFF;
					// take care of premultiplied formats
					rgbClamp = outputPremultiplied ? a : 0xFF;
					r = AM_MIN(r, rgbClamp);
					g = AM_MIN(g, rgbClamp);
					b = AM_MIN(b, rgbClamp);

					if (dstBits > 4)
						converter((void *)&dst8[j * dstBytesPerPixel], r, g, b, a, 0, filterChannels);
					else
						converter((void *)&dst8[j >> rightShift], r, g, b, a, j, filterChannels);
				}
				src8 += src->dataStride;
				dst8 += dst->dataStride;
			}
		}
		else
	#endif
		{
			// src is 1bit image (BW1, A1)
			AM_ASSERT(srcBits == 1);
			for (i = 0; i < height; ++i) {
				for (j = 0; j < width; ++j) {

					AMuint32 rgbClamp, r, g, b, a, rgba32;
					
					a = src8[j >> 3];
					a = (a >> (j & 0x07)) & 0x01;

					AM_ASSERT(a == 0 || a == 1);
					rgba32 = a ? lookupTable[255] : lookupTable[0];
					r = rgba32 >> 24;
					g = (rgba32 >> 16) & 0xFF;
					b = (rgba32 >> 8) & 0xFF;
					a = rgba32 & 0xFF;
					// take care of premultiplied formats
					rgbClamp = outputPremultiplied ? a : 0xFF;
					r = AM_MIN(r, rgbClamp);
					g = AM_MIN(g, rgbClamp);
					b = AM_MIN(b, rgbClamp);

					if (dstBits > 4)
						converter((void *)&dst8[j * dstBytesPerPixel], r, g, b, a, 0, filterChannels);
					else
						converter((void *)&dst8[j >> rightShift], r, g, b, a, j, filterChannels);
				}
				src8 += src->dataStride;
				dst8 += dst->dataStride;
			}
		}
	}
#if defined(AM_GLE) || defined(AM_GLS)
	// invalidate image textures
	amImageTexturesInvalidate(dst, context);
#endif
}

#endif // AM_LITE_PROFILE

// *********************************************************************
//                        Public implementations
// *********************************************************************

/*!
	\brief This function computes a linear combination, according to the given matrix, of color and alpha
	values from the source image at each pixel.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dst destination image.
	\param src source image.
	\param matrix input color matrix.
*/
VG_API_CALL void VG_API_ENTRY vgColorMatrix(VGImage dst,
                                            VGImage src,
                                            const VGfloat *matrix) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgColorMatrix");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

#if !defined(AM_LITE_PROFILE)
{
	AMfloat fixedMatrix[20];
	AMuint32 i;
	AMImage *dstImg;
	const AMImage *srcImg;
	AMbool fltFormatLinear = (currentContext->filterFormatLinear == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMbool fltFormatPremultiplied = (currentContext->filterFormatPremultiplied == VG_TRUE) ? AM_TRUE : AM_FALSE;

	// check for bad handles
	if (amCtxHandleValid(currentContext, dst) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgColorMatrix");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (amCtxHandleValid(currentContext, src) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgColorMatrix");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	dstImg = (AMImage *)currentContext->handles->createdHandlesList.data[dst];
	srcImg = (const AMImage *)currentContext->handles->createdHandlesList.data[src];
	AM_ASSERT(dstImg);
	AM_ASSERT(srcImg);

#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (dstImg->inUseByEgl || srcImg->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgColorMatrix");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
#endif
	// check for illegal arguments
	if (!matrix || !amPointerIsAligned(matrix, 4) || amImagesOverlap(srcImg, dstImg)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgColorMatrix");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// handle NaN and Inf values
	for (i = 0; i < 20; ++i)
		fixedMatrix[i] = amNanInfFix(matrix[i]);

	amFltColorMatrix(dstImg, srcImg, fixedMatrix, fltFormatLinear, fltFormatPremultiplied, currentContext->filterChannelMask, currentContext);
}
#else
	(void)dst;
	(void)src;
	(void)matrix;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgColorMatrix");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief This function applies a user-supplied convolution kernel to a source image.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dst destination image.
	\param src source image.
	\param kernelWidth width of the convolution kernel.
	\param kernelHeight height of the convolution kernel.
	\param shiftX horizontal translation between source and destination images, in pixels.
	\param shiftY vertical translation between source and destination images, in pixels.
	\param kernel convolution kernel.
	\param scale scale factor to apply to the result of the convolution, at each pixel.
	\param bias value to add to the result of the convolution, at each pixel.
	\param tilingMode tiling mode to use when reading pixels from source image.
*/
VG_API_CALL void VG_API_ENTRY vgConvolve(VGImage dst,
                                         VGImage src,
                                         VGint kernelWidth,
                                         VGint kernelHeight,
                                         VGint shiftX,
                                         VGint shiftY,
                                         const VGshort *kernel,
                                         VGfloat scale,
                                         VGfloat bias,
                                         VGTilingMode tilingMode) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

#if !defined(AM_LITE_PROFILE)
{
	AMImage *dstImg;
	const AMImage *srcImg;
	AMbool fltFormatLinear = (currentContext->filterFormatLinear == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMbool fltFormatPremultiplied = (currentContext->filterFormatPremultiplied == VG_TRUE) ? AM_TRUE : AM_FALSE;

	// check for bad handles
	if (amCtxHandleValid(currentContext, dst) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (amCtxHandleValid(currentContext, src) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	dstImg = (AMImage *)currentContext->handles->createdHandlesList.data[dst];
	srcImg = (const AMImage *)currentContext->handles->createdHandlesList.data[src];
	AM_ASSERT(dstImg);
	AM_ASSERT(srcImg);
#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (dstImg->inUseByEgl || srcImg->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
#endif
	// check for illegal arguments
	if (kernelWidth <= 0 || kernelWidth > currentContext->maxKernelSize ||
		kernelHeight <= 0 || kernelHeight > currentContext->maxKernelSize) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (!kernel || !amPointerIsAligned(kernel, 2) || tilingMode < VG_TILE_FILL || tilingMode > VG_TILE_REFLECT) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (amImagesOverlap(srcImg, dstImg)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// handle NaN and Inf values
	scale = amNanInfFix(scale);
	bias = amNanInfFix(bias);

	amFltConvolve(dstImg, srcImg, kernelWidth, kernelHeight, shiftX, shiftY, kernel,
				  scale, bias, tilingMode, currentContext->tileFillColor,
				  fltFormatLinear, fltFormatPremultiplied, currentContext->filterChannelMask, currentContext);
}
#else
	(void)dst;
    (void)src;
    (void)kernelWidth;
    (void)kernelHeight;
    (void)shiftX;
    (void)shiftY;
    (void)kernel;
    (void)scale;
    (void)bias;
    (void)tilingMode;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgConvolve");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief This function applies a user-supplied separable convolution kernel to a source image.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dst destination image.
	\param src source image.
	\param kernelWidth width of the convolution kernel.
	\param kernelHeight height of the convolution kernel.
	\param shiftX horizontal translation between source and destination images, in pixels.
	\param shiftY vertical translation between source and destination images, in pixels.
	\param kernelX horizontal convolution kernel.
	\param kernelY vertical convolution kernel.
	\param scale scale factor to apply to the result of the convolution, at each pixel.
	\param bias value to add to the result of the convolution, at each pixel.
	\param tilingMode tiling mode to use when reading pixels from source image.
*/
VG_API_CALL void VG_API_ENTRY vgSeparableConvolve(VGImage dst,
                                                  VGImage src,
                                                  VGint kernelWidth,
                                                  VGint kernelHeight,
                                                  VGint shiftX,
                                                  VGint shiftY,
                                                  const VGshort *kernelX,
                                                  const VGshort *kernelY,
                                                  VGfloat scale,
                                                  VGfloat bias,
                                                  VGTilingMode tilingMode) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSeparableConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

#if !defined(AM_LITE_PROFILE)
{
	AMImage *dstImg;
	const AMImage *srcImg;
	AMbool fltFormatLinear = (currentContext->filterFormatLinear == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMbool fltFormatPremultiplied = (currentContext->filterFormatPremultiplied == VG_TRUE) ? AM_TRUE : AM_FALSE;

	// check for bad handles
	if (amCtxHandleValid(currentContext, dst) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgSeparableConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (amCtxHandleValid(currentContext, src) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgSeparableConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	dstImg = (AMImage *)currentContext->handles->createdHandlesList.data[dst];
	srcImg = (const AMImage *)currentContext->handles->createdHandlesList.data[src];
	AM_ASSERT(dstImg);
	AM_ASSERT(srcImg);
#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (dstImg->inUseByEgl || srcImg->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgSeparableConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
#endif
	// check for illegal arguments
	if (kernelWidth <= 0 || kernelWidth > currentContext->maxSeparableKernelSize || kernelHeight <= 0 || kernelHeight > currentContext->maxSeparableKernelSize) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgSeparableConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (!kernelX || !kernelY || !amPointerIsAligned(kernelX, 2) || !amPointerIsAligned(kernelY, 2) || tilingMode < VG_TILE_FILL || tilingMode > VG_TILE_REFLECT) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgSeparableConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (amImagesOverlap(srcImg, dstImg)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgSeparableConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// handle NaN and Inf values
	scale = amNanInfFix(scale);
	bias = amNanInfFix(bias);

	if (!amFltSeparableConvolve(dstImg, srcImg, kernelWidth, kernelHeight,	shiftX, shiftY, kernelX, kernelY,
								1.0f, scale, bias, tilingMode, currentContext->tileFillColor,
								fltFormatLinear, fltFormatPremultiplied, currentContext->filterChannelMask, currentContext)) {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgSeparableConvolve");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
}
#else
	(void)dst;
	(void)src;
	(void)kernelWidth;
	(void)kernelHeight;
	(void)shiftX;
	(void)shiftY;
	(void)kernelX;
	(void)kernelY;
	(void)scale;
	(void)bias;
	(void)tilingMode;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgSeparableConvolve");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief This function computes the convolution of a source image with a separable kernel defined in each
	dimension by the Gaussian function.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dst destination image.
	\param src source image.
	\param stdDeviationX horizontal standard deviation.
	\param stdDeviationY vertical standard deviation.
	\param tilingMode tiling mode to use when reading pixels from source image.
*/
VG_API_CALL void VG_API_ENTRY vgGaussianBlur(VGImage dst,
                                             VGImage src,
                                             VGfloat stdDeviationX,
                                             VGfloat stdDeviationY,
                                             VGTilingMode tilingMode) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGaussianBlur");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

#if !defined(AM_LITE_PROFILE)
{
	AMImage *dstImg;
	const AMImage *srcImg;
	AMbool fltFormatLinear = (currentContext->filterFormatLinear == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMbool fltFormatPremultiplied = (currentContext->filterFormatPremultiplied == VG_TRUE) ? AM_TRUE : AM_FALSE;

	// check for bad handles
	if (amCtxHandleValid(currentContext, dst) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgGaussianBlur");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (amCtxHandleValid(currentContext, src) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgGaussianBlur");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	dstImg = (AMImage *)currentContext->handles->createdHandlesList.data[dst];
	srcImg = (const AMImage *)currentContext->handles->createdHandlesList.data[src];
	AM_ASSERT(dstImg);
	AM_ASSERT(srcImg);
#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (dstImg->inUseByEgl || srcImg->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgGaussianBlur");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
#endif

	// handle NaN and Inf values
	stdDeviationX = amNanInfFix(stdDeviationX);
	stdDeviationY = amNanInfFix(stdDeviationY);

	// check for illegal arguments
	if (stdDeviationX <= 0.0f || stdDeviationY <= 0.0f ||
		stdDeviationX > (VGfloat)currentContext->maxGaussianStdDeviation ||
		stdDeviationY > (VGfloat)currentContext->maxGaussianStdDeviation ||
		tilingMode < VG_TILE_FILL || tilingMode > VG_TILE_REFLECT) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgGaussianBlur");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (amImagesOverlap(srcImg, dstImg)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgGaussianBlur");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if (!amFltGaussianBlur(dstImg, srcImg,
						   AM_MIN(stdDeviationX, (AMfloat)currentContext->maxGaussianStdDeviation),
						   AM_MIN(stdDeviationY, (AMfloat)currentContext->maxGaussianStdDeviation),
						   tilingMode, currentContext->tileFillColor,
						   fltFormatLinear, fltFormatPremultiplied, currentContext->filterChannelMask, currentContext)) {

		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgGaussianBlur");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
}
#else
	(void)dst;
	(void)src;
	(void)stdDeviationX;
	(void)stdDeviationY;
	(void)tilingMode;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgGaussianBlur");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief This function passes each image channel of the source image through a separate lookup table.
	Each channel of the source pixel is used as an index into the lookup table for that channel.
	Each LUT parameter should contain 256 unsigned bytes entries.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dst destination image.
	\param src source image.
	\param redLUT red component lookup table.
	\param greenLUT green component lookup table.
	\param blueLUT blue component lookup table.
	\param alphaLUT alpha component lookup table.
	\param outputLinear VG_TRUE if lookup values are in linear color space, else VG_FALSE.
	\param outputPremultiplied VG_TRUE if lookup values are in premultiplied color space, else VG_FALSE.
*/
VG_API_CALL void VG_API_ENTRY vgLookup(VGImage dst,
                                       VGImage src,
                                       const VGubyte *redLUT,
                                       const VGubyte *greenLUT,
                                       const VGubyte *blueLUT,
                                       const VGubyte *alphaLUT,
                                       VGboolean outputLinear,
                                       VGboolean outputPremultiplied) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgLookup");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

#if !defined(AM_LITE_PROFILE)
{
	AMImage *dstImg;
	const AMImage *srcImg;
	AMbool fltFormatLinear = (currentContext->filterFormatLinear == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMbool fltFormatPremultiplied = (currentContext->filterFormatPremultiplied == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMbool fltOutputLinear = (outputLinear == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMbool fltOutputPremultiplied = (outputPremultiplied == VG_TRUE) ? AM_TRUE : AM_FALSE;

	// check for bad handles
	if (amCtxHandleValid(currentContext, dst) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgLookup");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (amCtxHandleValid(currentContext, src) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgLookup");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	dstImg = (AMImage *)currentContext->handles->createdHandlesList.data[dst];
	srcImg = (const AMImage *)currentContext->handles->createdHandlesList.data[src];
	AM_ASSERT(dstImg);
	AM_ASSERT(srcImg);
#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (dstImg->inUseByEgl || srcImg->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgLookup");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
#endif
	// check for illegal arguments
	if (!redLUT || !greenLUT || !blueLUT || !alphaLUT) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgLookup");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (amImagesOverlap(srcImg, dstImg)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgLookup");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	amFltLookup(dstImg, srcImg, redLUT, greenLUT, blueLUT, alphaLUT, fltOutputLinear, fltOutputPremultiplied,
				fltFormatLinear, fltFormatPremultiplied, currentContext->filterChannelMask, currentContext);
}
#else
	(void)dst;
	(void)src;
	(void)redLUT;
	(void)greenLUT;
	(void)blueLUT;
	(void)alphaLUT;
	(void)outputLinear;
	(void)outputPremultiplied;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgLookup");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief This function passes a single image channel of the source image, selected by the sourceChannel
	parameter, through a combined lookup table that produces whole pixel values.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dst destination image.
	\param src source image.
	\param lookupTable it contains 256 4-byte aligned entries in an	RGBA_8888 pixel format, which is
	interpreted as lRGBA_8888, lRGBA_8888_PRE, sRGBA_8888, or sRGBA_8888_PRE, depending on the
	values of outputLinear and outputPremultiplied.
	\param sourceChannel an index into the lookup table. If the source image is in a single-channel
	grayscale or alpha-only format, this parameter is ignored and the single channel is used.
	\param outputLinear VG_TRUE if lookup values are in linear color space, else VG_FALSE.
	\param outputPremultiplied VG_TRUE if lookup values are in premultiplied color space, else VG_FALSE.
*/
VG_API_CALL void VG_API_ENTRY vgLookupSingle(VGImage dst,
                                             VGImage src,
                                             const VGuint *lookupTable,
                                             VGImageChannel sourceChannel,
                                             VGboolean outputLinear,
                                             VGboolean outputPremultiplied) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgLookupSingle");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

#if !defined(AM_LITE_PROFILE)
{
	AMImage *dstImg;
	const AMImage *srcImg;
	AMbool fltFormatLinear = (currentContext->filterFormatLinear == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMbool fltFormatPremultiplied = (currentContext->filterFormatPremultiplied == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMbool fltOutputLinear = (outputLinear == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMbool fltOutputPremultiplied = (outputPremultiplied == VG_TRUE) ? AM_TRUE : AM_FALSE;

	if (!currentContext || !currentContext->initialized) {
		AM_MEMORY_LOG("vgLookupSingle");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handles
	if (amCtxHandleValid(currentContext, dst) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgLookupSingle");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (amCtxHandleValid(currentContext, src) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgLookupSingle");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	dstImg = (AMImage *)currentContext->handles->createdHandlesList.data[dst];
	srcImg = (const AMImage *)currentContext->handles->createdHandlesList.data[src];
	AM_ASSERT(dstImg);
	AM_ASSERT(srcImg);
#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (dstImg->inUseByEgl || srcImg->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgLookupSingle");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
#endif
	// check for illegal arguments
	if (srcImg->format != VG_sL_8 && srcImg->format != VG_lL_8 && srcImg->format != VG_A_8 && srcImg->format != VG_BW_1) {
		if (sourceChannel != VG_RED && sourceChannel != VG_GREEN && sourceChannel != VG_BLUE && sourceChannel != VG_ALPHA) {
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			AM_MEMORY_LOG("vgLookupSingle");
			OPENVG_RETURN(OPENVG_NO_RETVAL)
		}
	}
	if (!lookupTable || !amPointerIsAligned(lookupTable, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgLookupSingle");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (amImagesOverlap(srcImg, dstImg)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgLookupSingle");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	amFltLookupSingle(dstImg, srcImg, lookupTable, sourceChannel,
					  fltOutputLinear, fltOutputPremultiplied, fltFormatLinear, fltFormatPremultiplied,
					  currentContext->filterChannelMask, currentContext);
}
#else
	(void)dst;
	(void)src;
	(void)lookupTable;
	(void)sourceChannel;
	(void)outputLinear;
	(void)outputPremultiplied;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgLookupSingle");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

