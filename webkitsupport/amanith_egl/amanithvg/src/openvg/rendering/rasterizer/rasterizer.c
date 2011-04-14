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
	\file rasterizer.c
	\brief Polygon rasterization routines, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "rasterizer.h"
#include "fillers.h"
#include "vgpaint.h"
#include "vg_priv.h"

AMbool amRasDrawBetter(AMDrawingSurface *surface,
					   AMRasterizer *rasterizer,
					   AMPaintGen *paintGen,
					   AMScanlineFillerFunction filler,
					   const VGFillRule fillRule,
					   const AMAABox2i *clipBox,
					   const AMbool trackScanlines);

AMbool amRasDrawFaster(AMDrawingSurface *surface,
					   AMRasterizer *rasterizer,
					   AMPaintGen *paintGen,
					   AMScanlineFillerFunction filler,
					   const VGFillRule fillRule,
					   const AMbool trackScanlines);

AMbool amRasDrawNoAA(AMDrawingSurface *surface,
					 AMRasterizer *rasterizer,
					 AMPaintGen *paintGen,
					 AMScanlineFillerFunction filler,
					 const VGFillRule fillRule,
					 const AMbool trackScanlines);

void amRasDynResourcesInit(AMRasterizer *rasterizer) {

	AM_ASSERT(rasterizer);

	rasterizer->coverageLineDeltas = NULL;
	AM_DYNARRAY_PREINIT(rasterizer->vertices)
	AM_DYNARRAY_PREINIT(rasterizer->contourPts)
	AM_DYNARRAY_PREINIT(rasterizer->eventsQueue)
	AM_DYNARRAY_PREINIT(rasterizer->eventsQueueTmp)
	AM_DYNARRAY_PREINIT(rasterizer->intersectionNumerators)
	AM_DYNARRAY_PREINIT(rasterizer->msortHelpers)
	AM_DYNARRAY_PREINIT(rasterizer->qsortHelpers)
	AM_DYNARRAY_PREINIT(rasterizer->activeEdgeSweepDists)
	AM_DYNARRAY_PREINIT(rasterizer->sortGEL32)
	AM_DYNARRAY_PREINIT(rasterizer->sortGEL64)
	AM_DYNARRAY_PREINIT(rasterizer->gel)
	AM_DYNARRAY_PREINIT(rasterizer->gelTmp)
	AM_DYNARRAY_PREINIT(rasterizer->ael)
#if (AM_OPENVG_VERSION >= 110)
	AM_DYNARRAY_PREINIT(rasterizer->globalScanlineList)
#endif
}

void amRasDynResourcesDestroy(AMRasterizer *rasterizer) {

	AM_ASSERT(rasterizer);

	if (rasterizer->coverageLineDeltas) {
		amFree(rasterizer->coverageLineDeltas);
		rasterizer->coverageLineDeltas = NULL;
	}

	AM_DYNARRAY_DESTROY(rasterizer->vertices)
	AM_DYNARRAY_DESTROY(rasterizer->contourPts)
	AM_DYNARRAY_DESTROY(rasterizer->eventsQueue)
	AM_DYNARRAY_DESTROY(rasterizer->eventsQueueTmp)
	AM_DYNARRAY_DESTROY(rasterizer->msortHelpers)
	AM_DYNARRAY_DESTROY(rasterizer->qsortHelpers)
	AM_DYNARRAY_DESTROY(rasterizer->intersectionNumerators)
	AM_DYNARRAY_DESTROY(rasterizer->activeEdgeSweepDists)
	AM_DYNARRAY_DESTROY(rasterizer->sortGEL32)
	AM_DYNARRAY_DESTROY(rasterizer->sortGEL64)
	AM_DYNARRAY_DESTROY(rasterizer->gel)
	AM_DYNARRAY_DESTROY(rasterizer->gelTmp)
	AM_DYNARRAY_DESTROY(rasterizer->ael)
#if (AM_OPENVG_VERSION >= 110)
	AM_DYNARRAY_DESTROY(rasterizer->globalScanlineList)
#endif

}

/*!
	\brief Rasterizer constructor, it allocates a new AMRasterizer structure, and initializes all
	common data (events queue, active edge list, global edge list and so on).
	\param rasterizerPtr input pointer to the AMRasterizer structure to be allocated and initialized.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amRasCreate(AMRasterizer **rasterizerPtr) {

	AMRasterizer *newRasterizer;

	AM_ASSERT(rasterizerPtr);

	newRasterizer = (AMRasterizer *)amMalloc(sizeof(AMRasterizer));
	if (!newRasterizer) {
		rasterizerPtr = NULL;
		return AM_FALSE;
	}

	amRasDynResourcesInit(newRasterizer);

	newRasterizer->coverageLineDeltas = (AMint32 *)amMalloc((AM_SURFACE_MAX_DIMENSION + 2) * sizeof(AMint32));
	if (!newRasterizer->coverageLineDeltas) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}
	// clear coverage line
	amMemset32((AMuint32 *)newRasterizer->coverageLineDeltas, 0, AM_SURFACE_MAX_DIMENSION + 2);

	AM_DYNARRAY_INIT(newRasterizer->vertices, AMSrfSpaceVertex)
	if (newRasterizer->vertices.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

	AM_DYNARRAY_INIT(newRasterizer->contourPts, AMint32)
	if (newRasterizer->contourPts.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

	AM_DYNARRAY_INIT(newRasterizer->eventsQueue, AMSrfSpaceEvent)
	if (newRasterizer->eventsQueue.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

	AM_DYNARRAY_INIT(newRasterizer->eventsQueueTmp, AMSrfSpaceEvent)
	if (newRasterizer->eventsQueueTmp.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

	AM_DYNARRAY_INIT(newRasterizer->intersectionNumerators, AMSrfSpaceIntersectionNums)
	if (newRasterizer->intersectionNumerators.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

	AM_DYNARRAY_INIT(newRasterizer->msortHelpers, AMSrfSpaceMSortHelper)
	if (newRasterizer->msortHelpers.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

	AM_DYNARRAY_INIT(newRasterizer->qsortHelpers, AMSrfSpaceQSortHelper)
	if (newRasterizer->qsortHelpers.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

	AM_DYNARRAY_INIT(newRasterizer->activeEdgeSweepDists, AMEdgeSweepDistance)
	if (newRasterizer->activeEdgeSweepDists.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

	AM_DYNARRAY_INIT(newRasterizer->sortGEL32, AMSortGEL32)
	if (newRasterizer->sortGEL32.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

	AM_DYNARRAY_INIT(newRasterizer->sortGEL64, AMSortGEL64)
	if (newRasterizer->sortGEL64.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

	AM_DYNARRAY_INIT(newRasterizer->gel, AMSrfSpaceEdge)
	if (newRasterizer->gel.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

	AM_DYNARRAY_INIT(newRasterizer->gelTmp, AMSrfSpaceEdge)
	if (newRasterizer->gelTmp.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

	AM_DYNARRAY_INIT(newRasterizer->ael, AMSrfSpaceEdgePtr)
	if (newRasterizer->ael.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}

#if (AM_OPENVG_VERSION >= 110)
	AM_DYNARRAY_INIT(newRasterizer->globalScanlineList, AMint32)
	if (newRasterizer->globalScanlineList.error) {
		amRasDynResourcesDestroy(newRasterizer);
		return AM_FALSE;
	}
#endif

	newRasterizer->boxMaxY = 0;
	*rasterizerPtr = newRasterizer;
	return AM_TRUE;
}

/*!
	\brief Rasterizer destructor. It frees all used memory used by the specified rasterizer.
	\param rasterizer rasterizer structure to be destroyed.
*/
void amRasDestroy(AMRasterizer *rasterizer) {

	AM_ASSERT(rasterizer);

	amRasDynResourcesDestroy(rasterizer);
	amFree(rasterizer);
}

void amRasMemoryRetrieve(AMRasterizer *rasterizer,
						 const AMbool maxMemoryRetrieval) {

	AM_ASSERT(rasterizer);

	if (maxMemoryRetrieval) {
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->vertices, AMSrfSpaceVertex)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->contourPts, AMint32)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->eventsQueue, AMSrfSpaceEvent)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->eventsQueueTmp, AMSrfSpaceEvent)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->intersectionNumerators, AMSrfSpaceIntersectionNums)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->msortHelpers, AMSrfSpaceMSortHelper)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->qsortHelpers, AMSrfSpaceQSortHelper)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->gelTmp, AMSrfSpaceEdge)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->activeEdgeSweepDists, AMEdgeSweepDistance)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->sortGEL32, AMSortGEL32)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->sortGEL64, AMSortGEL64)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->gel, AMSrfSpaceEdge)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->ael, AMSrfSpaceEdgePtr)
	#if (AM_OPENVG_VERSION >= 110)
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->globalScanlineList, AMint32)
	#endif
	}
	else {
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->vertices, AMSrfSpaceVertex)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->contourPts, AMint32)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->eventsQueue, AMSrfSpaceEvent)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->eventsQueueTmp, AMSrfSpaceEvent)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->intersectionNumerators, AMSrfSpaceIntersectionNums)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->msortHelpers, AMSrfSpaceMSortHelper)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->qsortHelpers, AMSrfSpaceQSortHelper)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->gelTmp, AMSrfSpaceEdge)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->activeEdgeSweepDists, AMEdgeSweepDistance)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->sortGEL32, AMSortGEL32)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->sortGEL64, AMSortGEL64)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->gel, AMSrfSpaceEdge)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->ael, AMSrfSpaceEdgePtr)
	#if (AM_OPENVG_VERSION >= 110)
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(rasterizer->globalScanlineList, AMint32)
	#endif
	}
}

#if defined(AM_SRE)

/*!
	\brief Draw a set of closed polygons.
	\param _context pointer to a AMContext structure, containing the atan2 table.
	\param _surface pointer to a AMDrawingSurface structure, specifying the drawing surface where rasterization will be performed to.
	\param rasterizer rasterizer containing the array of coverage deltas.
	\param points input pointer to an array that contains drawing surface space vertices.
	\param ptsPerContour input pointer to a AMInt32DynArray structure that contains the number of drawing surface space vertices for each sub-contour.
	\param _paintDesc input pointer to a AMPaintDesc structure that describes the paint to use.
	\param clipBox drawing surface space clip box.
	\param doClipping if AM_TRUE clip is performed, if AM_FALSE clip is not performed.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
*/
AMbool amRasPolygonsDraw(void *_context,
						 void *_surface,
						 AMRasterizer *rasterizer,
						 const AM_RAS_INPUT_VERTICES_ARRAY_TYPE *points,
						 const AMInt32DynArray *ptsPerContour,
						 const void *_paintDesc,
						 const AMAABox2i *clipBox,
						 const AMbool doClipping) {

	AMContext *context = (AMContext *)_context;
	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	const AMPaintDesc *paintDesc = (const AMPaintDesc *)_paintDesc;
	const AMUserToSurfaceDesc *userToSurfaceDesc = paintDesc->userToSurfaceDesc;
	AMScanlineFillerFunction filler = NULL;
	const AM_RAS_MATRIX_TYPE *actualMatrix;
#if !defined(AM_FIXED_POINT_PIPELINE)
	AM_RAS_MATRIX_TYPE m;
#endif
#if defined(VG_MZT_statistics)
	
	#define RAS_POLYGONS_RETURN(_res) \
		endMS = amTimeGet(); \
		context->statisticsInfo.rasterizerTotalTimeMS += (endMS - startMS); \
		return(_res);

	AMuint32 startMS, endMS;
#else
	#define RAS_POLYGONS_RETURN(_res) return(_res);
#endif

	AM_ASSERT(points);
	AM_ASSERT(ptsPerContour);
	AM_ASSERT(paintDesc);
	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(rasterizer);

#if defined(VG_MZT_statistics)
	startMS = amTimeGet();
#endif

	if (ptsPerContour->size == 0 || points->size == 0) {
		RAS_POLYGONS_RETURN(AM_TRUE)
	}

	if (rasterizer->vertices.capacity < points->size) {
		AM_DYNARRAY_CLEAR_RESERVE(rasterizer->vertices, AMSrfSpaceVertex, points->size)
		if (rasterizer->vertices.error) {
			rasterizer->vertices.error = AM_DYNARRAY_NO_ERROR;
			RAS_POLYGONS_RETURN(AM_FALSE)
		}
	}

	if (rasterizer->contourPts.capacity < ptsPerContour->size) {
		AM_DYNARRAY_CLEAR_RESERVE(rasterizer->contourPts, AMint32, ptsPerContour->size)
		if (rasterizer->contourPts.error) {
			rasterizer->contourPts.error = AM_DYNARRAY_NO_ERROR;
			RAS_POLYGONS_RETURN(AM_FALSE)
		}
	}

#if defined(AM_FIXED_POINT_PIPELINE)
	actualMatrix = &userToSurfaceDesc->userToSurfacex;
#else
	if (doClipping)
		actualMatrix = userToSurfaceDesc->userToSurface;
	else {
		m.a[0][0] = userToSurfaceDesc->userToSurface->a[0][0] * AM_RAS_VERTEX_FTOX;
		m.a[0][1] = userToSurfaceDesc->userToSurface->a[0][1] * AM_RAS_VERTEX_FTOX;
		m.a[0][2] = userToSurfaceDesc->userToSurface->a[0][2] * AM_RAS_VERTEX_FTOX;
		m.a[1][0] = userToSurfaceDesc->userToSurface->a[1][0] * AM_RAS_VERTEX_FTOX;
		m.a[1][1] = userToSurfaceDesc->userToSurface->a[1][1] * AM_RAS_VERTEX_FTOX;
		m.a[1][2] = userToSurfaceDesc->userToSurface->a[1][2] * AM_RAS_VERTEX_FTOX;
		m.a[2][0] = 0.0f;
		m.a[2][1] = 0.0f;
		m.a[2][2] = 1.0f;
		actualMatrix = &m;
	}
#endif

	// clip polygon against the clipBox
	if (doClipping) {
		if (!amRasPolygonTransformAndClip(&rasterizer->vertices, &rasterizer->contourPts, points, ptsPerContour, actualMatrix, clipBox)) {
			RAS_POLYGONS_RETURN(AM_FALSE)
		}
	#if defined(_DEBUG)
		{
			AMuint32 i;
			// consistency check: transformed and clipped points must be inside the surface
			for (i = 0; i < rasterizer->vertices.size; ++i) {
				AM_ASSERT((rasterizer->vertices.data[i].p.x >> AM_RAS_FIXED_PRECISION) <= amSrfWidthGet(surface));
				AM_ASSERT((rasterizer->vertices.data[i].p.y >> AM_RAS_FIXED_PRECISION) <= amSrfHeightGet(surface));
			}
		}
	#endif
	}
	else {
		AMuint32 i;
		AM_RAS_INPUT_VERTEX_TYPE *src = points->data;
		AMSrfSpaceVertex *dst = rasterizer->vertices.data;
		
		for (i = (AMuint32)points->size; i != 0; --i) {

			AM_RAS_INPUT_VERTEX_TYPE tmpDst;

			amRasVertexTransform(&tmpDst, src, actualMatrix);
			dst->p.x = (AMuint16)tmpDst.x;
			dst->p.y = (AMuint16)tmpDst.y;
			AM_ASSERT((dst->p.x >> AM_RAS_FIXED_PRECISION) <= amSrfWidthGet(surface));
			AM_ASSERT((dst->p.y >> AM_RAS_FIXED_PRECISION) <= amSrfHeightGet(surface));
			src++;
			dst++;
		}

		for (i = 0; i < (AMuint32)ptsPerContour->size; ++i)
			rasterizer->contourPts.data[i] = ptsPerContour->data[i];

		rasterizer->vertices.size = points->size;
		rasterizer->contourPts.size = ptsPerContour->size;
	}

	// if some edges are inside the clipBox, the do the rasterization process
	if (rasterizer->vertices.size > 0 && rasterizer->contourPts.size > 0) {

		AMPaintGen paintGen;
		AMbool res;

		// initialize paint generator (taking care of current color transform)
		amPaintGenPathInit(&paintGen, context, surface, rasterizer, paintDesc,
					#if (AM_OPENVG_VERSION >= 110)
						context->colorTransformHash
					#else
						AM_COLOR_TRANSFORM_IDENTITY_HASH
					#endif
						);
		AM_ASSERT(paintDesc == paintGen.paintDesc);
		// select the filler
		filler = amFilPathSelect(context, paintGen.paintDesc);
		// do the real rasterization
		switch (paintDesc->renderingQuality) {
			case VG_RENDERING_QUALITY_NONANTIALIASED:
				res = amRasDrawNoAA(surface, rasterizer, &paintGen, filler, paintDesc->fillRule, AM_FALSE);
				break;
			case VG_RENDERING_QUALITY_FASTER:
				res = amRasDrawFaster(surface, rasterizer, &paintGen, filler, paintDesc->fillRule, AM_FALSE);
				break;
			default:
				res = amRasDrawBetter(surface, rasterizer, &paintGen, filler, paintDesc->fillRule, clipBox, AM_FALSE);
				break;
		}
		RAS_POLYGONS_RETURN(res)
	}
	else {
		RAS_POLYGONS_RETURN(AM_TRUE)
	}

	#undef RAS_POLYGONS_RETURN
}

/*!
	\brief Draw an image or fill a rectangular area.
	\param _context pointer to a AMContext structure, containing the atan2 table.
	\param _surface pointer to a AMDrawingSurface structure, specifying the drawing surface where rasterization will be performed to.
	\param rasterizer rasterizer containing the array of coverage deltas.
	\param p0 first image/area corner, in drawing surface space.
	\param p1 second image/area corner, in drawing surface space.
	\param p2 third image/area corner, in drawing surface space.
	\param p3 fourth image/area corner, in drawing surface space.
	\param _paintDesc input pointer to a AMPaintDesc structure that describes the paint to use.
	\param clear if AM_TRUE fill the rectangular area, if AM_FALSE draw the image.
	\param clipBox drawing surface space clip box.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
*/
AMbool amRasImageDraw(const void *_context,
					  void *_surface,
					  AMRasterizer *rasterizer,
					  const AMVect2f *p0,
					  const AMVect2f *p1,
					  const AMVect2f *p2,
					  const AMVect2f *p3,
					  const void *_paintDesc,
					  const AMbool clear,
					  const AMAABox2i *clipBox) {

	const AMContext *context = (const AMContext *)_context;
	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	const AMPaintDesc *paintDesc = (const AMPaintDesc *)_paintDesc;
	AMScanlineFillerFunction filler = NULL;
	AMPaintGen paintGen;
	AM_RAS_MATRIX_TYPE matrix;
	AM_RAS_INPUT_VERTEX_TYPE imgPts[4];
	AM_RAS_INPUT_VERTICES_ARRAY_TYPE imgPointsArray;
	AMint32 imgPtsPerContour;
	AMInt32DynArray imgPtsPerContourArray;
	AMbool res;

	AM_ASSERT(paintDesc);
	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(rasterizer);

	imgPointsArray.size = imgPointsArray.capacity = 4;
	imgPointsArray.data = &imgPts[0];
	imgPtsPerContourArray.size = imgPtsPerContourArray.capacity = 1;
	imgPtsPerContourArray.data = &imgPtsPerContour;
	imgPtsPerContour = 4;

#if defined(AM_FIXED_POINT_PIPELINE)
	AM_VECT2_SET(&imgPts[0], amFloatToFixed1616(p0->x), amFloatToFixed1616(p0->y));
	AM_VECT2_SET(&imgPts[1], amFloatToFixed1616(p1->x), amFloatToFixed1616(p1->y));
	AM_VECT2_SET(&imgPts[2], amFloatToFixed1616(p2->x), amFloatToFixed1616(p2->y));
	AM_VECT2_SET(&imgPts[3], amFloatToFixed1616(p3->x), amFloatToFixed1616(p3->y));
	AM_MATRIX33_SET(&matrix, 1 << 15, 0, 0, 0, 1 << 15, 0, 0, 0, 1 << 15)
#else
	imgPts[0] = *p0;
	imgPts[1] = *p1;
	imgPts[2] = *p2;
	imgPts[3] = *p3;
	AM_MATRIX33_IDENTITY(&matrix)
#endif

	// always clip image outline polygon against the clipBox
	if (!amRasPolygonTransformAndClip(&rasterizer->vertices, &rasterizer->contourPts, &imgPointsArray, &imgPtsPerContourArray, &matrix, clipBox))
		return AM_FALSE;

	if (clear) {
		// initialize paint generator
		amPaintGenPathInit(&paintGen, context, surface, rasterizer, paintDesc,
						#if (AM_OPENVG_VERSION >= 110)
							paintDesc->colorTransform ? context->colorTransformHash : AM_COLOR_TRANSFORM_IDENTITY_HASH
						#else
							AM_COLOR_TRANSFORM_IDENTITY_HASH
						#endif
							);
		// select the filler
		filler = amFilPathSelect(context, paintGen.paintDesc);
	}
	else {
		AM_ASSERT(paintDesc->image);

		amPaintGenImageInit(&paintGen, context, surface, rasterizer, paintDesc);
		// select the filler
		filler = amFilImageSelect(context, paintGen.paintDesc);
	}
	// do the rasterization
	switch (paintDesc->renderingQuality) {
		case VG_RENDERING_QUALITY_NONANTIALIASED:
			res = amRasDrawNoAA(surface, rasterizer, &paintGen, filler, paintDesc->fillRule, AM_FALSE);
			break;
		case VG_RENDERING_QUALITY_FASTER:
			res = amRasDrawFaster(surface, rasterizer, &paintGen, filler, paintDesc->fillRule, AM_FALSE);
			break;
		default:
			res = amRasDrawBetter(surface, rasterizer, &paintGen, filler, paintDesc->fillRule, clipBox, AM_FALSE);
			break;
	}
	return res;
}

#endif

#if (AM_OPENVG_VERSION >= 110)
void amFilMask_Set(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 ofs0;
	AMuint8 *scrPixels;
	AMint32 *covLine;
	AMuint32 i;
	AMint32 cov, tmp;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(paintGen->clipBox);
	AM_ASSERT(surface->alphaMaskPixels);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(paintGen->clipBox);

	x0 = paintGen->clipBox->minPoint.x;
	x1 = paintGen->clipBox->maxPoint.x;
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 <= amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 <= amSrfWidthGet(surface));

	ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	scrPixels = surface->alphaMaskPixels + ofs0;
	covLine = paintGen->coverageLineDeltas + x0;
	i = x1 - x0;

	cov = (*covLine);
	for (;;) {
		*covLine = 0;
		while (cov == AM_RAS_MAX_COVERAGE) {
			*scrPixels++ = 0xFF;
			covLine++;
			tmp = *covLine;
			if (--i == 0) {
				covLine[0] = 0;
				return;
			}
			cov += tmp;
		}
		*covLine = 0;
		while (cov == 0) {
			*scrPixels++ = 0;
			covLine++;
			tmp = *covLine;
			if (--i == 0) {
				covLine[0] = 0;
				return;
			}
			cov += tmp;
		}
		*covLine = 0;
		*scrPixels++ = (AMuint8)(((cov >> AM_RAS_COVERAGE_PRECISION) * 255) >> 8);
		covLine++;
		tmp = *covLine;
		if (--i == 0) {
			covLine[0] = 0;
			return;
		}
		cov += tmp;
	}
}

void amFilMask_Union(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	AMuint8 *scrPixels = surface->alphaMaskPixels + ofs0;
	AMint32 *covLine = paintGen->coverageLineDeltas + x0;
	AMuint32 i = x1 - x0 + 1;
	AMint32 cov, tmp;
	AMuint32 k0, k1, r;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(surface->alphaMaskPixels);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 < amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 < amSrfWidthGet(surface));

	(void)paintGen;

	cov = (*covLine);
	for (;;) {
		*covLine = 0;
		while (cov == AM_RAS_MAX_COVERAGE) {
			*scrPixels++ = 0xFF;
			covLine++;
			tmp = *covLine;
			if (--i == 0) {
				covLine[0] = 0;
				return;
			}
			cov += tmp;
		}
		*covLine = 0;
		while (cov == 0) {
			scrPixels++;
			covLine++;
			tmp = *covLine;
			if (--i == 0) {
				covLine[0] = 0;
				return;
			}
			cov += tmp;
		}
		*covLine = 0;

		k0 = 255 - (AMuint32)(((cov >> AM_RAS_COVERAGE_PRECISION) * 255) >> 8);
		k1 = 255 - *scrPixels;
		MULT_DIV_255(r, k0, k1)
		*scrPixels++ = 255 - r;
		
		covLine++;
		tmp = *covLine;
		if (--i == 0) {
			covLine[0] = 0;
			return;
		}
		cov += tmp;
	}
}

void amFilMask_Intersect(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 ofs0;
	AMuint8 *scrPixels;
	AMint32 *covLine;
	AMuint32 i, k0, k1, r;
	AMint32 cov, tmp;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(surface->alphaMaskPixels);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(paintGen->clipBox);

	x0 = paintGen->clipBox->minPoint.x;
	x1 = paintGen->clipBox->maxPoint.x;
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 <= amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 <= amSrfWidthGet(surface));

	ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	scrPixels = surface->alphaMaskPixels + ofs0;
	covLine = paintGen->coverageLineDeltas + x0;
	i = x1 - x0;

	cov = (*covLine);
	for (;;) {
		*covLine = 0;
		while (cov == AM_RAS_MAX_COVERAGE) {
			scrPixels++;
			covLine++;
			tmp = *covLine;
			if (--i == 0) {
				covLine[0] = 0;
				return;
			}
			cov += tmp;
		}
		*covLine = 0;
		while (cov == 0) {
			*scrPixels++ = 0;
			covLine++;
			tmp = *covLine;
			if (--i == 0) {
				covLine[0] = 0;
				return;
			}
			cov += tmp;
		}
		*covLine = 0;

		k0 = (AMuint32)(((cov >> AM_RAS_COVERAGE_PRECISION) * 255) >> 8);
		k1 = (AMuint32)(*scrPixels);
		MULT_DIV_255(r, k0, k1)
		*scrPixels++ = r;

		covLine++;
		tmp = *covLine;
		if (--i == 0) {
			covLine[0] = 0;
			return;
		}
		cov += tmp;
	}
}

void amFilMask_Subtract(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	AMuint8 *scrPixels = surface->alphaMaskPixels + ofs0;
	AMint32 *covLine = paintGen->coverageLineDeltas + x0;
	AMuint32 i = x1 - x0 + 1;
	AMint32 cov, tmp;
	AMuint32 k0, k1, r;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(surface->alphaMaskPixels);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 < amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 < amSrfWidthGet(surface));

	(void)paintGen;

	cov = (*covLine);
	for (;;) {
		*covLine = 0;
		while (cov == AM_RAS_MAX_COVERAGE) {
			*scrPixels++ = 0;
			covLine++;
			tmp = *covLine;
			if (--i == 0) {
				covLine[0] = 0;
				return;
			}
			cov += tmp;
		}
		*covLine = 0;
		while (cov == 0) {
			scrPixels++;
			covLine++;
			tmp = *covLine;
			if (--i == 0) {
				covLine[0] = 0;
				return;
			}
			cov += tmp;
		}
		*covLine = 0;

		k0 = 255 - (AMuint32)(((cov >> AM_RAS_COVERAGE_PRECISION) * 255) >> 8);
		k1 = (AMuint32)(*scrPixels);
		MULT_DIV_255(r, k0, k1)
		*scrPixels++ = r;

		covLine++;
		tmp = *covLine;
		if (--i == 0) {
			covLine[0] = 0;
			return;
		}
		cov += tmp;
	}
}

void amRasPolygonsMaskClearEmptyLines(AMDrawingSurface *surface,
									  AMInt32DynArray *scanLines,
									  const AMAABox2i *clipBox) {

	AMint32 i, j = 0, y;

	AM_ASSERT(surface);
	AM_ASSERT(scanLines);

	y = scanLines->data[0];
	for (i = clipBox->maxPoint.y - 1; i >= clipBox->minPoint.y; --i) {

		if (i == y)
			// if scanline has been rasterized, skip it
			y = scanLines->data[++j];
		else {
			AMint32 ofs = (amSrfHeightGet(surface) - i - 1) * amSrfWidthGet(surface) + clipBox->minPoint.x;
			AMuint32 width = clipBox->maxPoint.x - clipBox->minPoint.x;
			// clear the scanline
			amMemset(&surface->alphaMaskPixels[ofs], 0, width);
		}
	}

	// all rasterized scan lines must be analyzed
	AM_ASSERT(j == (AMint32)scanLines->size);
}

/*!
	Draw a set of closed polygons onto the alpha mask.

	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
*/
AMbool amRasPolygonsMaskDraw(const void *_context,
							 void *_surface,
							 AMRasterizer *rasterizer,
							 const AM_RAS_INPUT_VERTICES_ARRAY_TYPE *points,
							 const AMInt32DynArray *ptsPerContour,
							 const void *_paintDesc,
							 const VGMaskOperation operation,
							 const AMAABox2i *clipBox,
							 const AMbool doClipping) {

	const AMContext *context = (const AMContext *)_context;
	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	const AMPaintDesc *paintDesc = (const AMPaintDesc *)_paintDesc;
#if defined(AM_FIXED_POINT_PIPELINE)
	const AMUserToSurfaceDesc *userToSurfaceDesc = paintDesc->userToSurfaceDesc;
#else
	AM_RAS_MATRIX_TYPE m;
#endif
	const AM_RAS_MATRIX_TYPE *actualMatrix;
	AMPaintGen paintGen;

	AM_ASSERT(points);
	AM_ASSERT(ptsPerContour);
	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(surface->alphaMaskPixels);
	AM_ASSERT(rasterizer);

	if (ptsPerContour->size == 0 || points->size == 0)
		return AM_TRUE;

	if (rasterizer->vertices.capacity < points->size) {
		AM_DYNARRAY_CLEAR_RESERVE(rasterizer->vertices, AMSrfSpaceVertex, points->size)
		if (rasterizer->vertices.error) {
			rasterizer->vertices.error = AM_DYNARRAY_NO_ERROR;
			return AM_FALSE;
		}
	}

	if (rasterizer->contourPts.capacity < ptsPerContour->size) {
		AM_DYNARRAY_CLEAR_RESERVE(rasterizer->contourPts, AMint32, ptsPerContour->size)
		if (rasterizer->contourPts.error) {
			rasterizer->contourPts.error = AM_DYNARRAY_NO_ERROR;
			return AM_FALSE;
		}
	}

#if defined(AM_FIXED_POINT_PIPELINE)
	actualMatrix = &userToSurfaceDesc->userToSurfacex;
#else
	if (doClipping)
		actualMatrix = &context->pathUserToSurface;
	else {
		m.a[0][0] = context->pathUserToSurface.a[0][0] * AM_RAS_VERTEX_FTOX;
		m.a[0][1] = context->pathUserToSurface.a[0][1] * AM_RAS_VERTEX_FTOX;
		m.a[0][2] = context->pathUserToSurface.a[0][2] * AM_RAS_VERTEX_FTOX;
		m.a[1][0] = context->pathUserToSurface.a[1][0] * AM_RAS_VERTEX_FTOX;
		m.a[1][1] = context->pathUserToSurface.a[1][1] * AM_RAS_VERTEX_FTOX;
		m.a[1][2] = context->pathUserToSurface.a[1][2] * AM_RAS_VERTEX_FTOX;
		m.a[2][0] = 0.0f;
		m.a[2][1] = 0.0f;
		m.a[2][2] = 1.0f;
		actualMatrix = &m;
	}
#endif

	// clip polygon against the clipBox
	if (doClipping) {
		if (!amRasPolygonTransformAndClip(&rasterizer->vertices, &rasterizer->contourPts, points, ptsPerContour, actualMatrix, clipBox))
			return AM_FALSE;
	}
	else {
		AMuint32 i;
		AM_RAS_INPUT_VERTEX_TYPE *src = points->data;
		AMSrfSpaceVertex *dst = rasterizer->vertices.data;
		
		for (i = (AMuint32)points->size; i != 0; --i) {

			AM_RAS_INPUT_VERTEX_TYPE tmpDst;

			amRasVertexTransform(&tmpDst, src, actualMatrix);
			dst->p.x = (AMuint16)tmpDst.x;
			dst->p.y = (AMuint16)tmpDst.y;
			src++;
			dst++;
		}

		for (i = 0; i < (AMuint32)ptsPerContour->size; ++i)
			rasterizer->contourPts.data[i] = ptsPerContour->data[i];

		rasterizer->vertices.size = points->size;
		rasterizer->contourPts.size = ptsPerContour->size;
	}

	// if some edges are inside the clipBox, the do the rasterization process
	if (rasterizer->vertices.size > 0 && rasterizer->contourPts.size > 0) {

		AMScanlineFillerFunction filler;
		AMbool trackScanlines, res;

		// select filler according to the mask operation
		switch (operation) {
			case VG_SET_MASK:
				trackScanlines = AM_TRUE;
				filler = amFilMask_Set;
				break;
			case VG_UNION_MASK:
				trackScanlines = AM_FALSE;
				filler = amFilMask_Union;
				break;
			case VG_INTERSECT_MASK:
				trackScanlines = AM_TRUE;
				filler = amFilMask_Intersect;
				break;
			case VG_SUBTRACT_MASK:
				trackScanlines = AM_FALSE;
				filler = amFilMask_Subtract;
				break;
			default:
				trackScanlines = AM_FALSE;
				filler = NULL;
				break;
		}
		AM_ASSERT(filler);

		// do the real rasterization
		paintGen.paintDesc = paintDesc;
		paintGen.coverageLineDeltas = rasterizer->coverageLineDeltas;
		paintGen.clipBox = clipBox;

		switch (context->renderingQuality) {
			case VG_RENDERING_QUALITY_NONANTIALIASED:
				res = amRasDrawNoAA(surface, rasterizer, &paintGen, filler, paintDesc->fillRule, trackScanlines);
				break;
			case VG_RENDERING_QUALITY_FASTER:
				res = amRasDrawFaster(surface, rasterizer, &paintGen, filler, paintDesc->fillRule, trackScanlines);
				break;
			default:
				res = amRasDrawBetter(surface, rasterizer, &paintGen, filler, paintDesc->fillRule, clipBox, trackScanlines);
				break;
		}

		// clear all non-rasterized scanline
		if (res && trackScanlines)
			amRasPolygonsMaskClearEmptyLines(surface, &rasterizer->globalScanlineList, clipBox);

		return res;
	}
	else
		return AM_TRUE;
}
#endif

#if defined (RIM_VG_SRC)
#pragma pop
#endif

