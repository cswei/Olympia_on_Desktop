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
	\file sredrawingsurface.c
	\brief OpenVG drawing surface (SRE), implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#if defined(AM_SRE)

#include "sredrawingsurface.h"
#include "vgdrawingsurface.h"
#include "vgconversions.h"
#include "vgpaint.h"
#include "pixel_utils.h"

/*!
	\brief Copy a subregion from a source pixelmap to a destination pixelmap, performing a SRC_OVER operation for each pixel.
	\param dstPixels pointer to the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels and srcPixels must point to valid 32bit aligned memory regions, with the same pixel format.
*/
void amPxlMapBlend(void *dstPixels,
				   const AMint32 dstDataStride,
				   const AMint32 dstX,
				   const AMint32 dstY,
				   const void *srcPixels,
				   const AMint32 srcDataStride,
				   const AMint32 srcX,
				   const AMint32 srcY,
				   const AMint32 width,
				   const AMint32 height) {

#if defined(AM_LITE_PROFILE)
	(void)dstPixels;
	(void)dstDataStride;
	(void)dstX;
	(void)dstY;
	(void)srcPixels;
	(void)srcDataStride;
	(void)srcX;
	(void)srcY;
	(void)width;
	(void)height;
	AM_ASSERT(0 == 1);
#else
	AMint32 x, y;
	const AMuint32 *src32 = (const AMuint32 *)srcPixels;
	AMuint32 *dst32 = (AMuint32 *)dstPixels;
	AMint32 srcJump = (srcDataStride >> 2);
	AMint32 dstJump = (dstDataStride >> 2);

	AM_ASSERT((srcDataStride & 0x03) == 0);
	AM_ASSERT((dstDataStride & 0x03) == 0);

	src32 += srcY * srcJump + srcX;
	dst32 += dstY * dstJump + dstX;

	for (y = height; y != 0; --y) {

		const AMuint32 *tmpSrc = src32;
		AMuint32 *tmpDst = dst32;

		for (x = width; x != 0; --x) {

			AMPixel32 pixel;

			pixel.value = *tmpSrc++;
			*tmpDst = pixel.value + amPxlScl255(pixel.c.a ^ 0xFF, *tmpDst);
			tmpDst++;
		}
		src32 += srcJump;
		dst32 += dstJump;
	}
#endif
}

// It copies pixel data from the source image onto the drawing surface, performing a SRC_OVER operation for each pixel.
// Full clip of the specified bound to the current drawing surface is performed too.
AMbool amSreDrawingSurfacePixelsBlend(AMContext *context,
									  AMDrawingSurface *surface,
									  AMint32 dx,
									  AMint32 dy,
									  VGImage src,
									  AMint32 sx,
									  AMint32 sy,
									  AMint32 width,
									  AMint32 height) {

	AMImage *srcImg;
	AMint32 tmpDy, srcX, srcY;
	const AMuint8 *data8;

	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(context->scissoring == VG_FALSE);
	AM_ASSERT(src > 0 && src < context->handles->createdHandlesList.size);
	AM_ASSERT(width > 0 && height > 0);

	srcImg = (AMImage *)context->handles->createdHandlesList.data[src];
	AM_ASSERT(srcImg);

	// clip specified bound to the current drawing surface
	if (sx < 0) {
		width += sx;
		if (width <= 0)
			return AM_TRUE;
		dx -= sx;
		sx = 0;
	}
	if (sy < 0) {
		height += sy;
		if (height <= 0)
			return AM_TRUE;
		dy -= sy;
		sy = 0;
	}
	if (sx + width > srcImg->width) {
		width = srcImg->width - sx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (sy + height > srcImg->height) {
		height = srcImg->height - sy;
		if (height <= 0)
			return AM_TRUE;
	}

	if (dx < 0) {
		width += dx;
		if (width <= 0)
			return AM_TRUE;
		sx -= dx;
		AM_ASSERT(sx < srcImg->width);
		dx = 0;
	}
	if (dy < 0) {
		height += dy;
		if (height <= 0)
			return AM_TRUE;
		sy -= dy;
		AM_ASSERT(sy < srcImg->height);
		dy = 0;
	}
	if (dx + width > amSrfWidthGet(surface)) {
		width = amSrfWidthGet(surface) - dx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (dy + height > amSrfHeightGet(surface)) {
		height = amSrfHeightGet(surface) - dy;
		if (height <= 0)
			return AM_TRUE;
	}

	if (surface->wholeCleared && !surface->dirtyRegionsOverflow) {
		// update dirty regions
#if defined ( RIM_VG_SRC )
    // Suppress compiler warning: "pointless comparison of unsigned integer with zero".
    #if ( AM_MAX_DIRTY_REGIONS_NUMBER > 0 )
		if ((surface->dirtyRegions.size >> 2) < AM_MAX_DIRTY_REGIONS_NUMBER) {
    #else
		if ( 0 ) {
    #endif
#else
		if ((surface->dirtyRegions.size >> 2) < AM_MAX_DIRTY_REGIONS_NUMBER) {
#endif
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, dx)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, dy)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, width)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, height)
		}
		else
			surface->dirtyRegionsOverflow = AM_TRUE;
	}

	// patch for screen buffer
	tmpDy = amSrfHeightGet(surface) - height - dy;
	// take care of child images
	srcX = sx + srcImg->x;
	srcY = sy + srcImg->y;
	data8 = (const AMuint8 *)srcImg->pixels;
	data8 += (srcY + height - 1) * srcImg->dataStride;
	amPxlMapBlend(amSrfPixelsGet(surface), amSrfDataStrideGet(surface), dx, tmpDy, (const void *)data8, -srcImg->dataStride, srcX, 0, width, height);
	return AM_TRUE;
}

// It copies pixel data from the source image onto the drawing surface swapping y coordinates, performing a SRC_OVER operation for each pixel.
// Full clip of the specified bound to the current drawing surface is performed too.
AMbool amSreDrawingSurfacePixelsBlendInvertedY(AMContext *context,
											   AMDrawingSurface *surface,
											   AMint32 dx,
											   AMint32 dy,
											   VGImage src,
											   AMint32 sx,
											   AMint32 sy,
											   AMint32 width,
											   AMint32 height) {

	AMImage *srcImg;
	AMint32 tmpDy, srcX, srcY;

	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(context->scissoring == VG_FALSE);
	AM_ASSERT(src > 0 && src < context->handles->createdHandlesList.size);
	AM_ASSERT(width > 0 && height > 0);

	srcImg = (AMImage *)context->handles->createdHandlesList.data[src];
	AM_ASSERT(srcImg);

	// clip specified bound to the current drawing surface
	if (sx < 0) {
		width += sx;
		if (width <= 0)
			return AM_TRUE;
		dx -= sx;
		sx = 0;
	}
	if (sy < 0) {
		height += sy;
		if (height <= 0)
			return AM_TRUE;
		dy -= sy;
		sy = 0;
	}
	if (sx + width > srcImg->width) {
		width = srcImg->width - sx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (sy + height > srcImg->height) {
		height = srcImg->height - sy;
		if (height <= 0)
			return AM_TRUE;
	}

	if (dx < 0) {
		width += dx;
		if (width <= 0)
			return AM_TRUE;
		sx -= dx;
		AM_ASSERT(sx < srcImg->width);
		dx = 0;
	}
	if (dy < 0) {
		height += dy;
		if (height <= 0)
			return AM_TRUE;
		AM_ASSERT(sy < srcImg->height);
		dy = 0;
	}
	if (dx + width > amSrfWidthGet(surface)) {
		width = amSrfWidthGet(surface) - dx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (dy + height > amSrfHeightGet(surface)) {
		sy += height - (amSrfHeightGet(surface) - dy);
		height = amSrfHeightGet(surface) - dy;
		if (height <= 0)
			return AM_TRUE;
		AM_ASSERT(sy < srcImg->height);
	}

	if (surface->wholeCleared && !surface->dirtyRegionsOverflow) {
		// update dirty regions
#if defined ( RIM_VG_SRC )
    // Suppress compiler warning: "pointless comparison of unsigned integer with zero".
    #if ( AM_MAX_DIRTY_REGIONS_NUMBER > 0 )
		if ((surface->dirtyRegions.size >> 2) < AM_MAX_DIRTY_REGIONS_NUMBER) {
    #else
		if ( 0 ) {
    #endif
#else
		if ((surface->dirtyRegions.size >> 2) < AM_MAX_DIRTY_REGIONS_NUMBER) {
#endif
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, dx)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, dy)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, width)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, height)
		}
		else
			surface->dirtyRegionsOverflow = AM_TRUE;
	}

	// patch for screen buffer
	tmpDy = amSrfHeightGet(surface) - height - dy;
	// take care of child images
	srcX = sx + srcImg->x;
	srcY = sy + srcImg->y;
	amPxlMapBlend(amSrfPixelsGet(surface), amSrfDataStrideGet(surface), dx, tmpDy, (const void *)srcImg->pixels, srcImg->dataStride, srcX, srcY, width, height);
	return AM_TRUE;
}

AMbool amSreDrawingSurfacePixelsSetInvertedY(AMContext *context,
											 AMDrawingSurface *surface,
											 AMint32 dx,
											 AMint32 dy,
											 VGImage src,
											 AMint32 sx,
											 AMint32 sy,
											 AMint32 width,
											 AMint32 height) {

	AMImage tmpImage, *srcImg;
	AMbool res;
	VGImage handle;

	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(context->scissoring == VG_FALSE);
	AM_ASSERT(src > 0 && src < context->handles->createdHandlesList.size);
	AM_ASSERT(width > 0 && height > 0);

	srcImg = (AMImage *)context->handles->createdHandlesList.data[src];
	AM_ASSERT(srcImg);

	// clip specified bound to the current drawing surface
	if (sx < 0) {
		width += sx;
		if (width <= 0)
			return AM_TRUE;
		dx -= sx;
		sx = 0;
	}
	if (sy < 0) {
		height += sy;
		if (height <= 0)
			return AM_TRUE;
		dy -= sy;
		sy = 0;
	}
	if (sx + width > srcImg->width) {
		width = srcImg->width - sx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (sy + height > srcImg->height) {
		height = srcImg->height - sy;
		if (height <= 0)
			return AM_TRUE;
	}

	if (dx < 0) {
		width += dx;
		if (width <= 0)
			return AM_TRUE;
		sx -= dx;
		AM_ASSERT(sx < srcImg->width);
		dx = 0;
	}
	if (dy < 0) {
		height += dy;
		if (height <= 0)
			return AM_TRUE;
		sy -= dy;
		AM_ASSERT(sy < srcImg->height);
		dy = 0;
	}
	if (dx + width > amSrfWidthGet(surface)) {
		width = amSrfWidthGet(surface) - dx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (dy + height > amSrfHeightGet(surface)) {
		height = amSrfHeightGet(surface) - dy;
		if (height <= 0)
			return AM_TRUE;
	}

	// implementation uses a temporary image, but pixels are not physically copied, just referenced
	tmpImage.id = AM_IMAGE_HANDLE_ID;
	tmpImage.type = AM_IMAGE_HANDLE_ID;
	tmpImage.format = srcImg->format;
	tmpImage.allowedQuality = VG_IMAGE_QUALITY_NONANTIALIASED;
	tmpImage.width = srcImg->width;
	tmpImage.height = srcImg->height;
	// y inversion
	tmpImage.pixels = &srcImg->pixels[(srcImg->root->height - 1) * srcImg->dataStride];
	tmpImage.dataStride = -srcImg->dataStride;
	tmpImage.x = srcImg->x;
	tmpImage.y = srcImg->root->height - (srcImg->y + srcImg->height);

	tmpImage.referenceCounter = 1;
	tmpImage.parent = srcImg->parent;
	tmpImage.root = srcImg->root;
	tmpImage.isPow2 = srcImg->isPow2;
	tmpImage.widthShift = srcImg->widthShift;
	tmpImage.heightShift = srcImg->heightShift;

	handle = amCtxHandleNew(context, (AMhandle)&tmpImage);
	if (handle == VG_INVALID_HANDLE)
		return AM_FALSE;
	res = amSreDrawingSurfacePixelsSet(context, surface, dx, dy, handle, sx, sy, width, height);
	amCtxHandleRemove(context, handle);
	return res;
}

/*!
	\brief It copies pixel data from the source image onto the drawing surface (SRE). Pixels whose source lies outside
	of the bounds of source image or whose destination lies outside the bounds of the drawing
	surface are ignored. Pixel format conversion is applied as needed. Scissoring takes place normally.
	Transformations, masking, and blending are not applied.
	\param context context containing the handle list (used to create / remove a temporary image, when needed) and the scissoring rectangles.
	\param surface the drawing surface where pixels are written to.
	\param dx x-coordinate of the rectangular region to write on the drawing surface.
	\param dy y-coordinate of the rectangular region to write on the drawing surface.
	\param src source image.
	\param sx x-coordinate of the rectangular region to read.
	\param sy y-coordinate of the rectangular region to read.
	\param width width of the rectangular region to set, in pixels.
	\param height height of the rectangular region to set, in pixels.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amSreDrawingSurfacePixelsSet(AMContext *context,
									AMDrawingSurface *surface,
									const AMint32 dx,
									const AMint32 dy,
									VGImage src,
									const AMint32 sx,
									const AMint32 sy,
									const AMint32 width,
									const AMint32 height) {
	
	AMImage *srcImg = (AMImage *)context->handles->createdHandlesList.data[src];
	
	AM_ASSERT(srcImg);
	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(src > 0 && src < context->handles->createdHandlesList.size);
	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT(sx >= 0 && sx < srcImg->width);
	AM_ASSERT(sy >= 0 && sy < srcImg->height);
	AM_ASSERT(dx >= 0 && dx < amSrfWidthGet(surface));
	AM_ASSERT(dy >= 0 && dy < amSrfHeightGet(surface));

	if (context->scissoring == VG_TRUE) {

		AMint32 i;
		AMVect2f p0, p1, p2, p3;
		AMImage child;
		AMPaintDesc paintDesc;
		AMAABox2i srfSpaceBox;
		AMMatrix33f userToSurface, invUserToSurface;
		AMfloat userToSurfaceScale[2] = { 1.0f, 1.0f };
		AMUserToSurfaceDesc userToSurfaceDesc;

		// update scissor rectangle decomposition, if needed
		if (context->scissorRectsModified) {
			if (!amScissorRectsDecompose(context, surface))
				return AM_FALSE;
		}
		// if there aren't scissor rectangles, and scissoring is enabled, do not draw anything
		if (context->splitScissorRects.size < 1)
			return AM_TRUE;
		else {
			AMAABox2i srfSpaceBox;

			AM_VECT2_SET(&srfSpaceBox.minPoint, dx, dy)
			AM_VECT2_SET(&srfSpaceBox.maxPoint, dx + width, dy + height)
			// update dirty regions
			if (surface->wholeCleared && !surface->dirtyRegionsOverflow) {

				AMAABox2i intersectionBox;

				if (amAABox2iIntersect(&intersectionBox, &context->splitScissorRectsBox, &srfSpaceBox)) {
					// update dirty regions
#if defined ( RIM_VG_SRC )
    // Suppress compiler warning: "pointless comparison of unsigned integer with zero".
    #if ( AM_MAX_DIRTY_REGIONS_NUMBER > 0 )
					if ((surface->dirtyRegions.size >> 2) < AM_MAX_DIRTY_REGIONS_NUMBER) {
    #else
					if ( 0 ) {
    #endif
#else
					if ((surface->dirtyRegions.size >> 2) < AM_MAX_DIRTY_REGIONS_NUMBER) {
#endif
						AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, intersectionBox.minPoint.x)
						AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, intersectionBox.minPoint.y)
						AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, intersectionBox.maxPoint.x - intersectionBox.minPoint.x)
						AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, intersectionBox.maxPoint.y - intersectionBox.minPoint.y)
					}
					else
						surface->dirtyRegionsOverflow = AM_TRUE;
				}
				else
					return AM_TRUE;
			}
		}

		// initialize a the temporary child image
		if (!amImageInit(&child, srcImg->format, VG_IMAGE_QUALITY_NONANTIALIASED, sx, sy, width, height, src, context))
			return AM_FALSE;

		// define the four corners of the drawing region
		AM_VECT2_SET(&p0, (AMfloat)dx, (AMfloat)dy)
		AM_VECT2_SET(&p2, p0.x + (AMfloat)width, p0.y + (AMfloat)height)
		AM_VECT2_SET(&p1, p2.x, p0.y)
		AM_VECT2_SET(&p3, p0.x, p2.y)

		AM_TRANSLATION_TO_MATRIX33(&userToSurface, (AMfloat)dx, (AMfloat)dy)
		AM_TRANSLATION_TO_MATRIX33(&invUserToSurface, (AMfloat)(-dx), (AMfloat)(-dy))
		userToSurfaceDesc.userToSurface = &userToSurface;
		userToSurfaceDesc.inverseUserToSurface = &invUserToSurface;
		userToSurfaceDesc.userToSurfaceScale = userToSurfaceScale;
		userToSurfaceDesc.flags = 0;
		userToSurfaceDesc.userToSurfaceAffine = AM_TRUE;

		// set a temporary paintDesc
		paintDesc.blendMode = VG_BLEND_SRC;
		paintDesc.imageMode = VG_DRAW_IMAGE_NORMAL;
		paintDesc.fillRule = VG_EVEN_ODD;
		paintDesc.imageQuality = VG_IMAGE_QUALITY_NONANTIALIASED;
		paintDesc.image = &child;
		paintDesc.userToSurfaceDesc = &userToSurfaceDesc;
		paintDesc.masking = AM_FALSE;
		paintDesc.renderingQuality = VG_RENDERING_QUALITY_NONANTIALIASED;
	#if (AM_OPENVG_VERSION >= 110)
		paintDesc.colorTransform = AM_FALSE;
	#endif

		AM_VECT2_SET(&srfSpaceBox.minPoint, dx, dy)
		AM_VECT2_SET(&srfSpaceBox.maxPoint, dx + width, dy + height)

		for (i = 0; i < (AMint32)context->splitScissorRects.size; ++i) {
			// extract and set the i-th clipping rectangle
			AMAABox2i clipBox;
			AMint32 x0 = context->splitScissorRects.data[i].bottomLeft.p.x;
			AMint32 y0 = context->splitScissorRects.data[i].bottomLeft.p.y;
			AMint32 x1 = context->splitScissorRects.data[i].topRight.p.x;
			AMint32 y1 = context->splitScissorRects.data[i].topRight.p.y;

			AM_VECT2_SET(&clipBox.minPoint, x0, y0)
			AM_VECT2_SET(&clipBox.maxPoint, x1, y1)

			if (amAABox2iOverlap(&clipBox, &srfSpaceBox)) {
				if (!amRasImageDraw(context, surface, context->rasterizer, &p0, &p1, &p2, &p3, &paintDesc, AM_FALSE, &clipBox)) {
					// destroy the temporary child image
					amImageDestroy(&child, context);
					return AM_FALSE;
				}
			}
		}
		// destroy the temporary child image
		amImageDestroy(&child, context);
	}
	else {
		const AMuint8 *data8;
		AMint32 tmpDy, srcX, srcY;

		if (surface->wholeCleared && !surface->dirtyRegionsOverflow) {
#if defined ( RIM_VG_SRC )
    // Suppress compiler warning: "pointless comparison of unsigned integer with zero".
    #if ( AM_MAX_DIRTY_REGIONS_NUMBER > 0 )
			if ((surface->dirtyRegions.size >> 2) < AM_MAX_DIRTY_REGIONS_NUMBER) {
    #else
			if ( 0 ) {
    #endif
#else
			if ((surface->dirtyRegions.size >> 2) < AM_MAX_DIRTY_REGIONS_NUMBER) {
#endif
				AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, dx)
				AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, dy)
				AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, width)
				AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, height)
			}
		}
		else
			surface->dirtyRegionsOverflow = AM_TRUE;

		// patch for screen buffer
		tmpDy = amSrfHeightGet(surface) - height - dy;

		// take care of child images
		srcX = sx + srcImg->x;
		srcY = sy + srcImg->y;
		data8 = (const AMuint8 *)srcImg->pixels;
		data8 += (srcY + height - 1) * srcImg->dataStride;
		amPxlMapConvert(amSrfPixelsGet(surface), (VGImageFormat)amSrfRealFormatGet(surface), amSrfDataStrideGet(surface), dx, tmpDy, (const void *)data8, srcImg->format, -srcImg->dataStride, srcX, 0, width, height, AM_FALSE, AM_TRUE);
	}

	return AM_TRUE;
}

/*!
	\brief It copies pixel data onto the drawing surface, without the creation of a VGImage object (SRE).
	The pixel values to be drawn are taken from the data pointer at the time of the amDrawingSurfacePixelsWrite
	call, so future changes to the data have no effect. Pixels whose destination coordinate lies outside the
	bounds of the drawing surface are ignored. Pixel format conversion is applied as needed. Scissoring takes
	place normally. Transformations, masking, and blending are not applied.
	\param context context containing the handle list (used to create / remove a temporary image, when needed) and the scissoring rectangles.
	\param surface the drawing surface where pixels are written to.
	\param dx x-coordinate of the rectangular region to write on the drawing surface.
	\param dy y-coordinate of the rectangular region to write on the drawing surface.
	\param data source pixels.
	\param dataFormat format of the source pixels.
	\param dataStride data stride of the source pixels.
	\param width width of the rectangular region to write, in pixels.
	\param height height of the rectangular region to write, in pixels.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amSreDrawingSurfacePixelsWrite(AMContext *context,
									  AMDrawingSurface *surface,
									  const AMint32 dx,
									  const AMint32 dy,
									  const void *data,
									  const VGImageFormat dataFormat,
									  const AMint32 dataStride,
									  const AMint32 width,
									  const AMint32 height) {

	AMImage tmpImage;
	AMbool res;
	AMuint32 srcIdx = AM_FMT_GET_INDEX(dataFormat);
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];
	AMuint32 src_bits = pxlFormatTable[srcIdx][FMT_BITS];

	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(data);
	AM_ASSERT(data != amSrfPixelsGet(surface));
	AM_ASSERT(width > 0 && height > 0);

	// - if scissoring is enabled we have to pass through the rasterizer (amDrawingSurfacePixelsSet)
	// - for premultiplied formats we have to pass through a temporary image to ensure valid premultiplied values (amImageSubDataSet)
	// - for 32bit formats without alpha we have to pass through a temporary image to ensure destination alpha channel = 255 (amImageSubDataSet)
	if ((context->scissoring == VG_TRUE) || (src_flags & FMT_PRE) || (!(src_flags & FMT_ALPHA) && src_bits == 32)) {

		res = amImageInit(&tmpImage, (VGImageFormat)amSrfRealFormatGet(surface), VG_IMAGE_QUALITY_NONANTIALIASED, 0, 0, width, height, 0, context);
		if (res) {
			// implementation uses a temporary image
			VGImage handle = amCtxHandleNew(context, (AMhandle)&tmpImage);

			if (handle == VG_INVALID_HANDLE) {
				amImageDestroy(&tmpImage, context);
				return AM_FALSE;
			}
			amImageSubDataSet(&tmpImage, data, dataFormat, dataStride, 0, 0, width, height, context);
			res = amDrawingSurfacePixelsSet(context, surface, dx, dy, handle, 0, 0, width, height);
			amImageDestroy(&tmpImage, context);
			amCtxHandleRemove(context, handle);
		}
	}
	else {
		VGImage handle;

		// implementation uses a temporary image, but pixels are not physically copied, just referenced
		tmpImage.id = AM_IMAGE_HANDLE_ID;
		tmpImage.type = AM_IMAGE_HANDLE_ID;
		tmpImage.format = dataFormat;
		tmpImage.allowedQuality = VG_IMAGE_QUALITY_NONANTIALIASED;
		tmpImage.width = width;
		tmpImage.height = height;
		tmpImage.pixels = (AMuint8 *)data;
		tmpImage.dataStride = dataStride;
		tmpImage.x = 0;
		tmpImage.y = 0;
		tmpImage.referenceCounter = 1;
		tmpImage.parent = 0;
		tmpImage.root = &tmpImage;
		// check if pow2
		if (amPow2Check(tmpImage.width) && amPow2Check(tmpImage.height)) {
			tmpImage.isPow2 = AM_TRUE;
			tmpImage.widthShift = amPow2Shift(tmpImage.width);
			tmpImage.heightShift = amPow2Shift(tmpImage.height);
		}
		else {
			tmpImage.isPow2 = AM_FALSE;
			tmpImage.widthShift = 0;
			tmpImage.heightShift = 0;
		}
		handle = amCtxHandleNew(context, (AMhandle)&tmpImage);
		if (handle == VG_INVALID_HANDLE)
			return AM_FALSE;
		res = amDrawingSurfacePixelsSet(context, surface, dx, dy, handle, 0, 0, width, height);
		amCtxHandleRemove(context, handle);
	}
	return res;
}

/*!
	\brief It retrieves pixel data from the drawing surface into the destination image (SRE). Pixels whose source
	lies outside of the bounds of the drawing surface or whose destination lies outside
	the bounds of destination image are ignored. Pixel format conversion is applied as needed. The
	scissoring region does not affect the reading of pixels.
	\param dst destination image.
	\param dx x-coordinate of the rectangular region to write on the destination image.
	\param dy y-coordinate of the rectangular region to write on the destination image.
	\param context context containing the handle list (used to create / remove a temporary image, when needed).
	\param surface the drawing surface where pixels are read from.
	\param sx x-coordinate of the rectangular region to read from the drawing surface.
	\param sy y-coordinate of the rectangular region to read from the drawing surface.
	\param width width of the rectangular region to get, in pixels.
	\param height height of the rectangular region to get, in pixels.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amSreDrawingSurfacePixelsGet(AMImage *dst,
									const AMint32 dx,
									const AMint32 dy,
									AMContext *context,
									const AMDrawingSurface *surface,
									const AMint32 sx,
									const AMint32 sy,
									const AMint32 width,
									const AMint32 height) {

	const AMuint8 *data8;
	AMint32 tmpSy;

	AM_ASSERT(dst);
	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT(sx >= 0 && sx < amSrfWidthGet(surface));
	AM_ASSERT(sy >= 0 && sy < amSrfHeightGet(surface));
	AM_ASSERT(dx >= 0 && dx < dst->width);
	AM_ASSERT(dy >= 0 && dy < dst->height);

	(void)context;
	// patch for screen buffer
	// th real sequence would be:
	// - sy = src->height - height - sy;
	// - sy = sy + height - 1;
	tmpSy = amSrfHeightGet(surface) - sy - 1;
	data8 = (const AMuint8 *)amSrfPixelsGet(surface) + (tmpSy * amSrfDataStrideGet(surface));
	amPxlMapConvert(dst->pixels, dst->format, dst->dataStride, dx, dy, (const void *)data8, (VGImageFormat)amSrfRealFormatGet(surface), -amSrfDataStrideGet(surface), sx, 0, width, height, AM_FALSE, AM_TRUE);
	return AM_TRUE;
}

/*!
	\brief It allows pixel data to be copied from the drawing surface without the creation of a VGImage object (SRE).
	Pixels whose source lies outside of the bounds of the drawing surface are ignored. Pixel format conversion
	is applied as needed. The scissoring region does not affect the reading of pixels.
	\param data destination pixels.
	\param dataFormat data format of destination pixels.
	\param dataStride data stride of destination pixels.
	\param dx x-coordinate of the rectangular region to write on the destination pixels.
	\param dy y-coordinate of the rectangular region to write on the destination pixels.
	\param context context containing the handle list (used to create / remove a temporary image, when needed).
	\param surface the drawing surface where pixels are read from.
	\param sx x-coordinate of the rectangular region to read from the drawing surface.
	\param sy y-coordinate of the rectangular region to read from the drawing surface.
	\param width width of the rectangular region to read, in pixels.
	\param height height of the rectangular region to read, in pixels.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amSreDrawingSurfacePixelsRead(void *data,
									 const VGImageFormat dataFormat,
									 const AMint32 dataStride,
									 const AMint32 dx,
									 const AMint32 dy,
									 const AMContext *context,
									 const AMDrawingSurface *surface,
									 const AMint32 sx,
									 const AMint32 sy,
									 const AMint32 width,
									 const AMint32 height) {

	const AMuint8 *data8;
	AMint32 tmpSy;

	AM_ASSERT(data);
	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(dx >= 0 && dy >= 0);
	AM_ASSERT(sx >= 0 && sy >= 0);
	AM_ASSERT(width > 0 && height > 0);

	(void)context;
	// patch for screen buffer
	tmpSy = amSrfHeightGet(surface) - sy - 1;
	data8 = (const AMuint8 *)amSrfPixelsGet(surface) + (tmpSy * amSrfDataStrideGet(surface));
	amPxlMapConvert(data, dataFormat, dataStride, dx, dy, (const void *)data8, (VGImageFormat)amSrfRealFormatGet(surface), -amSrfDataStrideGet(surface), sx, 0, width, height, AM_FALSE, AM_TRUE);
	return AM_TRUE;
}

/*!
	\brief It copies pixels from one region of the drawing surface to another (SRE). Copies between overlapping
	regions are allowed and always produce consistent results identical to copying the entire source region
	to a scratch buffer followed by copying the scratch buffer into the destination region.
	Pixels whose source or destination lies outside of the bounds of the drawing surface are ignored.
	Transformations, masking, and blending are not applied. Scissoring is applied to the destination, but
	does not affect the reading of pixels.
	\param context context containing the handle list (used to create / remove a temporary image, when needed).
	\param surface the drawing surface where the copy operation is performed on.
	\param dx x-coordinate of the rectangular region to write onto the drawing surface.
	\param dy y-coordinate of the rectangular region to write onto the drawing surface.
	\param sx x-coordinate of the rectangular region to read from the drawing surface.
	\param sy y-coordinate of the rectangular region to read from the drawing surface.
	\param width width of the rectangular region to copy, in pixels.
	\param height height of the rectangular region to copy, in pixels.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amSreDrawingSurfacePixelsCopy(AMContext *context,
									 AMDrawingSurface *surface,
									 const AMint32 dx,
									 const AMint32 dy,
									 const AMint32 sx,
									 const AMint32 sy,
									 const AMint32 width,
									 const AMint32 height) {

	AMAABox2i box0, box1;
	AMbool res;

	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT(sx >= 0 && sx < amSrfWidthGet(surface));
	AM_ASSERT(sy >= 0 && sy < amSrfHeightGet(surface));
	AM_ASSERT(dx >= 0 && dx < amSrfWidthGet(surface));
	AM_ASSERT(dy >= 0 && dy < amSrfHeightGet(surface));

	// if regions overlap, use a temporary image
	AM_VECT2_SET(&box0.minPoint, sx, sy)
	AM_VECT2_SET(&box0.maxPoint, sx + width, sy + height)
	AM_VECT2_SET(&box1.minPoint, dx, dy)
	AM_VECT2_SET(&box1.maxPoint, dx + width, dy + height)

	if (amAABox2iOverlap(&box0, &box1) || context->scissoring == VG_TRUE) {

		AMImage tmpImage;
		
		res = amImageInit(&tmpImage, (VGImageFormat)amSrfRealFormatGet(surface), VG_IMAGE_QUALITY_NONANTIALIASED, 0, 0, width, height, 0, context);
		if (res) {
			VGImage handle = amCtxHandleNew(context, (AMhandle)&tmpImage);
			
			if (handle == VG_INVALID_HANDLE) {
				amImageDestroy(&tmpImage, context);
				return AM_FALSE;
			}

			res = amDrawingSurfacePixelsGet(&tmpImage, 0, 0, context, surface, sx, sy, width, height);
			if (res)
				// amDrawingSurfacePixelsSet will take care of updating dirty regions too
				res = amDrawingSurfacePixelsSet(context, surface, dx, dy, handle, 0, 0, width, height);

			amImageDestroy(&tmpImage, context);
			amCtxHandleRemove(context, handle);
		}
	}
	else {
		AMint32 srcY = amSrfHeightGet(surface) - (height + sy);
		AMint32 dstY = amSrfHeightGet(surface) - (height + dy);

		if (surface->wholeCleared && !surface->dirtyRegionsOverflow) {
			// update dirty regions
#if defined ( RIM_VG_SRC )
    // Suppress compiler warning: "pointless comparison of unsigned integer with zero".
    #if ( AM_MAX_DIRTY_REGIONS_NUMBER > 0 )
			if ((surface->dirtyRegions.size >> 2) < AM_MAX_DIRTY_REGIONS_NUMBER) {
    #else
			if ( 0 ) {
    #endif
#else
			if ((surface->dirtyRegions.size >> 2) < AM_MAX_DIRTY_REGIONS_NUMBER) {
#endif
				AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, dx)
				AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, dy)
				AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, dx + width)
				AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, dy + height)
			}
			else
				surface->dirtyRegionsOverflow = AM_TRUE;
		}

		amPxlMapConvert(amSrfPixelsGet(surface), (VGImageFormat)amSrfRealFormatGet(surface), amSrfDataStrideGet(surface), dx, dstY,
						amSrfPixelsGet(surface), (VGImageFormat)amSrfRealFormatGet(surface), amSrfDataStrideGet(surface), sx, srcY,
						width, height, AM_FALSE, AM_TRUE);
		res = AM_TRUE;
	}

	return res;
}

#endif

#if defined (RIM_VG_SRC)
#pragma pop
#endif

