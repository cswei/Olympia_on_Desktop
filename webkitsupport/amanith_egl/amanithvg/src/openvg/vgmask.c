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
	\file vgmask.c
	\brief Masking and clearing, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#if defined(AM_SRE)
	#include "sremask.h"
#elif defined(AM_GLE)
	#include "glemask.h"
#elif defined(AM_GLS)
	#include "glsmask.h"
#else
	#error Unreachable point.
#endif
#include "vgprimitives.h"
#include "vgpaint.h"
#include "vgmask.h"
#include "vgmatrix.h"
#include "vgconversions.h"

#if defined RIM_VG_SRC
#define VG_API_ENTRY 
#endif


// *********************************************************************
//                        Private implementations
// *********************************************************************

/*!
	\brief Get the alpha value of a given 32bit or 16bit pixel.
	\param srcPixel the source pixel.
	\param fmtTableIndex index corresponding to the format of the source pixel.
	\return the alpha value.
*/
AMuint32 amAlphaGet(const AMuint32 srcPixel,
					const AMuint32 fmtTableIndex) {

	AMuint32 aShift, aBits;
	AMuint32 srcBits = pxlFormatTable[fmtTableIndex][FMT_BITS];
	AMuint32 srcFlags = pxlFormatTable[fmtTableIndex][FMT_FLAGS];

	AM_ASSERT(srcBits == 32 || srcBits == 16);

	if (srcFlags & FMT_ALPHA) {
		aShift = pxlFormatTable[fmtTableIndex][FMT_A_SH];
		aBits = pxlFormatTable[fmtTableIndex][FMT_A_BITS];
	}
	else {
		// get the red component, according to OpenVG specifications
		aShift = pxlFormatTable[fmtTableIndex][FMT_R_SH];
		aBits = pxlFormatTable[fmtTableIndex][FMT_R_BITS];
	}

	if (srcBits == 32)
		return ((srcPixel >> aShift) & 0xFF);
	else {
		AMuint32 aMask = (1 << aBits) - 1;

		AM_ASSERT(aBits == 1 || aBits == 4 || aBits == 5);
		if (aBits == 1)
			return (((srcPixel >> aShift) & aMask) * 255);
		else
		if (aBits == 4)
			return (((srcPixel >> aShift) & aMask) * 17);
		else
			return (((srcPixel >> aShift) & aMask) * 2106) >> 8;
	}
}

/*!
	\brief Update the given alpha mask values, according to a mask operation.
	\param surface drawing surface containing the alpha mask to update.
	\param srcImage input source image containing alpha values.
	\param operation mask operation.
	\param x x-coordinate of the first pixel to update.
	\param y y-coordinate of the first pixel to update.
	\param width width of the rectangular region to update, in pixels.
	\param height height of the rectangular region to update, in pixels.
	\pre width, height > 0
	\pre alpha mask format must be VG_A_8.
*/
void amMaskDoUpdate(AMDrawingSurface *surface,
					const AMImage *srcImage,
					const VGMaskOperation operation,
					AMint32 x,
					AMint32 y,
					AMint32 width,
					AMint32 height) {

	AMint32 i, j, sx, sy;
	AMuint32 *src32, dstAlpha32, srcAlpha32;
	AMuint8 *dst8, *src8;
	AMuint16 *src16;
	AMbool fullUpdate;
	AMuint32 srcIndex = 0, srcBits = 0;

	AM_ASSERT(surface);
	AM_ASSERT(surface->alphaMaskPixels);
	AM_ASSERT(width > 0 && height > 0);

	sx = 0;
	sy = 0;

	// clip specified bound
	if (x < 0) {
		width += x;
		if (width <= 0)
			return;
		sx -= x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		if (height <= 0)
			return;
		sy -= y;
		y = 0;
	}
	if (x + width > amSrfWidthGet(surface)) {
		width = amSrfWidthGet(surface) - x;
		if (width <= 0)
			return;
	}
	if (y + height > amSrfHeightGet(surface)) {
		height = amSrfHeightGet(surface) - y;
		if (height <= 0)
			return;
	}
	// now clamp source region
	AM_ASSERT(sx >= 0 && sy >= 0);

	if (srcImage) {
		if (sx + width > srcImage->width) {
			width = srcImage->width - sx;
			if (width <= 0)
				return;
		}
		if (sy + height > srcImage->height) {
			height = srcImage->height - sy;
			if (height <= 0)
				return;
		}
		AM_ASSERT(sx >= 0 && sx < srcImage->width);
		AM_ASSERT(sy >= 0 && sy < srcImage->height);
		// take into account child images
		sx += srcImage->x;
		sy += srcImage->y;
	}

	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT(x >= 0 && x < amSrfWidthGet(surface));
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));

	fullUpdate = (x == 0 && y == 0 && width == amSrfWidthGet(surface) && height == amSrfHeightGet(surface));

	if (operation != VG_CLEAR_MASK && operation != VG_FILL_MASK) {
		AM_ASSERT(srcImage);
		AM_ASSERT((AMint32)srcImage->format != (AMint32)VG_IMAGE_FORMAT_INVALID);
		// get format index (in the format table)
		srcIndex = AM_FMT_GET_INDEX(srcImage->format);
		srcBits = pxlFormatTable[srcIndex][FMT_BITS];
	}

	// alpha mask is kept internally in 8bit, with values going from 0 to 255
	dst8 = surface->alphaMaskPixels;
	// invert y to match OpenVG coordinate system (so in the scanline fillers we can access the buffer linearly)
	y = amSrfHeightGet(surface) - y - 1;
	dst8 += y * amSrfWidthGet(surface);

	switch (operation) {
		// clear
		case VG_CLEAR_MASK:
			if (fullUpdate)
				amMemset(surface->alphaMaskPixels, 0, width * height);
			else {
			    for (i = 0; i < height; ++i) {
			 		amMemset(&dst8[x], 0, width);
				    dst8 -= amSrfWidthGet(surface);
			    }
			}
			break;
		// fill
		case VG_FILL_MASK:
			if (fullUpdate)
				amMemset(surface->alphaMaskPixels, 0xFF, width * height);
			else {
			    for (i = 0; i < height; ++i) {
					amMemset(&dst8[x], 0xFF, width);
				    dst8 -= amSrfWidthGet(surface);
			    }
			}
			break;
		// set
		case VG_SET_MASK:
			switch (srcBits) {
				case 32:
					src32 = (AMuint32 *)srcImage->pixels;
					src32 += sy * srcImage->width;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = amAlphaGet(src32[j + sx], srcIndex);
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src32 += srcImage->width;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
				case 16:
					src16 = (AMuint16 *)srcImage->pixels;
					src16 += sy * srcImage->width;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = amAlphaGet(src16[j + sx], srcIndex);
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src16 += srcImage->width;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
				case 8:
					src8 = srcImage->pixels;
					src8 += sy * srcImage->width;
					for (i = 0; i < height; ++i) {
						amMemcpy(&dst8[x], &src8[sx], width);
						src8 += srcImage->width;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
			#if (AM_OPENVG_VERSION >= 110)
				case 4:
					src8 = srcImage->pixels;
					src8 += sy * srcImage->dataStride;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							AMint32 u = j + sx;
							srcAlpha32 = src8[u >> 1];
							srcAlpha32 = ((srcAlpha32 >> ((u & 1) << 2)) & 0x0F) * 17;
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src8 += srcImage->dataStride;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
			#endif
				case 1:
					src8 = srcImage->pixels;
					src8 += sy * srcImage->dataStride;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = (AMuint32)(src8[(j + sx) >> 3]);
							srcAlpha32 = (srcAlpha32 >> ((j + sx) & 0x07)) & 0x01;
							srcAlpha32 = srcAlpha32 * 255;
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src8 += srcImage->dataStride;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
				default:
					AM_ASSERT(0 == 1);
					break;
			}
			break;

		// union
		case VG_UNION_MASK:
			switch (srcBits) {
				case 32:
					src32 = (AMuint32 *)srcImage->pixels;
					src32 += sy * srcImage->width;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = amAlphaGet(src32[j + sx], srcIndex);
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, 255 - srcAlpha32, 255 - dstAlpha32)
							srcAlpha32 = 255 - srcAlpha32;
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src32 += srcImage->width;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
				case 16:
					src16 = (AMuint16 *)srcImage->pixels;
					src16 += sy * srcImage->width;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = amAlphaGet(src16[j + sx], srcIndex);
							AM_ASSERT(srcAlpha32 <= 255);
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, 255 - srcAlpha32, 255 - dstAlpha32)
							srcAlpha32 = 255 - srcAlpha32;
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src16 += srcImage->width;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
				case 8:
					src8 = srcImage->pixels;
					src8 += sy * srcImage->width;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = (AMuint32)(src8[j + sx]);
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, 255 - srcAlpha32, 255 - dstAlpha32)
							srcAlpha32 = 255 - srcAlpha32;
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src8 += srcImage->width;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
			#if (AM_OPENVG_VERSION >= 110)
				case 4:
					src8 = srcImage->pixels;
					src8 += sy * srcImage->dataStride;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							AMint32 u = j + sx;
							srcAlpha32 = src8[u >> 1];
							srcAlpha32 = ((srcAlpha32 >> ((u & 1) << 2)) & 0x0F) * 17;
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, 255 - srcAlpha32, 255 - dstAlpha32)
							srcAlpha32 = 255 - srcAlpha32;
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src8 += srcImage->dataStride;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
			#endif
				case 1:
					src8 = srcImage->pixels;
					src8 += sy * srcImage->dataStride;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = (AMuint32)(src8[(j + sx) >> 3]);
							srcAlpha32 = (srcAlpha32 >> ((j + sx) & 0x07)) & 0x01;
							srcAlpha32 = srcAlpha32 * 255;
							AM_ASSERT(srcAlpha32 <= 255);
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, 255 - srcAlpha32, 255 - dstAlpha32)
							srcAlpha32 = 255 - srcAlpha32;
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src8 += srcImage->dataStride;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
				default:
					AM_ASSERT(0 == 1);
					break;
			}
			break;

		// intersect
		case VG_INTERSECT_MASK:
			switch (srcBits) {
				case 32:
					src32 = (AMuint32 *)srcImage->pixels;
					src32 += sy * srcImage->width;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = amAlphaGet(src32[j + sx], srcIndex);
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, srcAlpha32, dstAlpha32)
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src32 += srcImage->width;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
				case 16:
					src16 = (AMuint16 *)srcImage->pixels;
					src16 += sy * srcImage->width;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = amAlphaGet(src16[j + sx], srcIndex);
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, srcAlpha32, dstAlpha32)
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src16 += srcImage->width;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
				case 8:
					src8 = srcImage->pixels;
					src8 += sy * srcImage->width;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = (AMuint32)(src8[j + sx]);
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, srcAlpha32, dstAlpha32)
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src8 += srcImage->width;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
			#if (AM_OPENVG_VERSION >= 110)
				case 4:
					src8 = srcImage->pixels;
					src8 += sy * srcImage->dataStride;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							AMint32 u = j + sx;
							srcAlpha32 = src8[u >> 1];
							srcAlpha32 = ((srcAlpha32 >> ((u & 1) << 2)) & 0x0F) * 17;
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, srcAlpha32, dstAlpha32)
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src8 += srcImage->dataStride;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
			#endif
				case 1:
					src8 = srcImage->pixels;
					src8 += sy * srcImage->dataStride;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = (AMuint32)(src8[(j + sx) >> 3]);
							srcAlpha32 = (srcAlpha32 >> ((j + sx) & 0x07)) & 0x01;
							srcAlpha32 = srcAlpha32 * 255;
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, srcAlpha32, dstAlpha32)
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src8 += srcImage->dataStride;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
				default:
					AM_ASSERT(0 == 1);
					break;
			}
			break;

		// subtract
		case VG_SUBTRACT_MASK:
			switch (srcBits) {
				case 32:
					src32 = (AMuint32 *)srcImage->pixels;
					src32 += sy * srcImage->width;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = amAlphaGet(src32[j + sx], srcIndex);
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, 255 - srcAlpha32, dstAlpha32)
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src32 += srcImage->width;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
				case 16:
					src16 = (AMuint16 *)srcImage->pixels;
					src16 += sy * srcImage->width;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = amAlphaGet(src16[j + sx], srcIndex);
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, 255 - srcAlpha32, dstAlpha32)
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src16 += srcImage->width;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
				case 8:
					src8 = srcImage->pixels;
					src8 += sy * srcImage->width;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = (AMuint32)(src8[j + sx]);
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, 255 - srcAlpha32, dstAlpha32)
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src8 += srcImage->width;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
			#if (AM_OPENVG_VERSION >= 110)
				case 4:
					src8 = srcImage->pixels;
					src8 += sy * srcImage->dataStride;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							AMint32 u = j + sx;
							srcAlpha32 = src8[u >> 1];
							srcAlpha32 = ((srcAlpha32 >> ((u & 1) << 2)) & 0x0F) * 17;
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, 255 - srcAlpha32, dstAlpha32)
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src8 += srcImage->dataStride;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
			#endif
				case 1:
					src8 = srcImage->pixels;
					src8 += sy * srcImage->dataStride;
					for (i = 0; i < height; ++i) {
						for (j = 0; j < width; ++j) {
							srcAlpha32 = (AMuint32)(src8[(j + sx) >> 3]);
							srcAlpha32 = (srcAlpha32 >> ((j + sx) & 0x07)) & 0x01;
							srcAlpha32 = srcAlpha32 * 255;
							dstAlpha32 = dst8[j + x];
							MULT_DIV_255(srcAlpha32, 255 - srcAlpha32, dstAlpha32)
							AM_ASSERT(srcAlpha32 <= 255);
							dst8[j + x] = (AMuint8)srcAlpha32;
						}
						src8 += srcImage->dataStride;
						dst8 -= amSrfWidthGet(surface);
					}
					break;
				default:
					AM_ASSERT(0 == 1);
					break;
			}
			break;
		default:
			AM_ASSERT(0 == 1);
			break;
	}
}

/*!
	\brief Update the drawing surface mask values, according to a mask operation. If alpha mask has not been
	previously allocate, this function creates it.
	\param surface input drawing surface containing the alpha mask to update.
	\param srcImage input source image containing alpha values.
	\param operation mask operation.
	\param x x-coordinate of the first pixel to update.
	\param y y-coordinate of the first pixel to update.
	\param width width of the rectangular region to update, in pixels.
	\param height height of the rectangular region to update, in pixels.
*/
void amMaskUpdate(AMDrawingSurface *surface,
					const AMImage *srcImage,
					const VGMaskOperation operation,
					const AMint32 x,
					const AMint32 y,
					const AMint32 width,
					const AMint32 height) {

	AM_ASSERT(surface);

	// re-extract the alphaMask from the surface (now it has been created by previous call, if necessary)
	if (surface->alphaMaskPixels) {
		amMaskDoUpdate(surface, srcImage, operation, x, y, width, height);
	#if defined(AM_GLE) || defined(AM_GLS)
		// invaidate alpha mask textures
		surface->alphaMaskTextureValid = AM_FALSE;
		surface->invAlphaMaskTextureValid = AM_FALSE;
	#endif
	}
}

/*!
	\brief It fills the portion of the drawing surface intersecting the specified rectangle with a constant
	color value, taken from the VG_CLEAR_COLOR parameter. The color value is expressed in non-premultiplied
	sRGBA (sRGB color plus alpha) format. Values outside the [0, 1] range are interpreted as the nearest
	endpoint of the range. The color is converted to the destination color space in the same manner as if
	a rectangular path were being filled. Clipping and scissoring take place in the usual fashion, but
	antialiasing, masking, and blending do not occur.
	\param context context containing the clearing color.
	\param surface drawing surface to be cleared.
	\param x x-coordinate of the first pixel to clear.
	\param y y-coordinate of the first pixel to clear.
	\param width width of the rectangular region to clear, in pixels.
	\param height height of the rectangular region to clear, in pixels.
*/
AMbool amDrawingSurfaceClear(AMContext *context,
							 AMDrawingSurface *surface,
							 AMint32 x,
							 AMint32 y,
							 AMint32 width,
							 AMint32 height) {

#if defined(AM_SRE)
	return amSreDrawingSurfaceClear(context, surface, x, y, width, height);
#elif defined(AM_GLE)
	return amGleDrawingSurfaceClear(context, surface, x, y, width, height);
#elif defined(AM_GLS)
	return amGlsDrawingSurfaceClear(context, surface, x, y, width, height);
#else
	#error Unreachable point.
#endif
}

#if defined RIM_VG_SRC
void amInternalDrawingSurfaceClear(AMDrawingSurface *surface,
						           AMint32 x,
						           AMint32 y,
						           AMint32 width,
						           AMint32 height) {

#if defined(AM_SRE)
	amSreInternalDrawingSurfaceClear(surface, x, y, width, height);
#elif defined(AM_GLE)
	amGleInternalDrawingSurfaceClear(surface, x, y, width, height);
#elif defined(AM_GLS)
	amGlsInternalDrawingSurfaceClear(surface, x, y, width, height);
#else
	#error Unreachable point.
#endif
}
#endif

#if (AM_OPENVG_VERSION >= 110)

/*!
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
*/

void amRenderToMaskClearSetup(AMContext *context,
							  AMDrawingSurface *surface) {

	AMint32 y;
	AMuint32 j;
	AMuint8 *alphaMask = surface->alphaMaskPixels;
	AMScissorRectDynArray *splitScissorRects = &context->splitScissorRects;

	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(surface->alphaMaskPixels);
	AM_ASSERT(context->scissoring == VG_TRUE);
	AM_ASSERT(context->splitScissorRects.size > 0);

	for (y = amSrfHeightGet(surface) - 1; y >= 0; --y) {

		AMuint32 width;
		AMuint32 x = 0;

		for (j = 0; j < splitScissorRects->size; ++j) {
			// check if the current y scanline intersect the i-thm scissor rectangle
			if (y >= splitScissorRects->data[j].bottomLeft.p.y && y < splitScissorRects->data[j].topRight.p.y) {
				// if the scissor rectangle intersects the scanline, clear unaffected pixels
				AM_ASSERT(x <= (AMuint32)amSrfWidthGet(surface));
				width = splitScissorRects->data[j].bottomLeft.p.x - x;
				if (width > 0)
					amMemset(&alphaMask[x], 0, width);
				// move the cursor at the end of the scissor rectangle
				x = splitScissorRects->data[j].topRight.p.x;
			}
		}

		// clear the remaining pixels on this scanline
		AM_ASSERT(x <= (AMuint32)amSrfWidthGet(surface));
		width = (AMuint32)amSrfWidthGet(surface) - x;
		if (width > 0)
			amMemset(&alphaMask[x], 0, width);
		// next scanline inside the alphamask
		alphaMask += amSrfWidthGet(surface);
	}
}

AMbool amRenderToMask(AMContext *context,
					  AMDrawingSurface *surface,
					  AMPath *path,
					  const VGbitfield paintModes,
					  const VGMaskOperation operation) {

	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(path);

	context->beforePathRasterization = AM_TRUE;

	if (surface->alphaMaskPixels) {
		// optimize CLEAR operation using memset instead of amMaskUpdate
		if (operation == VG_CLEAR_MASK)
			amMemset(surface->alphaMaskPixels, 0, amSrfWidthGet(surface) * amSrfHeightGet(surface));
		else
		// optimize FILL operation using memset instead of amMaskUpdate
		if (operation == VG_FILL_MASK)
			amMemset(surface->alphaMaskPixels, 0xFF, amSrfWidthGet(surface) * amSrfHeightGet(surface));
		else {
			AMuint32 slotIndex;
		#if defined(AM_FIXED_POINT_PIPELINE)
			AMAABox2x srfSpaceStrokeBox;
			AMVect2x p0, p1;
		#else
			AMAABox2f srfSpaceStrokeBox;
			AMVect2f p0, p1;
		#endif
			AMAABox2f objSpaceStrokeBox;
			AMAABox2i srfSpaceBox;
			AMint32 srfSpaceBoxWidth, srfSpaceBoxHeight;
			AMfloat tmpFloat;
			AMbool objSpaceBoxZeroDimension, doneFlattening, pathNeedClipping;
			AMUserToSurfaceDesc userToSurfaceDesc;

			// if the path has no segments, just exit
			if (path->segments.size == 0)
				return AM_TRUE;
			// update matrices and scale factors
			amMatricesUpdate(context);
			// if path matrix is singular, path won't be drawn
			if (context->pathUserToSurfaceFlags & AM_MATRIX_SINGULAR)
				return AM_TRUE;

			userToSurfaceDesc.userToSurface = &context->pathUserToSurface;
		#if defined(AM_FIXED_POINT_PIPELINE)
			amRasMatrixFToX(&userToSurfaceDesc.userToSurfacex, userToSurfaceDesc.userToSurface);
		#endif
			userToSurfaceDesc.inverseUserToSurface = &context->inversePathUserToSurface;
			userToSurfaceDesc.userToSurfaceScale = context->pathUserToSurfaceScale;
			userToSurfaceDesc.flags = context->pathUserToSurfaceFlags;
			userToSurfaceDesc.userToSurfaceAffine = AM_TRUE;

			// extract path bound
			amPathBounds(&tmpFloat, &tmpFloat, &tmpFloat, &tmpFloat, path, context);
			// initialize object space path bounding box
			objSpaceStrokeBox = path->box;
			// if the box has a zero dimension, skip the drawing
			objSpaceBoxZeroDimension = ((AM_AABOX2_WIDTH(&objSpaceStrokeBox) <= AM_EPSILON_FLOAT) || (AM_AABOX2_HEIGHT(&objSpaceStrokeBox) <= AM_EPSILON_FLOAT)) ? AM_TRUE : AM_FALSE;
			// if stroke is enabled we have to enlarge bounding box with stroke thickness
			if (paintModes & VG_STROKE_PATH) {
				// for square cap, we have to adjust expanding factor by sqrt(2)
				AMfloat adjSquareCap = (context->startCapStyle == VG_CAP_SQUARE) ? 1.4142136f : 1.0f;
				// if join style is miter, we have to add the maximum permitted miter join distance
				AMfloat expansion = (context->joinStyle == VG_JOIN_MITER) ? context->miterMulThickness * adjSquareCap : context->strokeLineThickness * adjSquareCap;

				objSpaceStrokeBox.minPoint.x -= expansion;
				objSpaceStrokeBox.minPoint.y -= expansion;
				objSpaceStrokeBox.maxPoint.x += expansion;
				objSpaceStrokeBox.maxPoint.y += expansion;
			}
		#if defined(AM_FIXED_POINT_PIPELINE)
			// generate drawing surface space bounding box, according to "path user to surface" matrix
			amAABox2fTransformx(&srfSpaceStrokeBox, &objSpaceStrokeBox, &userToSurfaceDesc.userToSurfacex);
			// align drawing surface space box to its nearest integer pixel
			AM_VECT2_SET(&p0, (srfSpaceStrokeBox.minPoint.x - AM_RAS_FIXED_MASK - AM_RAS_FIXED_ONE) >> AM_RAS_FIXED_PRECISION, (srfSpaceStrokeBox.minPoint.y - AM_RAS_FIXED_MASK - AM_RAS_FIXED_ONE) >> AM_RAS_FIXED_PRECISION)
			AM_VECT2_SET(&p1, (srfSpaceStrokeBox.maxPoint.x + AM_RAS_FIXED_MASK + AM_RAS_FIXED_ONE) >> AM_RAS_FIXED_PRECISION, (srfSpaceStrokeBox.maxPoint.y + AM_RAS_FIXED_MASK + AM_RAS_FIXED_ONE) >> AM_RAS_FIXED_PRECISION)
			AM_SRFSPACE_CORNER_CLAMPX(srfSpaceBox.minPoint, p0)
			AM_SRFSPACE_CORNER_CLAMPX(srfSpaceBox.maxPoint, p1)
		#else
			// generate drawing surface space bounding box, according to "path user to surface" matrix
			amAABox2fTransform(&srfSpaceStrokeBox, &objSpaceStrokeBox, userToSurfaceDesc.userToSurface);
			// align drawing surface space box to its nearest integer pixel
			AM_VECT2_SET(&p0, amFloorf(srfSpaceStrokeBox.minPoint.x - 1.0f), amFloorf(srfSpaceStrokeBox.minPoint.y - 1.0f))
			AM_VECT2_SET(&p1, amCeilf(srfSpaceStrokeBox.maxPoint.x + 1.0f), amCeilf(srfSpaceStrokeBox.maxPoint.y + 1.0f))
			AM_SRFSPACE_CORNER_CLAMP(srfSpaceBox.minPoint, p0)
			AM_SRFSPACE_CORNER_CLAMP(srfSpaceBox.maxPoint, p1)
		#endif

			pathNeedClipping = AM_FALSE;
			// convert srfSpaceBox into integer values (good for possible grab operations)
			srfSpaceBoxWidth = AM_AABOX2_WIDTH(&srfSpaceBox);
			srfSpaceBoxHeight = AM_AABOX2_HEIGHT(&srfSpaceBox);
			// clamp drawing surface space box to viewport
			if (srfSpaceBox.minPoint.x < 0) {
				srfSpaceBoxWidth += srfSpaceBox.minPoint.x;
				srfSpaceBox.minPoint.x = 0;
				pathNeedClipping = AM_TRUE;
			}
			if (srfSpaceBox.minPoint.y < 0) {
				srfSpaceBoxHeight += srfSpaceBox.minPoint.y;
				srfSpaceBox.minPoint.y = 0;
				pathNeedClipping = AM_TRUE;
			}
			if (srfSpaceBox.minPoint.x + srfSpaceBoxWidth > amSrfWidthGet(surface)) {
				srfSpaceBoxWidth = amSrfWidthGet(surface) - srfSpaceBox.minPoint.x;
				if (srfSpaceBoxWidth <= 0)
					return AM_TRUE;
				pathNeedClipping = AM_TRUE;
			}
			if (srfSpaceBox.minPoint.y + srfSpaceBoxHeight > amSrfHeightGet(surface)) {
				srfSpaceBoxHeight = amSrfHeightGet(surface) - srfSpaceBox.minPoint.y;
				if (srfSpaceBoxHeight <= 0)
					return AM_TRUE;
				pathNeedClipping = AM_TRUE;
			}
			// reject the whole draw operation if surface space box lies outside the viewport
			if (srfSpaceBox.minPoint.x >= amSrfWidthGet(surface) || srfSpaceBox.minPoint.y >= amSrfHeightGet(surface) || srfSpaceBoxWidth <= 0 || srfSpaceBoxHeight <= 0)
				return AM_TRUE;
			srfSpaceBox.maxPoint.x = srfSpaceBox.minPoint.x + srfSpaceBoxWidth;
			srfSpaceBox.maxPoint.y = srfSpaceBox.minPoint.y + srfSpaceBoxHeight;

			// calculate path deviation
			amPathDeviationUpdate(context, path, paintModes, &srfSpaceBox, &userToSurfaceDesc);

			// extract the slot for flattening, so we are sure that for drawing operations the correct slot will be used
			if (!amPathFlatten(&slotIndex, path, context, &doneFlattening))
				return AM_FALSE;

			if (context->scissoring == VG_TRUE) {
				// update scissor rectangle decomposition, if needed
				if (context->scissorRectsModified) {
					if (!amScissorRectsDecompose(context, surface))
						return AM_FALSE;
				}
				// if there aren't scissor rectangles, and scissoring is enabled, do not draw anything
				if (context->splitScissorRects.size < 1)
					return AM_TRUE;
			}

			if ((paintModes & VG_FILL_PATH) && !objSpaceBoxZeroDimension) {

				AMPaintDesc paintDesc;

				context->beforePathRasterization = AM_FALSE;
				// rasterize fill polygons
				paintDesc.image = NULL;
				// radial gradient is just used because color paint type would result in rasterizer fast pipelines (not safe in this case)
				paintDesc.paintType = VG_PAINT_TYPE_RADIAL_GRADIENT;
				paintDesc.blendMode = VG_BLEND_SRC;
				paintDesc.masking = AM_FALSE;
				paintDesc.fillRule = context->fillRule;
				paintDesc.userToSurfaceDesc = &userToSurfaceDesc;

				if (context->scissoring == VG_TRUE) {

					AMuint32 i;

					if (operation == VG_SET_MASK || operation == VG_INTERSECT_MASK)
						// clear alpha mask pixels not included in scissor rectangles
						amRenderToMaskClearSetup(context, surface);

					for (i = 0; i < context->splitScissorRects.size; ++i) {
						// extract and set the i-th clipping rectangle
						AMAABox2i clipBox;
						AMint32 x0 = context->splitScissorRects.data[i].bottomLeft.p.x;
						AMint32 y0 = context->splitScissorRects.data[i].bottomLeft.p.y;
						AMint32 x1 = context->splitScissorRects.data[i].topRight.p.x;
						AMint32 y1 = context->splitScissorRects.data[i].topRight.p.y;

						AM_VECT2_SET(&clipBox.minPoint, x0, y0)
						AM_VECT2_SET(&clipBox.maxPoint, x1, y1)

						// if the clipBox (i.e. the scissor rectangle) contains the polygon, clipping is not needed
						if (amAABox2iContain(&clipBox, &srfSpaceBox)) {
							if (!amRasPolygonsMaskDraw(context, surface, context->rasterizer,
										#if defined(AM_FIXED_POINT_PIPELINE)
											&path->cache[slotIndex].flattenPtsx,
										#else
											&path->cache[slotIndex].flattenPts,
										#endif
											&path->cache[slotIndex].ptsPerContour, &paintDesc, operation, &clipBox, pathNeedClipping))
								return AM_FALSE;
						}
						else
						if (amAABox2iOverlap(&clipBox, &srfSpaceBox)) {
							if (!amRasPolygonsMaskDraw(context, surface, context->rasterizer,
													#if defined(AM_FIXED_POINT_PIPELINE)
														&path->cache[slotIndex].flattenPtsx,
													#else
														&path->cache[slotIndex].flattenPts,
													#endif
														&path->cache[slotIndex].ptsPerContour, &paintDesc, operation, &clipBox, AM_TRUE))
								return AM_FALSE;
						}
					}
				}
				else {
					AMAABox2i clipBox;

					// set a clipBox equal to the whole screen
					AM_VECT2_SET(&clipBox.minPoint, 0, 0)
					AM_VECT2_SET(&clipBox.maxPoint, amSrfWidthGet(surface), amSrfHeightGet(surface))

				if (!amRasPolygonsMaskDraw(context, surface, context->rasterizer,
										#if defined(AM_FIXED_POINT_PIPELINE)
											&path->cache[slotIndex].flattenPtsx,
										#else
											&path->cache[slotIndex].flattenPts,
										#endif
											&path->cache[slotIndex].ptsPerContour, &paintDesc, operation, &clipBox, pathNeedClipping))
					return AM_FALSE;
			}
			}

			if (paintModes & VG_STROKE_PATH && context->strokeLineWidth > 0.0f) {

				AMPaintDesc paintDesc;
				// update the coefficient used to know how many points are generated by a round cap/join
				AMfloat devOverRadius = AM_MAX(context->flattenParams.flatness / context->strokeLineThickness, 1e-4f);
				AMfloat	roundJoinAuxCoef = (devOverRadius >= 2.0f) ? 1.0f / AM_2PI : 1.0f / (2.0f * amAcosf(1.0f - devOverRadius));

				if (!amStrokeGenerate(context, &path->cache[slotIndex], roundJoinAuxCoef))
					return AM_FALSE;

				context->beforePathRasterization = AM_FALSE;
				// rasterize stroke polygons
				paintDesc.image = NULL;
				// radial gradient is just used because color paint type would result in rasterizer fast pipelines (not safe in this case)
				paintDesc.paintType = VG_PAINT_TYPE_RADIAL_GRADIENT;
				paintDesc.blendMode = VG_BLEND_SRC;
				paintDesc.masking = AM_FALSE;
				paintDesc.fillRule = VG_NON_ZERO;
				paintDesc.userToSurfaceDesc = &userToSurfaceDesc;

				if (context->scissoring == VG_TRUE) {

					AMuint32 i;

					if (operation == VG_SET_MASK || operation == VG_INTERSECT_MASK)
						// clear alpha mask pixels not included in scissor rectangles
						amRenderToMaskClearSetup(context, surface);

					for (i = 0; i < context->splitScissorRects.size; ++i) {
						// extract and set the i-th clipping rectangle
						AMAABox2i clipBox;
						AMint32 x0 = context->splitScissorRects.data[i].bottomLeft.p.x;
						AMint32 y0 = context->splitScissorRects.data[i].bottomLeft.p.y;
						AMint32 x1 = context->splitScissorRects.data[i].topRight.p.x;
						AMint32 y1 = context->splitScissorRects.data[i].topRight.p.y;

						AM_VECT2_SET(&clipBox.minPoint, x0, y0)
						AM_VECT2_SET(&clipBox.maxPoint, x1, y1)

						// if the clipBox (i.e. the scissor rectangle) contains the polygon, clipping is not needed
						if (amAABox2iContain(&clipBox, &srfSpaceBox)) {
				if (!amRasPolygonsMaskDraw(context, surface, context->rasterizer, &context->strokeAuxPts, &context->strokeAuxPtsPerContour, &paintDesc, operation, &clipBox, pathNeedClipping))
					return AM_FALSE;
			}
						else
						if (amAABox2iOverlap(&clipBox, &srfSpaceBox)) {
							if (!amRasPolygonsMaskDraw(context, surface, context->rasterizer, &context->strokeAuxPts, &context->strokeAuxPtsPerContour, &paintDesc, operation, &clipBox, AM_TRUE))
								return AM_FALSE;
						}
					}
				}
				else {
					AMAABox2i clipBox;

					// set a clipBox equal to the whole screen
					AM_VECT2_SET(&clipBox.minPoint, 0, 0)
					AM_VECT2_SET(&clipBox.maxPoint, amSrfWidthGet(surface), amSrfHeightGet(surface))

					if (!amRasPolygonsMaskDraw(context, surface, context->rasterizer, &context->strokeAuxPts, &context->strokeAuxPtsPerContour, &paintDesc, operation, &clipBox, pathNeedClipping))
						return AM_FALSE;
				}
			}
		}

	#if defined(AM_GLE) || defined(AM_GLS)
		// invaidate alpha mask textures
		surface->alphaMaskTextureValid = AM_FALSE;
		surface->invAlphaMaskTextureValid = AM_FALSE;
	#endif
		return AM_TRUE;
	}
	else
		return AM_TRUE;
}

AMbool amMaskLayerInit(AMImage *layer,
					   const AMint32 width,
					   const AMint32 height,
					   AMContext *context) {

	AM_ASSERT(layer);
	AM_ASSERT(width > 0 && height > 0);

	if (!amImageInit(layer, VG_A_8, VG_IMAGE_QUALITY_NONANTIALIASED, 0, 0, width, height, 0, context))
		return AM_FALSE;

	amMemset(layer->pixels, 0xFF, width * height);
	layer->id = AM_LAYER_HANDLE_ID;
	layer->type = AM_LAYER_HANDLE_ID;
	return AM_TRUE;
}

void amMaskLayerDestroy(AMImage *layer,
						AMContext *context) {

	amImageDestroy(layer, context);
}

void amCopyMask(AMContext *context,
				  AMDrawingSurface *surface,
				  AMImage *layer,
				  AMint32 dx,
				  AMint32 dy,
				  AMint32 sx,
				  AMint32 sy,
				  AMint32 width,
				  AMint32 height) {

	AM_ASSERT(surface);
	AM_ASSERT(layer && layer->id == AM_LAYER_HANDLE_ID && layer->format == VG_A_8);
	AM_ASSERT(width > 0 && height > 0);

#if defined(AM_SRE)
	(void)context;
#endif

	// re-extract the alphaMask from the context (now it has been created by previous call, if necessary)
	if (surface->alphaMaskPixels) {

		// clip specified bound
		if (sx < 0) {
			width += sx;
			if (width <= 0)
				return;
			dx -= sx;
			sx = 0;
		}
		if (sy < 0) {
			height += sy;
			if (height <= 0)
				return;
			dy -= sy;
			sy = 0;
		}
		if (sx + width > amSrfWidthGet(surface)) {
			width = amSrfWidthGet(surface) - sx;
			if (width <= 0)
				return;
		}
		if (sy + height > amSrfHeightGet(surface)) {
			height = amSrfHeightGet(surface) - sy;
			if (height <= 0)
				return;
		}

		if (dx < 0) {
			width += dx;
			if (width <= 0)
				return;
			sx -= dx;
			dx = 0;
		}
		if (dy < 0) {
			height += dy;
			if (height <= 0)
				return;
			sy -= dy;
			dy = 0;
		}
		if (dx + width > layer->width) {
			width = layer->width - dx;
			if (width <= 0)
				return;
		}
		if (dy + height > layer->height) {
			height = layer->height - dy;
			if (height <= 0)
				return;
		}

		AM_ASSERT(width > 0 && height > 0);
		AM_ASSERT(sx >= 0 && sx < amSrfWidthGet(surface));
		AM_ASSERT(sy >= 0 && sy < amSrfHeightGet(surface));
		AM_ASSERT(dx >= 0 && dx < layer->width);
		AM_ASSERT(dy >= 0 && dy < layer->height);

		sy = amSrfHeightGet(surface) - sy - height;
		amPxlMapConvert((void *)(&layer->pixels[layer->width * (dy + height - 1)]), layer->format, -layer->dataStride, dx, 0, surface->alphaMaskPixels, VG_A_8, amSrfWidthGet(surface), sx, sy, width, height, AM_FALSE, AM_TRUE);

	#if defined(AM_GLE) || defined(AM_GLS)
		// invalidate image textures
		amImageTexturesInvalidate(layer, context);
	#endif
	}
}
#endif

// *********************************************************************
//                        Public implementations
// *********************************************************************

/*!
	\brief It modifies the drawing surface mask values according to a given operation, possibly using coverage
	values taken from a mask layer or bitmap image given by the mask parameter. If no mask is configured for the
	current drawing surface, vgMask has no effect.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param mask the image defining coverage values at each of its pixels.
	\param operation the mask operation.
	\param x x-coordinate of the first alpha mask pixel to modify.
	\param y y-coordinate of the first alpha mask pixel to modify.
	\param width width of the rectangular alpha mask region to modify, in pixels.
	\param height height of the rectangular alpha mask region to modify, in pixels.
*/
#if (AM_OPENVG_VERSION >= 110)
VG_API_CALL void VG_API_ENTRY vgMask(VGHandle mask,
#else
VG_API_CALL void VG_API_ENTRY vgMask(VGImage mask,
#endif
									 VGMaskOperation operation,
                                     VGint x,
                                     VGint y,
                                     VGint width,
                                     VGint height) VG_API_EXIT {

	const AMImage *img;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgMask");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (operation != VG_CLEAR_MASK && operation != VG_FILL_MASK &&
		amCtxHandleValid(currentContext, mask) != AM_IMAGE_HANDLE_ID
	#if (AM_OPENVG_VERSION >= 110)
		&& amCtxHandleValid(currentContext, mask) != AM_LAYER_HANDLE_ID
	#endif		
		) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgMask");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (width <= 0 || height <= 0 || operation < VG_CLEAR_MASK || operation > VG_SUBTRACT_MASK) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgMask");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// NULL values are allowed
	img = (const AMImage *)currentContext->handles->createdHandlesList.data[mask];
#if !defined(AM_STANDALONE)|| defined(RIM_VG_SRC)
	// if 'mask' is an image, check if it's used by EGL
	if (img && img->type == AM_IMAGE_HANDLE_ID && img->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgMask");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
#endif

	amMaskUpdate(currentSurface, img, operation, x, y, width, height);
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgMask");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

#if (AM_OPENVG_VERSION >= 110)
VG_API_CALL void VG_API_ENTRY vgRenderToMask(VGPath path,
											 VGbitfield paintModes,
											 VGMaskOperation operation) VG_API_EXIT {

	AMPath *pth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgRenderToMask");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgRenderToMask");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (paintModes == 0 || paintModes > (VG_STROKE_PATH | VG_FILL_PATH)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgRenderToMask");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (operation < VG_CLEAR_MASK || operation > VG_SUBTRACT_MASK) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgRenderToMask");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	if (!amRenderToMask(currentContext, currentSurface, pth, paintModes, operation)) {
		AM_MEMORY_LOG("vgRenderToMask (amRenderToMask fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		if (currentContext->beforePathRasterization) {
			// re-extract the path pointer, because it could be changed by amMemMngRetrieve
			pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
			// try to re-draw the path if the "out of memory" is not due to rasterization
			if (!amRenderToMask(currentContext, currentSurface, pth, paintModes, operation)) {
			#if defined(AM_DEBUG_MEMORY)
				amCtxCheckConsistence(currentContext);
			#endif
				amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
				AM_MEMORY_LOG("vgRenderToMask (amRenderToMask fail, before rasterization)");
				OPENVG_RETURN(OPENVG_NO_RETVAL)
			}
		}
		else {
			// "out of memory" is due to rasterization, so exit with a memory error
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
			AM_MEMORY_LOG("vgRenderToMask (amRenderToMask fail, after rasterization)");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	}
	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgRenderToMask");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

VG_API_CALL VGMaskLayer VG_API_ENTRY vgCreateMaskLayer(VGint width,
													   VGint height) VG_API_EXIT {

	AMImage *layer;
	VGMaskLayer handle;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgCreateMaskLayer");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// check for illegal arguments
	if (width <= 0 || height <= 0 ||
		width > currentContext->maxImageWidth || height > currentContext->maxImageHeight) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgCreateMaskLayer");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	if (width * height > currentContext->maxImagePixels) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgCreateMaskLayer");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// allocate the mask layer
	layer = (AMImage *)amMalloc(sizeof(AMImage));
	if (!layer) {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgCreateMaskLayer");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	// initialize the mask layer
	if (!amMaskLayerInit(layer, width, height, currentContext)) {
		AM_MEMORY_LOG("vgCreateMaskLayer (amMaskLayerInit fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		// try to re-initialize the mask layer
		if (!amMaskLayerInit(layer, width, height, currentContext)) {
		amFree(layer);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgCreateMaskLayer");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	}

	// add the new mask layer to the internal handles list of the context
	handle = amCtxHandleNew(currentContext, (AMhandle)layer);
	if (handle == VG_INVALID_HANDLE) {
		amMaskLayerDestroy(layer, currentContext);
		amFree(layer);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgCreateMaskLayer");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgCreateMaskLayer");
	OPENVG_RETURN(handle)
}

VG_API_CALL void VG_API_ENTRY vgDestroyMaskLayer(VGMaskLayer maskLayer) VG_API_EXIT {

	AMImage *layer;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgDestroyMaskLayer");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, maskLayer) != AM_LAYER_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgDestroyMaskLayer");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// invalidate handle and decrement reference counters
	layer = (AMImage *)currentContext->handles->createdHandlesList.data[maskLayer];
	AM_ASSERT(layer);
	amMaskLayerDestroy(layer, currentContext);

	if (layer->referenceCounter == 0) {
		// remove layer object from context internal list, and free associated pointer
		amCtxHandleRemove(currentContext, maskLayer);
		amFree(layer);
	}
	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgDestroyMaskLayer");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

VG_API_CALL void VG_API_ENTRY vgFillMaskLayer(VGMaskLayer maskLayer,
											  VGint x,
                                              VGint y,
                                              VGint width,
                                              VGint height,
											  VGfloat value) VG_API_EXIT {

	AMfloat color[4];
	AMImage *layer;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgFillMaskLayer");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, maskLayer) != AM_LAYER_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgFillMaskLayer");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	layer = (AMImage *)currentContext->handles->createdHandlesList.data[maskLayer];
	AM_ASSERT(layer);

	// handle NaN and Inf values
	value = amNanInfFix(value);
	
	// check for illegal arguments
	if (x < 0 || y < 0 ||
		width <= 0 || height <= 0 ||
		x + width > layer->width ||
		y + height > layer->height ||
		value < 0.0f || value > 1.0f) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgFillMaskLayer");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	color[AM_R] = 1.0f;
	color[AM_G] = 1.0f;
	color[AM_B] = 1.0f;
	color[AM_A] = value;

	amImageClear(layer, x, y, width, height, color, currentContext);
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgFillMaskLayer");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

VG_API_CALL void VG_API_ENTRY vgCopyMask(VGMaskLayer maskLayer,
										 VGint dx,
                                         VGint dy,
                                         VGint sx,
                                         VGint sy,
                                         VGint width,
										 VGint height) VG_API_EXIT {

	AMImage *layer;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgCopyMask");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, maskLayer) != AM_LAYER_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgCopyMask");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (width <= 0 || height <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgCopyMask");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	layer = (AMImage *)currentContext->handles->createdHandlesList.data[maskLayer];
	AM_ASSERT(layer);

	amCopyMask(currentContext, currentSurface, layer, dx, dy, sx, sy, width, height);
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgCopyMask");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}
#endif

/*!
	\brief It fills the portion of the drawing surface intersecting the specified rectangle with a constant
	color value, taken from the VG_CLEAR_COLOR parameter. The color value is expressed in non-premultiplied
	sRGBA (sRGB color plus alpha) format. Values outside the [0, 1] range are interpreted as the nearest
	endpoint of the range. The color is converted to the destination color space in the same manner as if
	a rectangular path were being filled. Clipping and scissoring take place in the usual fashion, but
	antialiasing, masking, and blending do not occur.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param x x-coordinate of the first pixel to clear.
	\param y y-coordinate of the first pixel to clear.
	\param width width of the rectangular region to clear, in pixels.
	\param height height of the rectangular region to clear, in pixels.
*/
VG_API_CALL void VG_API_ENTRY vgClear(VGint x,
									  VGint y,
                                      VGint width,
                                      VGint height) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgClear");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (width <= 0 || height <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgClear");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if (!amDrawingSurfaceClear(currentContext, currentSurface, x, y, width, height)) {
		AM_MEMORY_LOG("vgClear (amDrawingSurfaceClear fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
	}
	else
		amCtxErrorSet(currentContext, VG_NO_ERROR);

	AM_MEMORY_LOG("vgClear");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

