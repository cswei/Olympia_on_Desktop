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
	\file rasterizer_faster.c
	\brief Faster rasterizer, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "fillers.h"
#include "pixel_utils.h"
#include "vgimage.h"

#if defined(AM_LITE_PROFILE)
	#define PIXEL_TYPE AMuint16
#else
	#define PIXEL_TYPE AMuint32
#endif

//! 0.25 in AM_RAS_FIXED_PRECISION (fixed point).
#define AM_RAS_FIXED_QUARTER		(1 << (AM_RAS_FIXED_PRECISION - 2))
//! 0.75 in AM_RAS_FIXED_PRECISION (fixed point).
#define AM_RAS_FIXED_THREE_QUARTER	(AM_RAS_FIXED_ONE - AM_RAS_FIXED_QUARTER)
//! Half maximum coverage of a single pixel.
#define AM_RAS_HALF_MAX_COVERAGE	(AM_RAS_MAX_COVERAGE >> 1)
//! Minimum height unit, in AM_RAS_FIXED_PRECISION.
#define AM_RAS_SLICE_HEIGHT			(1 << (29 - AM_RAS_FIXED_PRECISION))

// this function are implemented in rasterizer_noaa.c
void amRasGEL32Qsort(AMuint32 *base, const AMuint32 num);
void amRasGEL64Qsort(AMSortGEL64 *base, const AMuint32 num);
void amRasGEL32Sort(AMRasterizer *rasterizer);
void amRasGEL64Sort(AMRasterizer *rasterizer);
void amRasGEL32Build(AMRasterizer *rasterizer);
void amRasGEL64Build(AMRasterizer *rasterizer);
AMbool amRasGELSetup(AMRasterizer *rasterizer);
void amRasAELSort(AMEdgeSweepDistance *base, const AMint32 num);

/*!
	\brief Compute pixel coverage deltas using active edge sweepline distances, according to the specified fillrule.
	\param minX updated x coordinate of the first interested pixel in the scanline.
	\param maxX updated x coordinate of the last interested pixel in the scanline.
	\param rasterizer rasterizer containing active edges sweep line distances.
	\param fillRule input fillrule.
	\note used by the faster rasterizer.
*/
void amRasCoverageComputeFaster(AMuint16 *minX,
								AMuint16 *maxX,
								AMRasterizer *rasterizer,
								const VGFillRule fillRule) {

	AMint32 i, even;
	AMint32 a0, a1;
	AMuint16 dist;

	AM_ASSERT(rasterizer);
	AM_ASSERT((rasterizer->ael.size & 1) == 0);
	AM_ASSERT(rasterizer->ael.size >= 2);

	// calculate first segment out of the loop, and update minX
	dist = rasterizer->activeEdgeSweepDists.data[0].sweepDist;
	a0 = (dist & AM_RAS_FIXED_MASK) * AM_RAS_SLICE_HEIGHT;
	a1 = AM_RAS_HALF_MAX_COVERAGE - a0;
	rasterizer->coverageLineDeltas[dist >> AM_RAS_FIXED_PRECISION] += a1;
	rasterizer->coverageLineDeltas[(dist >> AM_RAS_FIXED_PRECISION) + 1] += a0;
	// update minX
	if ((dist >> AM_RAS_FIXED_PRECISION) < *minX)
		*minX = dist >> AM_RAS_FIXED_PRECISION;

	if (fillRule == VG_EVEN_ODD) {

		even = 0;
		for (i = 1; i < (AMint32)rasterizer->activeEdgeSweepDists.size - 1; ++i) {

			dist = rasterizer->activeEdgeSweepDists.data[i].sweepDist;
			a0 = (dist & AM_RAS_FIXED_MASK) * AM_RAS_SLICE_HEIGHT;
			a1 = AM_RAS_HALF_MAX_COVERAGE - a0;

			if (even) {
				rasterizer->coverageLineDeltas[dist >> AM_RAS_FIXED_PRECISION] += a1;
				rasterizer->coverageLineDeltas[(dist >> AM_RAS_FIXED_PRECISION) + 1] += a0;
			}
			else {
				rasterizer->coverageLineDeltas[dist >> AM_RAS_FIXED_PRECISION] -= a1;
				rasterizer->coverageLineDeltas[(dist >> AM_RAS_FIXED_PRECISION) + 1] -= a0;
			}
			even = !even;
		}

		// calculate last segment out of the loop, and update maxX
		dist = rasterizer->activeEdgeSweepDists.data[i].sweepDist;
		a0 = (dist & AM_RAS_FIXED_MASK) * AM_RAS_SLICE_HEIGHT;
		a1 = AM_RAS_HALF_MAX_COVERAGE - a0;
		rasterizer->coverageLineDeltas[dist >> AM_RAS_FIXED_PRECISION] -= a1;
		rasterizer->coverageLineDeltas[(dist >> AM_RAS_FIXED_PRECISION) + 1] -= a0;
		// update maxX
		if (((dist >> AM_RAS_FIXED_PRECISION) + 1) > *maxX)
			*maxX = (dist >> AM_RAS_FIXED_PRECISION) + 1;
	}
	else {
		AMint32 currentSign = rasterizer->activeEdgeSweepDists.data[0].edge->sign;

		i = 1;
		while (i < (AMint32)rasterizer->activeEdgeSweepDists.size) {

			while (currentSign != 0 && i < (AMint32)rasterizer->activeEdgeSweepDists.size) {
				currentSign += rasterizer->activeEdgeSweepDists.data[i].edge->sign;
				i++;
			}

			dist = rasterizer->activeEdgeSweepDists.data[i - 1].sweepDist;
			a0 = (dist & AM_RAS_FIXED_MASK) * AM_RAS_SLICE_HEIGHT;
			a1 = AM_RAS_HALF_MAX_COVERAGE - a0;
			rasterizer->coverageLineDeltas[dist >> AM_RAS_FIXED_PRECISION] -= a1;
			rasterizer->coverageLineDeltas[(dist >> AM_RAS_FIXED_PRECISION) + 1] -= a0;

			if (i == (AMint32)rasterizer->activeEdgeSweepDists.size)
				break;
			
			currentSign = rasterizer->activeEdgeSweepDists.data[i].edge->sign;
			
			dist = rasterizer->activeEdgeSweepDists.data[i].sweepDist;
			a0 = (dist & AM_RAS_FIXED_MASK) * AM_RAS_SLICE_HEIGHT;
			a1 = AM_RAS_HALF_MAX_COVERAGE - a0;
			rasterizer->coverageLineDeltas[dist >> AM_RAS_FIXED_PRECISION] += a1;
			rasterizer->coverageLineDeltas[(dist >> AM_RAS_FIXED_PRECISION) + 1] += a0;

			i++;
		}
		// update maxX
		if (((dist >> AM_RAS_FIXED_PRECISION) + 1) > *maxX)
			*maxX = (dist >> AM_RAS_FIXED_PRECISION) + 1;
	}
}

void amRasDirectFillEdgeFasterSrc(const AMuint32 xLeft,
								  const AMuint32 xRight,
								  PIXEL_TYPE *scanLine,
								  const AMint32 addMask,
								  const AMint32 negate,
								  const AMuint32 color) {

	AMint32 cov;
	AMint32 int_l = xLeft >> AM_RAS_FIXED_PRECISION;
	AMint32 int_r = xRight >> AM_RAS_FIXED_PRECISION;
	AMint32 fract_l = xLeft & AM_RAS_FIXED_MASK;
	AMint32 fract_r = xRight & AM_RAS_FIXED_MASK;
	AMint32 n = int_r - int_l;

#if defined(AM_LITE_PROFILE)
	#define WRITE_AA_PIXEL_SRC(_cov) {\
		AMuint32 DcaDa = amPxlUnpack565(*scanLine); \
		AMint32 tmpCov = addMask + (((_cov) ^ (AMuint32)(-negate)) + negate); \
		AM_ASSERT((_cov) >= 0 && (_cov) <= AM_RAS_MAX_COVERAGE); \
		AM_ASSERT(tmpCov >= 0 && tmpCov <= AM_RAS_MAX_COVERAGE); \
		*scanLine = amPxlPack565(amPxlLerp(((AMuint32)tmpCov) >> AM_RAS_COVERAGE_PRECISION, DcaDa, color)); \
	}
#else
	#define WRITE_AA_PIXEL_SRC(_cov) {\
		AMuint32 DcaDa = *scanLine; \
		AMint32 tmpCov = addMask + (((_cov) ^ (AMuint32)(-negate)) + negate); \
		AM_ASSERT((_cov) >= 0 && (_cov) <= AM_RAS_MAX_COVERAGE); \
		AM_ASSERT(tmpCov >= 0 && tmpCov <= AM_RAS_MAX_COVERAGE); \
		*scanLine = amPxlLerp(((AMuint32)tmpCov) >> AM_RAS_COVERAGE_PRECISION, DcaDa, color); \
	}
#endif

	// go to the first interested pixel
	scanLine += int_l;

	cov = AM_RAS_HALF_MAX_COVERAGE - (fract_l * AM_RAS_SLICE_HEIGHT);
	if (n == 0) {
		if (((fract_l | fract_r) != 0) || (!negate)) {
		    cov += AM_RAS_HALF_MAX_COVERAGE - (fract_r * AM_RAS_SLICE_HEIGHT);
		    WRITE_AA_PIXEL_SRC(cov)
	    }
	}
	else {
		AMuint32 i;

		WRITE_AA_PIXEL_SRC(cov)
		scanLine++;

		AM_ASSERT(int_r - int_l - 1 >= 0);
		for (i = int_r - int_l - 1; i != 0; --i) {
			WRITE_AA_PIXEL_SRC(AM_RAS_HALF_MAX_COVERAGE)
			scanLine++;
		}
		cov = AM_RAS_MAX_COVERAGE - (fract_r * AM_RAS_SLICE_HEIGHT);
		WRITE_AA_PIXEL_SRC(cov)
	}

	#undef WRITE_AA_PIXEL_SRC
}

typedef struct _AMPixelAreaFaster {
	AMint32 int_x;
	AMint32 covDelta;
} AMPixelAreaFaster;

void amRasDirectFillEdgeCoupleFaster(const AMDrawingSurface *surface,
									 const AMint32 xLeft0,
									 const AMint32 xRight0,
									 const AMint32 xLeft1,
									 const AMint32 xRight1,
									 PIXEL_TYPE *scanLine,
									 const AMuint32 color) {

	AMint32 cov, x, xMax, int_x;
	AMPixelAreaFaster areaDelta0[4], areaDelta1[4];
	AMint32 fract_l, fract_r;
	AMuint32 e0, e1;

	#define SETUP_EDGE(_xLeft, _xRight, _areaIncr) \
		fract_l = (_xLeft) & AM_RAS_FIXED_MASK; \
		fract_r = (_xRight) & AM_RAS_FIXED_MASK; \
		(_areaIncr)[0].int_x = (_xLeft) >> AM_RAS_FIXED_PRECISION; \
		(_areaIncr)[0].covDelta = AM_RAS_HALF_MAX_COVERAGE - (fract_l * AM_RAS_SLICE_HEIGHT); \
		(_areaIncr)[1].int_x = (_areaIncr)[0].int_x + 1; \
		(_areaIncr)[1].covDelta = fract_l * AM_RAS_SLICE_HEIGHT; \
		int_x = (_xRight) >> AM_RAS_FIXED_PRECISION; \
		if (int_x == (_areaIncr)[0].int_x) { \
			(_areaIncr)[0].covDelta += AM_RAS_HALF_MAX_COVERAGE - (fract_r * AM_RAS_SLICE_HEIGHT); \
			(_areaIncr)[1].covDelta += fract_r * AM_RAS_SLICE_HEIGHT; \
			(_areaIncr)[2].int_x = (_areaIncr)[3].int_x = (_areaIncr)[0].int_x - 1; \
		} \
		else \
		if (int_x == (_areaIncr)[1].int_x) { \
			(_areaIncr)[1].covDelta += AM_RAS_HALF_MAX_COVERAGE - (fract_r * AM_RAS_SLICE_HEIGHT); \
			(_areaIncr)[2].int_x = int_x + 1; \
			(_areaIncr)[2].covDelta = fract_r * AM_RAS_SLICE_HEIGHT; \
			(_areaIncr)[3].int_x = (_areaIncr)[0].int_x - 1; \
		} \
		else { \
			(_areaIncr)[2].int_x = (_xRight) >> AM_RAS_FIXED_PRECISION; \
			(_areaIncr)[2].covDelta = AM_RAS_HALF_MAX_COVERAGE - (fract_r * AM_RAS_SLICE_HEIGHT); \
			(_areaIncr)[3].int_x = (_areaIncr)[2].int_x + 1; \
			(_areaIncr)[3].covDelta = fract_r * AM_RAS_SLICE_HEIGHT; \
		}

#if defined(AM_LITE_PROFILE)
	#define WRITE_AA_PIXEL_SRC(_x, _cov) {\
		AMuint32 DcaDa = amPxlUnpack565(scanLine[(_x)]); \
		AM_ASSERT((_cov) >= 0 && (_cov) <= AM_RAS_MAX_COVERAGE); \
		scanLine[(_x)] = amPxlPack565(amPxlLerp(((AMuint32)(_cov)) >> AM_RAS_COVERAGE_PRECISION, DcaDa, color)); \
	}
#else
	#define WRITE_AA_PIXEL_SRC(_x, _cov) {\
		AMuint32 DcaDa = scanLine[(_x)]; \
		AM_ASSERT((_cov) >= 0 && (_cov) <= AM_RAS_MAX_COVERAGE); \
		scanLine[(_x)] = amPxlLerp(((AMuint32)(_cov)) >> AM_RAS_COVERAGE_PRECISION, DcaDa, color); \
	}
#endif

	AM_ASSERT(surface);
	AM_ASSERT(scanLine);
	AM_ASSERT(xLeft0 <= xLeft1);
	AM_ASSERT(xRight0 <= xRight1);
	AM_ASSERT(xLeft0 <= xRight0);
	AM_ASSERT(xLeft1 <= xRight1);

	SETUP_EDGE(xLeft0, xRight0, areaDelta0)
	SETUP_EDGE(xLeft1, xRight1, areaDelta1)
	xMax = AM_MIN((xRight1 >> AM_RAS_FIXED_PRECISION), amSrfWidthGet(surface) - 1);

	// first pixel
	x = areaDelta0[0].int_x;
	// edge0 contribute
	cov = areaDelta0[0].covDelta;
	e0 = 1;
	// edge1 contribute
	if (x == areaDelta1[0].int_x) {
		cov -= areaDelta1[0].covDelta;
		e1 = 1;
	}
	else
		e1 = 0;
	// write the pixel
	if (x <= xMax) {
	    WRITE_AA_PIXEL_SRC(x, cov);
	}
	x++;

	for (; x <= xMax; ++x) {
		// edge0 contribute
		if (x == areaDelta0[e0].int_x) {
			cov += areaDelta0[e0].covDelta;
			e0++;
		}
		// edge1 contribute
		if (x == areaDelta1[e1].int_x) {
			cov -= areaDelta1[e1].covDelta;
			e1++;
		}
		// write the pixel
		WRITE_AA_PIXEL_SRC(x, cov);
	}

	#undef SETUP_EDGE
	#undef WRITE_AA_PIXEL_SRC
}

AMbool amRasDirectFillFaster(AMDrawingSurface *surface,
							 const AMRasterizer *rasterizer,
							 const AMuint16 y,
							 const VGFillRule fillRule,
							 AMPaintGen *paintGen) {

	AMEdgeSweepDistance *sweepDistances;
	AMuint32 i, j;
	PIXEL_TYPE *scrPixels;
	AMuint32 xUp0, xDown0, xUp1, xDown1;
	AMuint32 xLeft0, xRight0, xLeft1, xRight1;
	AMuint32 int_x0, int_x1;
	AMuint32 color = paintGen->paintColor32;
	AMuint32 int_y = y >> AM_RAS_FIXED_PRECISION;
	AMuint16 yDown = y - AM_RAS_FIXED_HALF;

#if defined(AM_LITE_PROFILE)
	#define WRITE_AA_PIXEL(_x, _cov) {\
		AMuint32 DcaDa = amPxlUnpack565(scrPixels[(_x)]); \
		AM_ASSERT((_cov) >= 0 && (_cov) <= AM_RAS_MAX_COVERAGE); \
		scrPixels[(_x)] = amPxlPack565(amPxlLerp(((AMuint32)(_cov)) >> AM_RAS_COVERAGE_PRECISION, DcaDa, color)); \
	}
#else
	#define WRITE_AA_PIXEL(_x, _cov) {\
		AMuint32 DcaDa = scrPixels[(_x)]; \
		AM_ASSERT((_cov) >= 0 && (_cov) <= AM_RAS_MAX_COVERAGE); \
		scrPixels[(_x)] = amPxlLerp(((AMuint32)(_cov)) >> AM_RAS_COVERAGE_PRECISION, DcaDa, color); \
	}
#endif

	AM_ASSERT(surface);
	AM_ASSERT(rasterizer);
	AM_ASSERT(rasterizer->ael.size >= 2);
	AM_ASSERT((rasterizer->ael.size & 1) == 0);
	AM_ASSERT(rasterizer->ael.size == rasterizer->activeEdgeSweepDists.size);

	sweepDistances = rasterizer->activeEdgeSweepDists.data;

	// check preconditions to realize the fast fill
	if (fillRule == VG_NON_ZERO && (sweepDistances[0].edge->sign + sweepDistances[1].edge->sign) != 0)
		return AM_FALSE;
	// calculate new sweppline distances
	xUp0 = sweepDistances[0].sweepDist;
	xDown0 = amRasSweepLineDistance(yDown, sweepDistances[0].edge);
	xUp1 = sweepDistances[1].sweepDist;
	xDown1 = amRasSweepLineDistance(yDown, sweepDistances[1].edge);
	// test for crossing edges
	if (xDown0 > xDown1)
		return AM_FALSE;
	xRight1 = AM_MAX(xUp1, xDown1);
	int_x1 = xRight1 >> AM_RAS_FIXED_PRECISION;

	// lopp over edges couples
	sweepDistances += 2;
	j = (AMuint32)rasterizer->activeEdgeSweepDists.size >> 1;
	for (i = j - 1; i != 0; --i) {

		xUp0 = sweepDistances[0].sweepDist;
		xDown0 = amRasSweepLineDistance(yDown, sweepDistances[0].edge);
		xLeft0 = AM_MIN(xUp0, xDown0);
		// projections must lie in two different pixels
		int_x0 = xLeft0 >> AM_RAS_FIXED_PRECISION;
		if (int_x0 <= int_x1)
			return AM_FALSE;

		if (fillRule == VG_NON_ZERO && (sweepDistances[0].edge->sign + sweepDistances[1].edge->sign) != 0)
			return AM_FALSE;

		xUp1 = sweepDistances[1].sweepDist;
		xDown1 = amRasSweepLineDistance(yDown, sweepDistances[1].edge);
		// test for crossing edges
		if (xDown0 > xDown1)
			return AM_FALSE;

		xRight1 = AM_MAX(xUp1, xDown1);
		int_x1 = xRight1 >> AM_RAS_FIXED_PRECISION;
		sweepDistances += 2;
	}

	sweepDistances = rasterizer->activeEdgeSweepDists.data;
	scrPixels = amSrfPixelsGet(surface) + (amSrfHeightGet(surface) - int_y - 1) * amSrfWidthGet(surface);
	for (i = j; i != 0 ; --i) {

		AMuint32 int_r0, int_l1;

		// edge0
		xUp0 = sweepDistances[0].sweepDist;
		xDown0 = amRasSweepLineDistance(yDown, sweepDistances[0].edge);
		if (xDown0 < xUp0) {
			xLeft0 = xDown0;
			xRight0 = xUp0;
		}
		else {
			xLeft0 = xUp0;
			xRight0 = xDown0;
		}
		int_r0 = xRight0 >> AM_RAS_FIXED_PRECISION;
		AM_ASSERT((xLeft0 >> AM_RAS_FIXED_PRECISION) <= int_r0);
		AM_ASSERT(int_r0 < (AMuint32)amSrfWidthGet(surface) + 1);
		// edge 1
		xUp1 = sweepDistances[1].sweepDist;
		xDown1 = amRasSweepLineDistance(yDown, sweepDistances[1].edge);
		if (xDown1 < xUp1) {
			xLeft1 = xDown1;
			xRight1 = xUp1;
		}
		else {
			xLeft1 = xUp1;
			xRight1 = xDown1;
		}
		int_l1 = xLeft1 >> AM_RAS_FIXED_PRECISION;
		AM_ASSERT(int_l1 <= (xRight1 >> AM_RAS_FIXED_PRECISION));
		AM_ASSERT((xRight1 >> AM_RAS_FIXED_PRECISION) < (AMuint32)amSrfWidthGet(surface) + 1);

		if (int_r0 < int_l1) {
			// edges do not overlap (in x-direction)
			amRasDirectFillEdgeFasterSrc(xLeft0, xRight0, scrPixels, 0, 0, color);
		#if defined(AM_LITE_PROFILE)
			amMemset16(&scrPixels[int_r0 + 1], amPxlPack565(color), int_l1 - int_r0 - 1);
		#else
			amMemset32(&scrPixels[int_r0 + 1], color, int_l1 - int_r0 - 1);
		#endif
			amRasDirectFillEdgeFasterSrc(xLeft1, xRight1, scrPixels, AM_RAS_MAX_COVERAGE, 1, color);
		}
		else
			amRasDirectFillEdgeCoupleFaster(surface, xLeft0, xRight0, xLeft1, xRight1, scrPixels, color);

		// next edges couple
		sweepDistances += 2;
	}
	return AM_TRUE;
	#undef WRITE_AA_PIXEL
}

/*!
	\brief Perform polygon drawing, using the faster rasterizer.
	\param surface destination drawing surface.
	\param rasterizer rasterizer containing polygon data and related structures.
	\param paintGen paint generator structure, needed to the specified scanline filler.
	\param filler scanline filler function to use during the drawing.
	\param fillRule fillrule to use.
	\param trackScanlines if AM_TRUE, the rasterizer keeps track of rasterized scanlines (used later for rendering on alpha mask).
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amRasDrawFaster(AMDrawingSurface *surface,
					   AMRasterizer *rasterizer,
					   AMPaintGen *paintGen,
					   AMScanlineFillerFunction filler,
					   const VGFillRule fillRule,
					   const AMbool trackScanlines) {

	AMint32 i, j;
	AMuint16 y, minX, maxX;
	AMbool paintDescCanBypassFiller;

	// build and sort global edge list
	if (!amRasGELSetup(rasterizer))
		return AM_FALSE;

#if (AM_OPENVG_VERSION >= 110)
	rasterizer->globalScanlineList.size = 0;
#endif

	// ensure at least 2 edges
	if (rasterizer->gel.size < 2)
		return AM_TRUE;

	// paint is such as rasterizer/filler can be bypassed and realized in a faster way
	paintDescCanBypassFiller = (paintGen &&
								!paintGen->paintDesc->image &&
								paintGen->paintDesc->paintType == VG_PAINT_TYPE_COLOR &&
								paintGen->paintDesc->blendMode == VG_BLEND_SRC &&
								paintGen->paintDesc->masking == AM_FALSE);

	// initialize rasterizer variables
	minX = AM_RAS_MAX_COORDINATE;
	maxX = 0;

	// calculate the first y such as in the first loop iteration at least one edge will be added to the active edge list
	if ((rasterizer->gel.data[0].v0->p.y & AM_RAS_FIXED_MASK) >= AM_RAS_FIXED_THREE_QUARTER)
		y = (rasterizer->gel.data[0].v0->p.y & AM_RAS_INT_MASK) | AM_RAS_FIXED_THREE_QUARTER;
	else
	if ((rasterizer->gel.data[0].v0->p.y & AM_RAS_FIXED_MASK) < AM_RAS_FIXED_QUARTER)
		y = ((rasterizer->gel.data[0].v0->p.y & AM_RAS_INT_MASK) | AM_RAS_FIXED_THREE_QUARTER) - AM_RAS_FIXED_ONE;
	else
		y = (rasterizer->gel.data[0].v0->p.y & AM_RAS_INT_MASK) | AM_RAS_FIXED_QUARTER;

	// empty active edge table (dictionary)
	rasterizer->ael.size = 0;

#if (AM_OPENVG_VERSION >= 110)
	// empty global scanline list
	if (trackScanlines) {
		AMuint32 srfHeight = (AMuint32)amSrfHeightGet(surface);
		if (rasterizer->globalScanlineList.capacity < srfHeight + 1) {
			AM_DYNARRAY_CLEAR_RESERVE(rasterizer->globalScanlineList, AMint32, srfHeight + 1)
		}
		if (rasterizer->globalScanlineList.error) {
			rasterizer->globalScanlineList.error = AM_DYNARRAY_NO_ERROR;
			return AM_FALSE;
		}
	}
#endif

	i = 0;
	while (i < (AMint32)rasterizer->gel.size || rasterizer->ael.size > 0) {

		// remove all outgoing edges
		for (j = 0; j < (AMint32)rasterizer->ael.size;) {
			if (rasterizer->ael.data[j]->v1->p.y >= y) {
				rasterizer->ael.data[j] = rasterizer->ael.data[rasterizer->ael.size - 1];
				rasterizer->ael.size--;
			}
			else
				j++;
		}

		// insert all ingoing edges
		while (i < (AMint32)rasterizer->gel.size && rasterizer->gel.data[i].v0->p.y >= y) {
			if (!amRasEdgeZeroLength(&rasterizer->gel.data[i]) &&
				rasterizer->gel.data[i].v1->p.y < y) {
				AM_DYNARRAY_PUSH_BACK(rasterizer->ael, AMSrfSpaceEdgePtr, &rasterizer->gel.data[i])
			}
			i++;
		}
		// check for memory errors
		if (rasterizer->ael.error) {
			rasterizer->ael.error = AM_DYNARRAY_NO_ERROR;
			return AM_FALSE;
		}

		if (rasterizer->ael.size > 0) {

			// number of edges must be even
			AM_ASSERT((rasterizer->ael.size & 1) == 0);

			// calculate sweep line distances
			rasterizer->activeEdgeSweepDists.size = 0;
			for (j = 0; j < (AMint32)rasterizer->ael.size; ++j) {

				AMEdgeSweepDistance newDist;

				newDist.edge = rasterizer->ael.data[j];
				newDist.sweepDist = amRasSweepLineDistance(y, newDist.edge);
				AM_DYNARRAY_PUSH_BACK(rasterizer->activeEdgeSweepDists, AMEdgeSweepDistance, newDist)
			}
			// check for memory errors
			if (rasterizer->activeEdgeSweepDists.error) {
				rasterizer->activeEdgeSweepDists.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
			amRasAELSort(rasterizer->activeEdgeSweepDists.data, (AMint32)rasterizer->activeEdgeSweepDists.size);

			// try to bypass the coverage calculation + filler, if possible
			if (paintDescCanBypassFiller && (y & AM_RAS_FIXED_MASK) == AM_RAS_FIXED_THREE_QUARTER) {

				AMuint32 yDown = y - AM_RAS_FIXED_HALF;
				
				// check if no new edges are going to be inserted (inside this pixel line)
				if (i < (AMint32)rasterizer->gel.size && rasterizer->gel.data[i].v0->p.y < yDown) {

					AMbool fastFill = AM_TRUE;

					// now check that edges already present in the active edge list will remain active at least inside the current scanline
					for (j = 0; j < (AMint32)rasterizer->ael.size; ++j) {
						if (rasterizer->ael.data[j]->v1->p.y >= yDown) {
							fastFill = AM_FALSE;
							break;
						}
					}

					if (fastFill && amRasDirectFillFaster(surface, rasterizer, y, fillRule, paintGen)) {
						y = yDown;
						goto nextLineFaster;
					}
					// in this case (overlapped edges), simply go throught the "slower" path (amRasCoverageComputeFaster)
				}
			}

			amRasCoverageComputeFaster(&minX, &maxX, rasterizer, fillRule);

			// check if we have to render the current scanline. NB: because we have edges in active edge table
			// so minX and maxX are always set correctly
			if ((y & AM_RAS_FIXED_MASK) == AM_RAS_FIXED_QUARTER) {
				if (maxX >= amSrfWidthGet(surface)) {
					maxX = amSrfWidthGet(surface) - 1;
					if (minX >= amSrfWidthGet(surface))
						minX = amSrfWidthGet(surface) - 1;
				}
			#if (AM_OPENVG_VERSION >= 110)
				if (trackScanlines) {
					AM_DYNARRAY_PUSH_BACK_LIGHT(rasterizer->globalScanlineList, y >> AM_RAS_FIXED_PRECISION)
				}
			#endif
				filler(surface, paintGen, y >> AM_RAS_FIXED_PRECISION, minX, maxX);
				minX = AM_RAS_MAX_COORDINATE;
				maxX = 0;
			}
		}
		else {
			// check if we have to render the current scanline
			if ((y & AM_RAS_FIXED_MASK) == AM_RAS_FIXED_QUARTER && minX <= maxX) {
				if (maxX >= amSrfWidthGet(surface)) {
					maxX = amSrfWidthGet(surface) - 1;
					if (minX >= amSrfWidthGet(surface))
						minX = amSrfWidthGet(surface) - 1;
				}
			#if (AM_OPENVG_VERSION >= 110)
				if (trackScanlines) {
					AM_DYNARRAY_PUSH_BACK_LIGHT(rasterizer->globalScanlineList, y >> AM_RAS_FIXED_PRECISION)
				}
			#endif
				filler(surface, paintGen, y >> AM_RAS_FIXED_PRECISION, minX, maxX);
				minX = AM_RAS_MAX_COORDINATE;
				maxX = 0;
			}

			if (i < (AMint32)rasterizer->gel.size) {
				if ((rasterizer->gel.data[i].v0->p.y & AM_RAS_FIXED_MASK) >= AM_RAS_FIXED_THREE_QUARTER)
					y = ((rasterizer->gel.data[i].v0->p.y & AM_RAS_INT_MASK) | AM_RAS_FIXED_THREE_QUARTER) + AM_RAS_FIXED_HALF;
				else
				if ((rasterizer->gel.data[i].v0->p.y & AM_RAS_FIXED_MASK) < AM_RAS_FIXED_QUARTER)
					y = ((rasterizer->gel.data[i].v0->p.y & AM_RAS_INT_MASK) | AM_RAS_FIXED_THREE_QUARTER) - AM_RAS_FIXED_HALF;
				else
					y = (rasterizer->gel.data[i].v0->p.y & AM_RAS_INT_MASK) | AM_RAS_FIXED_THREE_QUARTER;
			}
		}

nextLineFaster:
		if (y == AM_RAS_FIXED_QUARTER)
			break;
		y -= AM_RAS_FIXED_HALF;
	}

	return AM_TRUE;
}

#undef AM_RAS_FIXED_QUARTER
#undef AM_RAS_FIXED_THREE_QUARTER
#undef AM_RAS_HALF_MAX_COVERAGE
#undef AM_RAS_SLICE_HEIGHT
#undef PIXEL_TYPE

#if defined (RIM_VG_SRC)
#pragma pop
#endif

