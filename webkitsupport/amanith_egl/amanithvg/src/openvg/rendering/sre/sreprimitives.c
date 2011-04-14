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
	\file sreprimitives.c
	\brief Path and image drawing functions entry point (SRE), implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif


#if defined(AM_SRE)

#include "sreprimitives.h"
#include "vgprimitives.h"
#include "vgmatrix.h"
#include "vgpaint.h"
#if (AM_OPENVG_VERSION >= 110)
	#include "pixel_utils.h"
#endif

/*!
	\brief Update stroke software cache in a given cache slot (SRE).
	\param slot output cache slot, where stroke cache is updated.
	\param context input context containing stroke auxiliary structures.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amSreStrokeCacheUpdate(AMPathCacheSlot *slot,
							const AMContext *context) {

#if defined(AM_FIXED_POINT_PIPELINE)
	#define STROKE_VECT_TYPE AMVect2x
#else
	#define STROKE_VECT_TYPE AMVect2f
#endif

	AM_ASSERT(slot);
	AM_ASSERT(context);

	// allocate/reallocate memory used to store stroke points
	if (!slot->strokePts.data) {
		// create the dynamic array
		AM_DYNARRAY_INIT_RESERVE(slot->strokePts, STROKE_VECT_TYPE, context->strokeAuxPts.size)
	}
	else {
		// expand the dynamic array only if needed
	if (slot->strokePts.capacity < context->strokeAuxPts.size) {
		AM_DYNARRAY_CLEAR_RESERVE(slot->strokePts, STROKE_VECT_TYPE, context->strokeAuxPts.size)
	}
	}
	// check for memory errors
	if (slot->strokePts.error) {
		slot->strokePts.error = AM_DYNARRAY_NO_ERROR;
		return AM_FALSE;
	}
	// copy stroke points from the context to the path slot
	slot->strokePts.size = context->strokeAuxPts.size;
	amMemcpy(slot->strokePts.data, context->strokeAuxPts.data, context->strokeAuxPts.size * sizeof(STROKE_VECT_TYPE));

	// allocate/reallocate memory used to store stroke points-per-contour
	if (!slot->strokePtsPerContour.data) {
		// create the dynamic array
		AM_DYNARRAY_INIT_RESERVE(slot->strokePtsPerContour, AMint32, context->strokeAuxPtsPerContour.size)
	}
	else {
	if (slot->strokePtsPerContour.capacity < context->strokeAuxPtsPerContour.size) {
		AM_DYNARRAY_CLEAR_RESERVE(slot->strokePtsPerContour, AMint32, context->strokeAuxPtsPerContour.size)
	}
	}
	// check for memory errors
	if (slot->strokePtsPerContour.error) {
		slot->strokePtsPerContour.error = AM_DYNARRAY_NO_ERROR;
		return AM_FALSE;
	}
	// copy stroke points-per-contour from the context to the path slot
	slot->strokePtsPerContour.size = context->strokeAuxPtsPerContour.size;
	amMemcpy(slot->strokePtsPerContour.data, context->strokeAuxPtsPerContour.data, context->strokeAuxPtsPerContour.size * sizeof(AMint32));

	return AM_TRUE;
	#undef STROKE_VECT_TYPE
}

/*!
	\brief Entry point for path drawing function (SRE).
	\param context context containing all OpenVG states.
	\param surface the drawing surface where the path is going to be drawn to.
	\param userToSurfaceDesc descriptor for the "user to surface" matrix that will be applied during path drawing.
	\param path the path to draw.
	\param paintModes paint modes to use (fill and/or stroke).
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
*/
AMbool amSrePathDraw(AMContext *context,
					 AMDrawingSurface *surface,
					 AMUserToSurfaceDesc *userToSurfaceDesc,
					 AMPath *path,
					 const VGbitfield paintModes) {

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
	AMfloat roundJoinAuxCoef, tmpFloat;
	AMbool objSpaceBoxZeroDimension, doneFlattening, pathNeedClipping;

	AM_ASSERT(path);
	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(paintModes > 0);

	AM_ASSERT(!(userToSurfaceDesc->flags & AM_MATRIX_SINGULAR) && path->segments.size > 0);

	context->beforePathRasterization = AM_TRUE;
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
	amAABox2fTransformx(&srfSpaceStrokeBox, &objSpaceStrokeBox, &userToSurfaceDesc->userToSurfacex);
	// align drawing surface space box to its nearest integer pixel
	AM_VECT2_SET(&p0, (srfSpaceStrokeBox.minPoint.x - AM_RAS_FIXED_MASK - AM_RAS_FIXED_ONE) >> AM_RAS_FIXED_PRECISION, (srfSpaceStrokeBox.minPoint.y - AM_RAS_FIXED_MASK - AM_RAS_FIXED_ONE) >> AM_RAS_FIXED_PRECISION)
	AM_VECT2_SET(&p1, (srfSpaceStrokeBox.maxPoint.x + AM_RAS_FIXED_MASK + AM_RAS_FIXED_ONE) >> AM_RAS_FIXED_PRECISION, (srfSpaceStrokeBox.maxPoint.y + AM_RAS_FIXED_MASK + AM_RAS_FIXED_ONE) >> AM_RAS_FIXED_PRECISION)
	AM_SRFSPACE_CORNER_CLAMPX(srfSpaceBox.minPoint, p0)
	AM_SRFSPACE_CORNER_CLAMPX(srfSpaceBox.maxPoint, p1)
#else
	// generate drawing surface space bounding box, according to "path user to surface" matrix
	amAABox2fTransform(&srfSpaceStrokeBox, &objSpaceStrokeBox, userToSurfaceDesc->userToSurface);
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
	// reject the whole draw operation if drawing surface space box lies outside the viewport
	if (srfSpaceBox.minPoint.x >= amSrfWidthGet(surface) ||
		srfSpaceBox.minPoint.y >= amSrfHeightGet(surface) ||
		srfSpaceBoxWidth <= 0 || srfSpaceBoxHeight <= 0)
		return AM_TRUE;

	srfSpaceBox.maxPoint.x = srfSpaceBox.minPoint.x + srfSpaceBoxWidth;
	srfSpaceBox.maxPoint.y = srfSpaceBox.minPoint.y + srfSpaceBoxHeight;

	// calculate path deviation
	amPathDeviationUpdate(context, path, paintModes, &srfSpaceBox, userToSurfaceDesc);

	// extract the slot for flattening, so we are sure that for drawing operations the correct slot will be used
	if (!amPathFlatten(&slotIndex, path, context, &doneFlattening))
		return AM_FALSE;

	// update scissoring
	if (context->scissoring == VG_TRUE) {
		// update scissor rectangle decomposition, if needed
		if (context->scissorRectsModified) {
			if (!amScissorRectsDecompose(context, surface))
				return AM_FALSE;
		}
		// if there aren't scissor rectangles, and scissoring is enabled, do not draw anything
		if (context->splitScissorRects.size < 1)
			return AM_TRUE;
		else {
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
	}
	else {
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
				AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, srfSpaceBox.minPoint.x)
				AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, srfSpaceBox.minPoint.y)
				AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, srfSpaceBoxWidth)
				AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, srfSpaceBoxHeight)
			}
			else
				surface->dirtyRegionsOverflow = AM_TRUE;
		}
	}

	if ((paintModes & VG_FILL_PATH) && !objSpaceBoxZeroDimension) {

		AMPaintDesc paintDesc;

		// update path style and get a paint descriptor
		if (!amPaintDescPathFillGet(&paintDesc, context, surface, userToSurfaceDesc, paintModes))
			return AM_FALSE;

		// if paintType is different than color, we have to draw fill only if associated matrix is not singular
		if ((paintDesc.paintType == VG_PAINT_TYPE_COLOR || !(context->fillPaintToUserFlags & AM_MATRIX_SINGULAR))
		#if defined(VG_MZT_advanced_blend_modes)
			&& paintDesc.blendMode != VG_BLEND_DST_MZT
		#endif
			) {
			
			context->beforePathRasterization = AM_FALSE;

			if (context->scissoring == VG_TRUE) {

				AMuint32 i;

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
						if (!amRasPolygonsDraw(context, surface, context->rasterizer,
											#if defined(AM_FIXED_POINT_PIPELINE)
												&path->cache[slotIndex].flattenPtsx,
											#else
												&path->cache[slotIndex].flattenPts,
											#endif
												&path->cache[slotIndex].ptsPerContour,
												&paintDesc, &clipBox, pathNeedClipping))
							return AM_FALSE;
					}
					else
					if (amAABox2iOverlap(&clipBox, &srfSpaceBox)) {
						if (!amRasPolygonsDraw(context, surface, context->rasterizer,
											#if defined(AM_FIXED_POINT_PIPELINE)
												&path->cache[slotIndex].flattenPtsx,
											#else
												&path->cache[slotIndex].flattenPts,
											#endif
												&path->cache[slotIndex].ptsPerContour,
												&paintDesc, &clipBox, AM_TRUE))
							return AM_FALSE;
					}
				}
			}
			else {
				AMAABox2i clipBox;

				// set a clipBox equal to the whole drawing surface
				AM_VECT2_SET(&clipBox.minPoint, 0, 0)
				AM_VECT2_SET(&clipBox.maxPoint, amSrfWidthGet(surface), amSrfHeightGet(surface))
				// rasterize the fill polygons
				if (!amRasPolygonsDraw(context, surface, context->rasterizer,
									#if defined(AM_FIXED_POINT_PIPELINE)
										&path->cache[slotIndex].flattenPtsx,
									#else
										&path->cache[slotIndex].flattenPts,
									#endif
										&path->cache[slotIndex].ptsPerContour,
										&paintDesc, &clipBox, pathNeedClipping))
					return AM_FALSE;
			}
		}
	}

	if (paintModes & VG_STROKE_PATH && context->strokeLineWidth > 0.0f) {

		AMPaintDesc paintDesc;

		// update path style and get a paint descriptor
		if (!amPaintDescPathStrokeGet(&paintDesc, context, surface, userToSurfaceDesc))
			return AM_FALSE;

		// if paintType is different than color, we have to draw stroke only if associated matrix is not singular
		if ((paintDesc.paintType == VG_PAINT_TYPE_COLOR || !(context->strokePaintToUserFlags & AM_MATRIX_SINGULAR))
		#if defined(VG_MZT_advanced_blend_modes)
			&& paintDesc.blendMode != VG_BLEND_DST_MZT
		#endif
			) {
			// calculate an invalidating hash, so that if the amStrokeGenerate will fail, the next time amStrokeChanged will return AM_TRUE
			AMuint32 invalidatingHash = path->cache[slotIndex].strokeDesc.dashPatternHash ^ 0xFFFFFFFF;

			// if one of the stroke parameter has changed, rebuild the entire stroke cache
			if (amStrokeChanged(&path->cache[slotIndex].strokeDesc, context) || doneFlattening) {

				// update the coefficient used to know how many points are generated by a round cap/join
				AMfloat devOverRadius = AM_MAX(context->flattenParams.flatness / context->strokeLineWidth, 1e-5f);

				AM_ASSERT(context->strokeLineThickness != 0.0f);
				
				roundJoinAuxCoef = (devOverRadius >= 2.0f) ? 1.0f / AM_2PI : 1.0f / (2.0f * amAcosf(1.0f - devOverRadius));
				if (!amStrokeGenerate(context, &path->cache[slotIndex], roundJoinAuxCoef)) {
					path->cache[slotIndex].strokeDesc.dashPatternHash = invalidatingHash;
					return AM_FALSE;
				}
				if (!amSreStrokeCacheUpdate(&path->cache[slotIndex], context))
					return AM_FALSE;
			}

			context->beforePathRasterization = AM_FALSE;

			if (context->scissoring == VG_TRUE) {

				AMuint32 i;

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
						if (!amRasPolygonsDraw(context, surface, context->rasterizer, &path->cache[slotIndex].strokePts, &path->cache[slotIndex].strokePtsPerContour, &paintDesc, &clipBox, pathNeedClipping))
							return AM_FALSE;
					}
					else
					if (amAABox2iOverlap(&clipBox, &srfSpaceBox)) {
						if (!amRasPolygonsDraw(context, surface, context->rasterizer, &path->cache[slotIndex].strokePts, &path->cache[slotIndex].strokePtsPerContour, &paintDesc, &clipBox, AM_TRUE))
							return AM_FALSE;
				}
			}
			}
			else {
				AMAABox2i clipBox;

				// set a clipBox equal to the whole drawing surface
				AM_VECT2_SET(&clipBox.minPoint, 0, 0)
				AM_VECT2_SET(&clipBox.maxPoint, amSrfWidthGet(surface), amSrfHeightGet(surface))
				// rasterize the stroke polygons
				if (!amRasPolygonsDraw(context, surface, context->rasterizer, &path->cache[slotIndex].strokePts, &path->cache[slotIndex].strokePtsPerContour, &paintDesc, &clipBox, pathNeedClipping))
					return AM_FALSE;
			}
		}
	}
	return AM_TRUE;
}

/*!
	\brief Entry point for image drawing function (SRE).
	\param context context containing all OpenVG states.
	\param surface the drawing surface where the image is going to be drawn to.
	\param userToSurfaceDesc descriptor for the "user to surface" matrix that will be applied during image drawing.
	\param image the image to draw.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
*/
AMbool amSreImageDraw(AMContext *context,
					  AMDrawingSurface *surface,
					  AMUserToSurfaceDesc *userToSurfaceDesc,
					  AMImage *image) {

	AMAABox2i srfSpaceBox;
	AMint32 srfSpaceBoxWidth, srfSpaceBoxHeight;
	AMPaintDesc paintDesc;
	AMVect2f p0, p1, p2, p3;
	AMbool clearImageMode;

	AM_ASSERT(image);
	AM_ASSERT(image->width > 0 && image->height > 0);
	AM_ASSERT(context);
	AM_ASSERT(surface);

	if (!amImageToDraw(&srfSpaceBox, &p0, &p1, &p2, &p3, image, userToSurfaceDesc))
		return AM_TRUE;

	// convert srfSpaceBox into integer values (good for possible grab operations)
	srfSpaceBoxWidth = AM_AABOX2_WIDTH(&srfSpaceBox);
	srfSpaceBoxHeight = AM_AABOX2_HEIGHT(&srfSpaceBox);
	// clamp drawing surface space box to viewport
	if (srfSpaceBox.minPoint.x < 0) {
		srfSpaceBoxWidth += srfSpaceBox.minPoint.x;
		srfSpaceBox.minPoint.x = 0;
	}
	if (srfSpaceBox.minPoint.y < 0) {
		srfSpaceBoxHeight += srfSpaceBox.minPoint.y;
		srfSpaceBox.minPoint.y = 0;
	}
	if (srfSpaceBox.minPoint.x + srfSpaceBoxWidth > amSrfWidthGet(surface)) {
		srfSpaceBoxWidth = amSrfWidthGet(surface) - srfSpaceBox.minPoint.x;
		if (srfSpaceBoxWidth <= 0)
			return AM_TRUE;
	}
	if (srfSpaceBox.minPoint.y + srfSpaceBoxHeight > amSrfHeightGet(surface)) {
		srfSpaceBoxHeight = amSrfHeightGet(surface) - srfSpaceBox.minPoint.y;
		if (srfSpaceBoxHeight <= 0)
			return AM_TRUE;
	}
	// reject the whole draw operation if drawing surface space box lies outside the viewport
	if (srfSpaceBox.minPoint.x >= amSrfWidthGet(surface) ||
		srfSpaceBox.minPoint.y >= amSrfHeightGet(surface) ||
		srfSpaceBoxWidth <= 0 || srfSpaceBoxHeight <= 0)
		return AM_TRUE;

#if defined(VG_MZT_advanced_blend_modes)
	// DST is the null pass
	if (context->fillBlendMode == VG_BLEND_DST_MZT)
		return AM_TRUE;
#endif

	srfSpaceBox.maxPoint.x = srfSpaceBox.minPoint.x + srfSpaceBoxWidth;
	srfSpaceBox.maxPoint.y = srfSpaceBox.minPoint.y + srfSpaceBoxHeight;

	// update scissoring
	if (context->scissoring == VG_TRUE) {
		// update scissor rectangle decomposition, if needed
		if (context->scissorRectsModified) {
			if (!amScissorRectsDecompose(context, surface))
				return AM_FALSE;
		}
		// if there aren't scissor rectangles, and scissoring is enabled, do not draw anything
		if (context->splitScissorRects.size < 1)
			return AM_TRUE;
		else {
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
	}
	else {
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
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, srfSpaceBox.minPoint.x)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, srfSpaceBox.minPoint.y)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, srfSpaceBoxWidth)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, srfSpaceBoxHeight)
		}
			else
				surface->dirtyRegionsOverflow = AM_TRUE;
		}
	}

	// get paint descriptor and update style
	if (!amPaintDescImageFillGet(&paintDesc, context, surface, userToSurfaceDesc, image))
		return AM_FALSE;
	clearImageMode = AM_FALSE;

#if defined(VG_MZT_advanced_blend_modes)
	// for a CLEAR blendMode, simply use the path rasterizer to optimize performance
	if (context->fillBlendMode == VG_BLEND_CLEAR_MZT) {
		paintDesc.blendMode = VG_BLEND_SRC;
		paintDesc.paintType = VG_PAINT_TYPE_COLOR;
		paintDesc.paintColor[AM_R] = 0.0f;
		paintDesc.paintColor[AM_G] = 0.0f;
		paintDesc.paintColor[AM_B] = 0.0f;
		paintDesc.paintColor[AM_A] = 0.0f;
		clearImageMode = AM_TRUE;
	#if (AM_OPENVG_VERSION >= 110)
		paintDesc.colorTransform = AM_FALSE;
	#endif
	}
#endif

	if (context->scissoring == VG_TRUE) {

		AMuint32 i;

		for (i = 0; i < context->splitScissorRects.size; ++i) {
			// extract and set the i-th clipping rectangle
			AMAABox2i clipBox;
			AMint32 x0 = context->splitScissorRects.data[i].bottomLeft.p.x;
			AMint32 y0 = context->splitScissorRects.data[i].bottomLeft.p.y;
			AMint32 x1 = context->splitScissorRects.data[i].topRight.p.x;
			AMint32 y1 = context->splitScissorRects.data[i].topRight.p.y;

			AM_VECT2_SET(&clipBox.minPoint, x0, y0)
			AM_VECT2_SET(&clipBox.maxPoint, x1, y1)
			if (amAABox2iOverlap(&clipBox, &srfSpaceBox)) {
				if (!amRasImageDraw(context, surface, context->rasterizer, &p0, &p1, &p2, &p3, &paintDesc, clearImageMode, &clipBox))
					return AM_FALSE;
			}
		}
	}
	else {
		AMAABox2i clipBox;

		// set a clipBox equal to the whole drawing surface
		AM_VECT2_SET(&clipBox.minPoint, 0, 0)
		AM_VECT2_SET(&clipBox.maxPoint, amSrfWidthGet(surface), amSrfHeightGet(surface))
		if (!amRasImageDraw(context, surface, context->rasterizer, &p0, &p1, &p2, &p3, &paintDesc, clearImageMode, &clipBox))
			return AM_FALSE;
	}

	return AM_TRUE;
}

#if (AM_OPENVG_VERSION >= 110)
void amSreImageGlyphBlit(AMContext *context,
						 AMDrawingSurface *surface,
						 const AMUserToSurfaceDesc *userToSurfaceDesc,
						 const AMImage *image,
						 const AMAABox2i *imageBox) {

	AMuint32 x, y, width, height, fru, i, j;
	// take care of child images
	AMuint32 childX = (AMuint32)image->x;
	AMuint32 childY = (AMuint32)image->y;
	AMuint32 stride = (AMuint32)image->root->width;
	const AMuint32 *src32 = (const AMuint32 *)image->pixels;
	AMuint32 *dst32 = (AMuint32 *)amSrfPixelsGet(surface);

	AM_ASSERT(surface);
	AM_ASSERT(userToSurfaceDesc);
	AM_ASSERT(image);

	// update dirty regions
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
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, imageBox->minPoint.x)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, imageBox->minPoint.y)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, imageBox->maxPoint.x - imageBox->minPoint.x)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, imageBox->maxPoint.y - imageBox->minPoint.y)
		}
		else
			surface->dirtyRegionsOverflow = AM_TRUE;
	}

	if (context->renderingQuality != VG_RENDERING_QUALITY_NONANTIALIASED) {
		x = (AMuint32)(userToSurfaceDesc->userToSurface->a[0][2] * 256.0f);
		fru = x & 0xFF;
		x >>= 8;
	}
	else {
		x = (AMuint32)(userToSurfaceDesc->userToSurface->a[0][2] + 0.5f);
		fru = 0;
	}
	width = (AMuint32)image->width;

	y = (AMuint32)(userToSurfaceDesc->userToSurface->a[1][2] + 0.5f);
	height = (AMuint32)image->height;

	// first interested scanline inside the source image
	src32 += childY * stride + childX;
	// first interested scanline in the drawing surface
	dst32 = &dst32[(amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface)];

	if (fru) {
		// 255 - fru
		AMuint32 invFru = fru ^ 0xFF;

		for (i = height; i != 0; --i) {

			AMuint32 cov;
			AMPixel32 p00;
			AMuint32 dx = x;

			// first pixel of the current scanline
			p00.value = src32[0];
			MULT_DIV_255(cov, invFru, p00.c.a)
			AM_ASSERT(dx >= 0 && dx < (AMuint32)amSrfWidthGet(surface));
			dst32[dx] = p00.value + amPxlScl255(cov ^ 0xFF, dst32[dx]);
			dx++;

			for (j = 1; j < width; ++j) {

				AMPixel32 p10, pix;

				// read an image pixel
				p10.value = src32[j];
				// skip writing if both pixels are fully transparent
				if (!p00.c.a && !p10.c.a)
					goto skipTransparentPixels;
				// lerp the two pixels and write the result
				pix.value = amPxlLerp(invFru, p00.value, p10.value);
				AM_ASSERT(dx >= 0 && dx < (AMuint32)amSrfWidthGet(surface));
				dst32[dx] = pix.value + amPxlScl255(pix.c.a ^ 0xFF, dst32[dx]);

skipTransparentPixels:
				// update p00
				p00.value = p10.value;
				// next pixel in the drawing surface
				dx++;
			}
			// last pixel of the current scanline
			MULT_DIV_255(cov, fru, p00.c.a)
			AM_ASSERT(dx >= 0 && dx < (AMuint32)amSrfWidthGet(surface));
			dst32[dx] = p00.value + amPxlScl255(cov ^ 0xFF, dst32[dx]);
			// next scanlines
			src32 += stride;
			dst32 -= amSrfWidthGet(surface);
		}
	}
	else {

		for (i = height; i != 0; --i) {

			AMuint32 dx = x;

			for (j = 0; j < width; ++j) {

				AMPixel32 p00;

				// get pixel
				p00.value = src32[j];
				AM_ASSERT(dx >= 0 && dx < (AMuint32)amSrfWidthGet(surface));
				// skip writing if pixel is fully transparent
				if (p00.c.a)
					dst32[dx] = p00.value + amPxlScl255(p00.c.a ^ 0xFF, dst32[dx]);
				// next pixel in the drawing surface
				dx++;
			}
			// next scanlines
			src32 += stride;
			dst32 -= amSrfWidthGet(surface);
		}
	}
}

AMbool amSreImageGlyphStretch(AMContext *context,
							  AMDrawingSurface *surface,
							  AMUserToSurfaceDesc *userToSurfaceDesc,
							  AMImage *image,
							  const AMAABox2i *imageBox) {

	AMuint32 srcStepX, srcStepY, srcX, srcY, dstX, dstY, dstFru, childX, childY, stride, *dst32, i, j;
	VGImageQuality imageQuality;
	const AMuint32 *src32;
	AMint32 dstWidth = (AMint32)((AMfloat)image->width * userToSurfaceDesc->userToSurface->a[0][0]);
	AMint32 dstHeight = (AMint32)((AMfloat)image->height * userToSurfaceDesc->userToSurface->a[1][1]);

	AM_ASSERT(surface);
	AM_ASSERT(userToSurfaceDesc);
	AM_ASSERT(image);

	// image is less than a pixel wide
	if (dstWidth == 0 || dstHeight == 0)
		return AM_TRUE;
	
	// in case of identity scale (i.e. scale == 1) use the faster pipeline
	if (dstWidth == image->width && dstHeight == image->height) {
		amSreImageGlyphBlit(context, surface, userToSurfaceDesc, image, imageBox);
		return AM_TRUE;
	}
	else
	if (dstWidth < 0 || dstHeight < 0 || dstWidth >= image->width || dstHeight >= image->height)
		return amSreImageDraw(context, surface, userToSurfaceDesc, image);

	// extract image quality to use, and take care of child images
	childX = (AMuint32)image->x;
	childY = (AMuint32)image->y;
	stride = (AMuint32)image->root->width;
	src32 = (const AMuint32 *)image->pixels;
	dst32 = (AMuint32 *)amSrfPixelsGet(surface);
	imageQuality = amImageQualityClamp(context->imageQuality, image->allowedQuality);

	// update dirty regions
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
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, imageBox->minPoint.x)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, imageBox->minPoint.y)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, imageBox->maxPoint.x - imageBox->minPoint.x)
			AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, imageBox->maxPoint.y - imageBox->minPoint.y)
		}
		else
			surface->dirtyRegionsOverflow = AM_TRUE;
	}
	// calculate the steps inside the image
	srcStepX = (((AMuint32)image->width) << 16) / (AMuint32)dstWidth;
	srcStepY = (((AMuint32)image->height) << 16) / (AMuint32)dstHeight;
	// calculate destination position, inside the drawing surface
	if (context->renderingQuality != VG_RENDERING_QUALITY_NONANTIALIASED) {
		dstX = (AMuint32)(userToSurfaceDesc->userToSurface->a[0][2] * 65536.0f);
		dstFru = (dstX >> 8) & 0xFF;
		dstX >>= 16;
	}
	else {
		dstX = (AMuint32)(userToSurfaceDesc->userToSurface->a[0][2] + 0.5f);
		dstFru = 0;
	}
	// first interested scanline in the drawing surface
	dstY = (AMuint32)(userToSurfaceDesc->userToSurface->a[1][2] + 0.5f);
	dst32 = &dst32[(amSrfHeightGet(surface) - dstY - 1) * amSrfWidthGet(surface)];

	if (imageQuality != VG_IMAGE_QUALITY_NONANTIALIASED) {

		#define CALC_BILINEAR_PIXEL(_res) { \
			AMPixel32 p00, p10, p01, p11; \
			AMuint32 u = srcX >> 16; \
			AMuint32 fru = (srcX >> 8) & 0xFF; \
			AMuint32 srcOfs = srcOfsY + u; \
			AM_ASSERT(u >= 0 && u < (AMuint32)image->width); \
			AM_ASSERT((u + 1) >= 0 && (u + 1) < (AMuint32)image->width); \
			AM_ASSERT(v >= 0 && v < (AMuint32)image->height); \
			AM_ASSERT((v + 1) >= 0 && (v + 1) < (AMuint32)image->height); \
			p00.value = src32[srcOfs]; \
			p10.value = src32[srcOfs + 1]; \
			p01.value = src32[srcOfs + stride]; \
			p11.value = src32[srcOfs + stride + 1]; \
			p00.value = amPxlLerp(fru, p00.value, p10.value); \
			p01.value = amPxlLerp(fru, p01.value, p11.value); \
			_res = amPxlLerp(frv, p00.value, p01.value); \
		}

		srcY = (srcStepY >> 1) - 0x8000;

		if (dstFru) {
			// 255 - dstFru
			AMuint32 invDstFru = dstFru ^ 0xFF;

			for (i = (AMuint32)dstHeight; i != 0; --i) {

				AMPixel32 old;
				AMuint32 cov;
				AMuint32 v = srcY >> 16;
				AMuint32 frv = (srcY >> 8) & 0xFF;
				AMuint32 srcOfsY = (v + childY) * stride + childX;

				srcX = (srcStepX >> 1) - 0x8000;
				CALC_BILINEAR_PIXEL(old.value)
				// write the first pixel
				MULT_DIV_255(cov, invDstFru, old.c.a)
				AM_ASSERT(dstX >= 0 && dstX < (AMuint32)amSrfWidthGet(surface));
				dst32[dstX] = old.value + amPxlScl255(cov ^ 0xFF, dst32[dstX]);
				srcX += srcStepX;

				for (j = 1; j < (AMuint32)dstWidth; ++j) {

					AMPixel32 cur;
					// calculate the new pixel
					CALC_BILINEAR_PIXEL(cur.value)
					// skip writing if both pixels are fully transparent
					if (old.c.a || cur.c.a) {

						AMPixel32 tmp;
						AM_ASSERT((dstX + j) >= 0 && (dstX + j) < (AMuint32)amSrfWidthGet(surface));
						// lerp the two pixels and write the result
						tmp.value = amPxlLerp(invDstFru, old.value, cur.value);
						dst32[dstX + j] = tmp.value + amPxlScl255(tmp.c.a ^ 0xFF, dst32[dstX + j]);
					}

					old.value = cur.value;
					// next pixel
					srcX += srcStepX;
				}

				// last pixel of the current scanline
				MULT_DIV_255(cov, dstFru, old.c.a)
				AM_ASSERT((dstX + dstWidth) >= 0 && (dstX + dstWidth) < (AMuint32)amSrfWidthGet(surface));
				dst32[dstX + dstWidth] = old.value + amPxlScl255(cov ^ 0xFF, dst32[dstX + dstWidth]);
				// next scanlines
				srcY += srcStepY;
				dst32 -= amSrfWidthGet(surface);
			}
		}
		else {
			for (i = (AMuint32)dstHeight; i != 0; --i) {

				AMuint32 v = srcY >> 16;
				AMuint32 frv = (srcY >> 8) & 0xFF;
				AMuint32 srcOfsY = (v + childY) * stride + childX;

				srcX = (srcStepX >> 1) - 0x8000;
				for (j = 0; j < (AMuint32)dstWidth; ++j) {

					AMPixel32 cur;
					// calculate the new pixel
					CALC_BILINEAR_PIXEL(cur.value)
					AM_ASSERT((dstX + j) >= 0 && (dstX + j) < (AMuint32)amSrfWidthGet(surface));
					// skip writing if pixel is fully transparent
					if (cur.c.a) 
						dst32[dstX + j] = cur.value + amPxlScl255(cur.c.a ^ 0xFF, dst32[dstX + j]);
					// next pixel
					srcX += srcStepX;
				}
				// next scanlines
				srcY += srcStepY;
				dst32 -= amSrfWidthGet(surface);
			}
		}
		#undef CALC_BILINEAR_PIXEL
	}
	else {
		
		#define CALC_PIXEL(_res) { \
			AMuint32 u = srcX >> 16; \
			AMuint32 srcOfs = srcOfsY + u; \
			AM_ASSERT(u >= 0 && u < (AMuint32)image->width); \
			AM_ASSERT(v >= 0 && v < (AMuint32)image->height); \
			_res = src32[srcOfs]; \
		}

		srcY = srcStepY >> 1;

		if (dstFru) {
			// 255 - dstFru
			AMuint32 invDstFru = dstFru ^ 0xFF;

			for (i = (AMuint32)dstHeight; i != 0; --i) {

				AMPixel32 old;
				AMuint32 cov;
				AMuint32 v = srcY >> 16;
				AMuint32 srcOfsY = (v + childY) * stride + childX;

				srcX = srcStepX >> 1;
				CALC_PIXEL(old.value)
				// write the first pixel
				MULT_DIV_255(cov, invDstFru, old.c.a)
				AM_ASSERT(dstX >= 0 && dstX < (AMuint32)amSrfWidthGet(surface));
				dst32[dstX] = old.value + amPxlScl255(cov ^ 0xFF, dst32[dstX]);
				srcX += srcStepX;

				for (j = 1; j < (AMuint32)dstWidth; ++j) {

					AMPixel32 cur;
					// calculate the new pixel
					CALC_PIXEL(cur.value)
					// skip writing if both pixels are fully transparent	
					if (old.c.a || cur.c.a) {

						AMPixel32 tmp;
						AM_ASSERT((dstX + j) >= 0 && (dstX + j) < (AMuint32)amSrfWidthGet(surface));
						// lerp the two pixels and write the result
						tmp.value = amPxlLerp(invDstFru, old.value, cur.value);
						dst32[dstX + j] = tmp.value + amPxlScl255(tmp.c.a ^ 0xFF, dst32[dstX + j]);
					}
					// next pixel
					old.value = cur.value;
					srcX += srcStepX;
				}
				// last pixel of the current scanline
				MULT_DIV_255(cov, dstFru, old.c.a)
				AM_ASSERT((dstX + dstWidth) >= 0 && (dstX + dstWidth) < (AMuint32)amSrfWidthGet(surface));
				dst32[dstX + dstWidth] = old.value + amPxlScl255(cov ^ 0xFF, dst32[dstX + dstWidth]);
				// next scanlines
				srcY += srcStepY;
				dst32 -= amSrfWidthGet(surface);
			}
		}
		else {
			for (i = (AMuint32)dstHeight; i != 0; --i) {

				AMuint32 v = srcY >> 16;
				AMuint32 srcOfsY = (v + childY) * stride + childX;

				srcX = srcStepX >> 1;
				for (j = 0; j < (AMuint32)dstWidth; ++j) {

					AMPixel32 cur;
					// calculate the new pixel
					CALC_PIXEL(cur.value)
					AM_ASSERT((dstX + j) >= 0 && (dstX + j) < (AMuint32)amSrfWidthGet(surface));
					// skip writing if pixel is fully transparent
					if (cur.c.a) 
						dst32[dstX + j] = cur.value + amPxlScl255(cur.c.a ^ 0xFF, dst32[dstX + j]);
					// next pixel
					srcX += srcStepX;
				}
				// next scanlines
				srcY += srcStepY;
				dst32 -= amSrfWidthGet(surface);
			}
		}

		#undef CALC_PIXEL
	}

	return AM_TRUE;
}

AMbool amSreImageGlyphDraw(AMContext *context,
						   AMDrawingSurface *surface,
						   AMUserToSurfaceDesc *userToSurfaceDesc,
						   AMImage *image) {

	AMAABox2i srfSpaceBox;
	AMAABox2f tmpBox;
	AMfloat x = userToSurfaceDesc->userToSurface->a[0][2];
	AMfloat y = userToSurfaceDesc->userToSurface->a[1][2];
	AMfloat sx = userToSurfaceDesc->userToSurface->a[0][0];
	AMfloat sy = userToSurfaceDesc->userToSurface->a[1][1];

	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(image);
	AM_ASSERT(image->width > 0 && image->height > 0);
	AM_ASSERT(image->format == amSrfRealFormatGet(surface));
	AM_ASSERT(context->imageMode == VG_DRAW_IMAGE_NORMAL);
	AM_ASSERT(context->masking == VG_FALSE);
	AM_ASSERT(context->colorTransform == VG_FALSE || context->colorTransformHash == AM_COLOR_TRANSFORM_IDENTITY_HASH);
	AM_ASSERT(context->fillBlendMode == VG_BLEND_SRC_OVER);

	// align drawing surface space box to its nearest integer pixel
	tmpBox.minPoint.x = amFloorf(x);
	tmpBox.minPoint.y = amFloorf(y);
	tmpBox.maxPoint.x = amCeilf(x + (AMfloat)image->width * sx);
	tmpBox.maxPoint.y = amCeilf(y + (AMfloat)image->height * sy);
	AM_SRFSPACE_CORNER_CLAMP(srfSpaceBox.minPoint, tmpBox.minPoint)
	AM_SRFSPACE_CORNER_CLAMP(srfSpaceBox.maxPoint, tmpBox.maxPoint)

	// update scissoring
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

	if (context->scissoring == VG_TRUE) {

		AMuint32 i;

		for (i = 0; i < context->splitScissorRects.size; ++i) {
			// extract and set the i-th clipping rectangle
			AMAABox2i clipBox;
			AMint32 x0 = context->splitScissorRects.data[i].bottomLeft.p.x;
			AMint32 y0 = context->splitScissorRects.data[i].bottomLeft.p.y;
			AMint32 x1 = context->splitScissorRects.data[i].topRight.p.x;
			AMint32 y1 = context->splitScissorRects.data[i].topRight.p.y;

			AM_VECT2_SET(&clipBox.minPoint, x0, y0)
			AM_VECT2_SET(&clipBox.maxPoint, x1, y1)

			if (amAABox2iContain(&clipBox, &srfSpaceBox))
				return amSreImageGlyphStretch(context, surface, userToSurfaceDesc, image, &srfSpaceBox);
			else
			if (amAABox2iOverlap(&clipBox, &srfSpaceBox))
				return amSreImageDraw(context, surface, userToSurfaceDesc, image);
		}
	}
	else {
		AMAABox2i clipBox;

		// set a clipBox equal to the whole drawing surface
		AM_VECT2_SET(&clipBox.minPoint, 0, 0)
		AM_VECT2_SET(&clipBox.maxPoint, amSrfWidthGet(surface), amSrfHeightGet(surface))

		if (amAABox2iContain(&clipBox, &srfSpaceBox))
			return amSreImageGlyphStretch(context, surface, userToSurfaceDesc, image, &srfSpaceBox);
		else
		if (amAABox2iOverlap(&clipBox, &srfSpaceBox))
			return amSreImageDraw(context, surface, userToSurfaceDesc, image);
	}

	return AM_TRUE;
}

#endif

#endif

#if defined (RIM_VG_SRC)
#pragma pop
#endif
