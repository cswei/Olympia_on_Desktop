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

#ifndef _RASTERIZER_H
#define _RASTERIZER_H

/*!
	\file rasterizer.h
	\brief Polygon rasterization routines, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "rasterizer_better.h"
#include "rasterizer_faster.h"

/*!
	\brief Polygon rasterizer structure. It contains all data used by the 3 polygon rasterizers: better, faster, noaa.
*/
typedef struct _AMRasterizer {
	//! Clipped polygon vertices, in drawing surface space.
	AMSrfSpaceVertexDynArray vertices;
	//! Array that contains coverage deltas for a single (current) scanline.
	AMint32 *coverageLineDeltas;
	//! Number of drawing surface space vertices for each sub-contour.
	AMInt32DynArray contourPts;
	//! Maximum y coordinate (integer) of the clipBox (better rasterizer only).
	AMuint16 boxMaxY;
	//! Bentley-Ottman sorted events queue (better rasterizer only).
	AMSrfSpaceEventDynArray eventsQueue;
	//! Bentley-Ottman unsorted events queue (better rasterizer only).
	AMSrfSpaceEventDynArray eventsQueueTmp;
	//! Numerators (x, y) for intersection points; intersection points are represented in integer rational arithmetic (better rasterizer only).
	AMSrfSpaceIntersectionNumsDynArray intersectionNumerators;
	//! Helpers used by events merge sort (better rasterizer only).
	AMSrfSpaceMSortHelperDynArray msortHelpers;
	//! Helpers used by events quick sort (better rasterizer only).
	AMSrfSpaceQSortHelperDynArray qsortHelpers;
	//! Unsorted global edge list (faster and noaa rasterizers).
	AMSrfSpaceEdgeDynArray gelTmp;
	//! It contains sweep line distances for each edge present in the active edge list (faster and noaa rasterizers).
	AMEdgeSweepDistanceDynArray activeEdgeSweepDists;
	//! Quick sort helper, when number of vertices is less than 65536 (faster and noaa rasterizers).
	AMSortGEL32DynArray sortGEL32;
	//! Quick sort helper, when number of vertices is more than 65535 (faster and noaa rasterizers).
	AMSortGEL64DynArray sortGEL64;
	//! Global edge list.
	AMSrfSpaceEdgeDynArray gel;
	//! Active edge list (dictionary).
	AMSrfSpaceEdgePtrDynArray ael;
#if (AM_OPENVG_VERSION >= 110)
	//! List of all scanline interested by the current polygon rasterization (used by rendering on alpha mask only).
	AMInt32DynArray globalScanlineList;
#endif
} AMRasterizer;

// Rasterizer constructor.
AMbool amRasCreate(AMRasterizer **rasterizerPtr);
// Rasterizer destructor.
void amRasDestroy(AMRasterizer *rasterizer);
// Retrieve memory from rasterizer's internal structures.
void amRasMemoryRetrieve(AMRasterizer *rasterizer,
						 const AMbool maxMemoryRetrieval);

#if defined(AM_SRE)
// High level function to draw a set of closed polygons onto the drawing surface.
AMbool amRasPolygonsDraw(void *_context,
						 void *_surface,
						 AMRasterizer *rasterizer,
						 const AM_RAS_INPUT_VERTICES_ARRAY_TYPE *points,
						 const AMInt32DynArray *ptsPerContour,
						 const void *_paintDesc,
						 const AMAABox2i *clipBox,
						 const AMbool doClipping);

// High level function to draw an image made of 4 edges onto the drawing surface.
AMbool amRasImageDraw(const void *_context,
					  void *_surface,
					  AMRasterizer *rasterizer,
					  const AMVect2f *p0,
					  const AMVect2f *p1,
					  const AMVect2f *p2,
					  const AMVect2f *p3,
					  const void *_paintDesc,
					  const AMbool clear,
					  const AMAABox2i *clipBox);
#endif

#if (AM_OPENVG_VERSION >= 110)
// High level function to draw a set of closed polygons onto the alpha mask.
AMbool amRasPolygonsMaskDraw(const void *_context,
							 void *_surface,
							 AMRasterizer *rasterizer,
							 const AM_RAS_INPUT_VERTICES_ARRAY_TYPE *points,
							 const AMInt32DynArray *ptsPerContour,
							 const void *_paintDesc,
							 const VGMaskOperation operation,
							 const AMAABox2i *clipBox,
							 const AMbool doClipping);
#endif
#endif
