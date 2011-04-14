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
	\file sremask.c
	\brief Masking and clearing (SRE), implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#if defined(AM_SRE)

#include "sremask.h"
#include "vgconversions.h"
#include "vgpaint.h"
#if defined(AM_LITE_PROFILE)
	#include "pixel_utils.h"
#endif

#if defined(RIM_VG_SRC)
    extern void RasterFillC32(DWORD * dst,DWORD dstStride,DWORD dstx,DWORD width,DWORD height,DWORD colour);
#endif

/*!
	\brief It fills the portion of the drawing surface intersecting the specified rectangle with a constant
	color value, taken from the VG_CLEAR_COLOR parameter (SRE). The color value is expressed in non-premultiplied
	sRGBA (sRGB color plus alpha) format. Values outside the [0, 1] range are interpreted as the nearest
	endpoint of the range. The color is converted to the destination color space in the same manner as if
	a rectangular path were being filled. Clipping and scissoring take place in the usual fashion, but
	antialiasing, masking, and blending do not occur.
	\param context context containing the clearing color and scissor rectangles.
	\param surface drawing surface to be cleared.
	\param x x-coordinate of the first pixel to clear.
	\param y y-coordinate of the first pixel to clear.
	\param width width of the rectangular region to clear, in pixels.
	\param height height of the rectangular region to clear, in pixels.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amSreDrawingSurfaceClear(AMContext *context,
								AMDrawingSurface *surface,
								AMint32 x,
								AMint32 y,
								AMint32 width,
								AMint32 height) {

#if defined(AM_LITE_PROFILE)
	#define PIXEL_TYPE AMuint16
	#define PIXELS_MEMSET amMemset16
#else
	#define PIXEL_TYPE AMuint32
	#define PIXELS_MEMSET amMemset32
#endif

	#define CLIP_CLEAR_RECT \
		if (x < 0) { \
			width += x; \
			if (width <= 0) \
				return AM_TRUE; \
			x = 0; \
		} \
		if (y < 0) { \
			height += y; \
			if (height <= 0) \
				return AM_TRUE; \
			y = 0; \
		} \
		if (x + width > amSrfWidthGet(surface)) { \
			width = amSrfWidthGet(surface) - x; \
			if (width <= 0) \
				return AM_TRUE; \
		} \
		if (y + height > amSrfHeightGet(surface)) { \
			height = amSrfHeightGet(surface) - y; \
			if (height <= 0) \
				return AM_TRUE; \
		}

    #if defined(RIM_VG_SRC)
	#define CLEAR_RECT(_x, _y, _width, _height) \
		srfPixels = amSrfPixelsGet(surface) + ((_y) * amSrfWidthGet(surface)) + (_x); \
		RasterFillC32((DWORD*)srfPixels, amSrfWidthGet(surface), 0 , (DWORD) _width, (DWORD)_height, (DWORD) clearCol);
    #else
	#define CLEAR_RECT(_x, _y, _width, _height) \
		srfPixels = amSrfPixelsGet(surface) + ((_y) * amSrfWidthGet(surface)) + (_x); \
		for (j = (AMuint32)(_height); j != 0; --j) { \
			PIXELS_MEMSET(srfPixels, clearCol, (_width)); \
			srfPixels += amSrfWidthGet(surface); \
		}
    #endif

	#define MAX_DECOMPOSABLE_REGIONS_RATIO 1600

	AMfloat col[4];
	AMuint32 fullScreen;
	PIXEL_TYPE *srfPixels, clearCol;
	AMuint32 srfIdx = AM_FMT_GET_INDEX(amSrfFormat32Get(surface));

	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT(context);

	// according to drawing surfaces format (l/s, pre/nonpre) convert the non-linear non-premultiplied clear color
	AM_CLAMP4(col, context->clearColor, 0.0f, 1.0f)
#if defined(AM_LITE_PROFILE)
	clearCol = amPxlPack565(amColorPackByFormat(col, srfIdx));
#else
	clearCol = amColorPackByFormat(col, srfIdx);
#endif
	fullScreen = ((x <= 0) && (y <= 0) && (x + width >= amSrfWidthGet(surface)) && (y + height >= amSrfHeightGet(surface)));

	if (context->scissoring == VG_FALSE) {

		if (fullScreen) {

			AMuint32 srfPixelsCount = amSrfWidthGet(surface) * amSrfHeightGet(surface);
			AMuint32 regionsRatio = srfPixelsCount / (surface->dirtyRegions.size + 1);

			if (surface->wholeCleared && surface->clearColor == clearCol &&
				!surface->dirtyRegionsOverflow && regionsRatio > MAX_DECOMPOSABLE_REGIONS_RATIO &&
				surface->dirtyRegions.error == AM_DYNARRAY_NO_ERROR) {

				if (surface->dirtyRegions.size > 0) {
					
					AMAABox2i splitRegionsUnionBox;

                    #if defined (RIM_VG_SRC)
                        AMuint32 i, area, bbarea;
                    #else
					AMuint32 i, j, area, bbarea;
                    #endif

					// decompose dirty regions into a set of non-overlapping regions, then clear each decomposed region
					if (surface->dirtyRegions.size == 4) {
						splitRegionsUnionBox.minPoint.x = surface->dirtyRegions.data[0];
						splitRegionsUnionBox.minPoint.y = surface->dirtyRegions.data[1];
						splitRegionsUnionBox.maxPoint.x = surface->dirtyRegions.data[0] + surface->dirtyRegions.data[2];
						splitRegionsUnionBox.maxPoint.y = surface->dirtyRegions.data[1] + surface->dirtyRegions.data[3];
						area = bbarea = 0;
					}
					else {
						 if (!amRectsDecompose(&area, &surface->splitDirtyRegions, &splitRegionsUnionBox, (const AMInt32DynArray *)&surface->dirtyRegions, surface))
							 return AM_FALSE;
						bbarea = (splitRegionsUnionBox.maxPoint.x - splitRegionsUnionBox.minPoint.x) * (splitRegionsUnionBox.maxPoint.y - splitRegionsUnionBox.minPoint.y);
					}

					if (2 * area < bbarea) {
						for (i = 0; i < (AMuint32)surface->splitDirtyRegions.size; ++i) {
							width = surface->splitDirtyRegions.data[i].topRight.p.x - surface->splitDirtyRegions.data[i].bottomLeft.p.x;
							height = surface->splitDirtyRegions.data[i].topRight.p.y - surface->splitDirtyRegions.data[i].bottomLeft.p.y;
							x = surface->splitDirtyRegions.data[i].bottomLeft.p.x;
							y = amSrfHeightGet(surface) - (surface->splitDirtyRegions.data[i].bottomLeft.p.y + height);
							CLEAR_RECT(x, y, width, height)
						}
					}
					else {
						width = splitRegionsUnionBox.maxPoint.x - splitRegionsUnionBox.minPoint.x;
						height = splitRegionsUnionBox.maxPoint.y - splitRegionsUnionBox.minPoint.y;
						x = splitRegionsUnionBox.minPoint.x;
						y = amSrfHeightGet(surface) - (splitRegionsUnionBox.minPoint.y + height);
						CLEAR_RECT(x, y, width, height)
					}
				}
				else
					return AM_TRUE;
			}
			else
				PIXELS_MEMSET(amSrfPixelsGet(surface), clearCol, srfPixelsCount);

			surface->clearColor = clearCol;
			surface->wholeCleared = AM_TRUE;
		}
		else {
            #if !defined(RIM_VG_SRC)
			AMuint32 j;
            #endif

			// clip the clear rectangle against screen
			CLIP_CLEAR_RECT
			y = amSrfHeightGet(surface) - (y + height);
			AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
			CLEAR_RECT(x, y, width, height)
			surface->wholeCleared = AM_FALSE;
		}
		// rewind dirty regions array, and set overflow flag to false
			surface->dirtyRegions.size = 0;
		surface->dirtyRegions.error = AM_DYNARRAY_NO_ERROR;
		surface->dirtyRegionsOverflow = AM_FALSE;
	}
	else {
		AM_ASSERT(context->scissoring == VG_TRUE);

		// update scissor rectangle decomposition, if needed
		if (context->scissorRectsModified) {
			if (!amScissorRectsDecompose(context, surface))
				return AM_FALSE;
		}
		// if there aren't scissor rectangles, and scissoring is enabled, do not draw anything
		if (context->splitScissorRects.size < 1)
			return AM_TRUE;

		surface->wholeCleared = AM_FALSE;
		surface->dirtyRegions.size = 0;
		surface->dirtyRegions.error = AM_DYNARRAY_NO_ERROR;
		surface->dirtyRegionsOverflow = AM_FALSE;

		if (fullScreen) {
            #if defined(RIM_VG_SRC)
			    AMuint32 i;
            #else
			AMuint32 i, j;
            #endif

			for (i = 0; i < (AMuint32)context->splitScissorRects.size; ++i) {

				AMScissorRect *scissorRect = &context->splitScissorRects.data[i];
				
				width = scissorRect->topRight.p.x - scissorRect->bottomLeft.p.x;
				height = scissorRect->topRight.p.y - scissorRect->bottomLeft.p.y;
				x = scissorRect->bottomLeft.p.x;
				y = amSrfHeightGet(surface) - (scissorRect->bottomLeft.p.y + height);
				CLEAR_RECT(x, y, width, height)
			}
		}
		else {
			#if defined(RIM_VG_SRC)
			    AMuint32 i;
                        #else
			     AMuint32 i, j;
                        #endif
			AMAABox2i box0;

			// box0 = clearing region box
			AM_VECT2_SET(&box0.minPoint, x, y);
			AM_VECT2_SET(&box0.maxPoint, x + width, y + height);

			for (i = 0; i < (AMuint32)context->splitScissorRects.size; ++i) {

				AMAABox2i box1, intersectionBox;
				AMScissorRect *scissorRect = &context->splitScissorRects.data[i];
				
				// box1 = current scissor rectangle
				AM_VECT2_SET(&box1.minPoint, scissorRect->bottomLeft.p.x, scissorRect->bottomLeft.p.y);
				AM_VECT2_SET(&box1.maxPoint, scissorRect->topRight.p.x, scissorRect->topRight.p.y);
				if (amAABox2iIntersect(&intersectionBox, &box0, &box1)) {
					width = intersectionBox.maxPoint.x - intersectionBox.minPoint.x;
					height = intersectionBox.maxPoint.y - intersectionBox.minPoint.y;
					x = intersectionBox.minPoint.x;
					y = amSrfHeightGet(surface) - (intersectionBox.minPoint.y + height);
					CLEAR_RECT(x, y, width, height)
				}
			}
		}
	}

	return AM_TRUE;

	#undef CLIP_CLEAR_RECT
	#undef CLEAR_RECT
	#undef MAX_DECOMPOSABLE_REGIONS_RATIO
	#undef PIXELS_MEMSET
	#undef PIXEL_TYPE
}


#if defined RIM_VG_SRC
void amSreInternalDrawingSurfaceClear(AMDrawingSurface *surface,
							          AMint32 x,
							          AMint32 y,
							          AMint32 width,
							          AMint32 height) {
	AMuint32 *pixels32;

	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT(surface);

	pixels32 = (AMuint32 *)surface->pixels;

	if ((x <= 0) &&
		(y <= 0) &&
		(x + width >= surface->width) &&
		(y + height >= surface->height)) {

		// use amMemset32 to be as fast as possible
		amMemset32(pixels32, 0, surface->width * surface->height);
	}
}
#endif

#endif

#if defined (RIM_VG_SRC)
#pragma pop
#endif
