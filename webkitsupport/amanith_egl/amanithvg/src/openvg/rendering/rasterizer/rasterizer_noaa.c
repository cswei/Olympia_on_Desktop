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
	\file rasterizer_noaa.c
	\brief Non-antialiased rasterizer, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "fillers.h"

#if defined(AM_LITE_PROFILE)
	#include "pixel_utils.h"
	#define PIXEL_TYPE AMuint16
#else
	#define PIXEL_TYPE AMuint32
#endif

/*!
	\brief Quick sort the global edge list helpers, when number of vertices is less than 65536.
	\param base pointer to the first element to sort.
	\param num number of elements to sort.
*/
void amRasGEL32Qsort(AMuint32 *base,
					 const AMuint32 num) {

	// NB: base is a pointer to AMSortGEL32 structures, but here we access it as 32 bit unsigned integers
	// to optimize comparison
	#define TOO_SMALL_FOR_QSORT 8
	#define STACK_SIZE (8 * sizeof(void *) - 2)
	#define GEL32SWAP(_a, _b) \
		swapTmp = *(_a); \
		*(_a) = *(_b); \
		*(_b) = swapTmp;

    AMuint32 *lo = base;
	AMuint32 *hi = base + (num - 1);
	AMuint32 *mid, *loElement, *hiElement, *loStack[STACK_SIZE], *hiStack[STACK_SIZE], swapTmp;
    AMuint32 size;
    AMint32 stackPtr = 0;

    if (num < 2)
        return;

recurse:
    size = (AMuint32)(hi - lo) + 1;

    if (size <= TOO_SMALL_FOR_QSORT) {
		AMuint32 *p, *max;
		while (hi > lo) {
			max = lo;
			for (p = lo + 1; p <= hi; ++p) {
				if (*p <= *max)
					max = p;
			}
			GEL32SWAP(max, hi)
			hi--;
		}
    }
    else {
        mid = lo + (size >> 1);

		if (*lo <= *mid) {
            GEL32SWAP(lo, mid)
        }
		if (*lo <= *hi) {
            GEL32SWAP(lo, hi)
        }
		if (*mid <= *hi) {
            GEL32SWAP(mid, hi)
        }

        loElement = lo;
        hiElement = hi;

        for (;;) {
            if (mid > loElement) {
                do  {
                    loElement++;
				} while (loElement < mid && *loElement > *mid);
            }
            if (mid <= loElement) {
                do  {
                    loElement++;
				} while (loElement <= hi && *loElement > *mid);
            }
            do  {
                hiElement--;
			} while (hiElement > mid && *hiElement <= *mid);

            if (hiElement < loElement)
                break;

            GEL32SWAP(loElement, hiElement)

            if (mid == hiElement)
                mid = loElement;
        }

        hiElement++;
        if (mid < hiElement)
			hiElement--;

        if (mid >= hiElement)
			hiElement--;

        if (hiElement - lo >= hi - loElement) {
            if (lo < hiElement) {
                loStack[stackPtr] = lo;
                hiStack[stackPtr] = hiElement;
                ++stackPtr;
            }
            if (loElement < hi) {
                lo = loElement;
                goto recurse;
            }
        }
        else {
            if (loElement < hi) {
                loStack[stackPtr] = loElement;
                hiStack[stackPtr] = hi;
                ++stackPtr;
            }
            if (lo < hiElement) {
                hi = hiElement;
                goto recurse;
            }
        }
    }
    --stackPtr;
    if (stackPtr >= 0) {
        lo = loStack[stackPtr];
        hi = hiStack[stackPtr];
        goto recurse;
    }

	#undef TOO_SMALL_FOR_QSORT
	#undef STACK_SIZE
	#undef GEL32SWAP
}

/*!
	\brief Quick sort the global edge list helpers, when number of vertices is more than 65535.
	\param base pointer to the first element to sort.
	\param num number of elements to sort.
*/
void amRasGEL64Qsort(AMSortGEL64 *base,
					 const AMuint32 num) {

	#define TOO_SMALL_FOR_QSORT 8
	#define STACK_SIZE (8 * sizeof(void *) - 2)
	#define GEL64SWAP(_a, _b) \
		swapTmp = *(_a); \
		*(_a) = *(_b); \
		*(_b) = swapTmp;

    AMSortGEL64 *lo = base;
	AMSortGEL64 *hi = base + (num - 1);
	AMSortGEL64 *mid, *loElement, *hiElement, *loStack[STACK_SIZE], *hiStack[STACK_SIZE], swapTmp;
    AMuint32 size;
    AMint32 stackPtr = 0;

    if (num < 2)
        return;

recurse:
    size = (AMuint32)(hi - lo) + 1;

    if (size <= TOO_SMALL_FOR_QSORT) {
		AMSortGEL64 *p, *max;
		while (hi > lo) {
			max = lo;
			for (p = lo + 1; p <= hi; ++p) {
				if (p->key <= max->key)
					max = p;
			}
			GEL64SWAP(max, hi)
			hi--;
		}
    }
    else {
        mid = lo + (size >> 1);

		if (lo->key <= mid->key) {
            GEL64SWAP(lo, mid)
        }
		if (lo->key <= hi->key) {
            GEL64SWAP(lo, hi)
        }
		if (mid->key <= hi->key) {
            GEL64SWAP(mid, hi)
        }

        loElement = lo;
        hiElement = hi;

        for (;;) {
            if (mid > loElement) {
                do  {
                    loElement++;
				} while (loElement < mid && loElement->key > mid->key);
            }
            if (mid <= loElement) {
                do  {
                    loElement++;
				} while (loElement <= hi && loElement->key > mid->key);
            }
            do  {
                hiElement--;
			} while (hiElement > mid && hiElement->key <= mid->key);

            if (hiElement < loElement)
                break;

            GEL64SWAP(loElement, hiElement)

            if (mid == hiElement)
                mid = loElement;
        }

        hiElement++;
        if (mid < hiElement)
			hiElement--;

        if (mid >= hiElement)
			hiElement--;

        if (hiElement - lo >= hi - loElement) {
            if (lo < hiElement) {
                loStack[stackPtr] = lo;
                hiStack[stackPtr] = hiElement;
                ++stackPtr;
            }
            if (loElement < hi) {
                lo = loElement;
                goto recurse;
            }
        }
        else {
            if (loElement < hi) {
                loStack[stackPtr] = loElement;
                hiStack[stackPtr] = hi;
                ++stackPtr;
            }
            if (lo < hiElement) {
                hi = hiElement;
                goto recurse;
            }
        }
    }
    --stackPtr;
    if (stackPtr >= 0) {
        lo = loStack[stackPtr];
        hi = hiStack[stackPtr];
        goto recurse;
    }

	#undef TOO_SMALL_FOR_QSORT
	#undef STACK_SIZE
	#undef GEL64SWAP
}

/*!
	\brief Sort the global edge list, when number of vertices is less than 65536.
	\param rasterizer rasterizer whose global edge list is to sort.
*/
void amRasGEL32Sort(AMRasterizer *rasterizer) {

	AMuint32 i;

	amRasGEL32Qsort((AMuint32 *)rasterizer->sortGEL32.data, (AMuint32)rasterizer->sortGEL32.size);

	for (i = 0; i < (AMuint32)rasterizer->sortGEL32.size; ++i)
		rasterizer->gel.data[i] = rasterizer->gelTmp.data[rasterizer->sortGEL32.data[i].idx];

	rasterizer->gel.size = rasterizer->sortGEL32.size;
}

/*!
	\brief Sort the global edge list, when number of vertices is more than 65535.
	\param rasterizer rasterizer whose global edge list is to sort.
*/
void amRasGEL64Sort(AMRasterizer *rasterizer) {

	AMuint32 i;

	amRasGEL64Qsort(rasterizer->sortGEL64.data, (AMuint32)rasterizer->sortGEL64.size);

	for (i = 0; i < (AMuint32)rasterizer->sortGEL64.size; ++i)
		rasterizer->gel.data[i] = rasterizer->gelTmp.data[rasterizer->sortGEL64.data[i].idx];

	rasterizer->gel.size = rasterizer->sortGEL64.size;
}

/*!
	\brief Build the global edge list, when number of vertices is less than 65536.
	\param rasterizer rasterizer whose global edge list is to build.
*/
void amRasGEL32Build(AMRasterizer *rasterizer) {

	AMint32 i, j, k, k0, edgeDy;
	AMSrfSpaceVertex *v0, *v1;
	AMSrfSpaceEdge newEdge;
	AMSortGEL32 newSortElement;

	#define ADD_EDGE(_k0, _k1) \
		v0 = &rasterizer->vertices.data[_k0]; \
		v1 = &rasterizer->vertices.data[_k1]; \
		edgeDy = (AMint32)v0->p.y - (AMint32)v1->p.y; \
		if (edgeDy != 0) { \
			if (amRasVertexCmp(v0, v1) < 0) { \
				newEdge.v0 = v0; \
				newEdge.v1 = v1; \
				newEdge.sign = -1; \
			} \
			else { \
				newEdge.v0 = v1; \
				newEdge.v1 = v0; \
				newEdge.sign = +1; \
				edgeDy = -edgeDy; \
			} \
			newEdge.oldSweepDist = newEdge.v0->p.x; \
			newEdge.m = (AMint32)(((AMint32)((AMint32)newEdge.v1->p.x - (AMint32)newEdge.v0->p.x) << 15) / edgeDy); \
			AM_DYNARRAY_PUSH_BACK_LIGHT(rasterizer->gelTmp, newEdge) \
			newSortElement.idx = (AMuint16)(rasterizer->gelTmp.size - 1); \
			newSortElement.key = newEdge.v0->p.y; \
			AM_DYNARRAY_PUSH_BACK_LIGHT(rasterizer->sortGEL32, newSortElement) \
		}

	rasterizer->gelTmp.size = 0;
	rasterizer->sortGEL32.size = 0;
	k = 0;
	for (i = 0; i < (AMint32)rasterizer->contourPts.size; ++i) {
		k0 = k;
		// elaborate the i-th contour
		for (j = 0; j < rasterizer->contourPts.data[i] - 1; ++j) {
			ADD_EDGE(k, k + 1)
			k++;
		}
		// create last closing edge
		ADD_EDGE(k, k0)
		k++;
	}

	#undef ADD_EDGE
}

/*!
	\brief Build the global edge list, when number of vertices is more than 65535.
	\param rasterizer rasterizer whose global edge list is to build.
*/
void amRasGEL64Build(AMRasterizer *rasterizer) {

	AMint32 i, j, k, k0, edgeDy;
	AMSrfSpaceVertex *v0, *v1;
	AMSrfSpaceEdge newEdge;
	AMSortGEL64 newSortElement;

	#define ADD_EDGE(_k0, _k1) \
		v0 = &rasterizer->vertices.data[_k0]; \
		v1 = &rasterizer->vertices.data[_k1]; \
		edgeDy = (AMint32)v0->p.y - (AMint32)v1->p.y; \
		if (edgeDy != 0) { \
			if (amRasVertexCmp(v0, v1) < 0) { \
				newEdge.v0 = v0; \
				newEdge.v1 = v1; \
				newEdge.sign = -1; \
			} \
			else { \
				newEdge.v0 = v1; \
				newEdge.v1 = v0; \
				newEdge.sign = +1; \
				edgeDy = -edgeDy; \
			} \
			newEdge.oldSweepDist = newEdge.v0->p.x; \
			newEdge.m = (AMint32)(((AMint32)((AMint32)newEdge.v1->p.x - (AMint32)newEdge.v0->p.x) << 15) / edgeDy); \
			AM_DYNARRAY_PUSH_BACK_LIGHT(rasterizer->gelTmp, newEdge) \
			newSortElement.idx = (AMuint32)(rasterizer->gelTmp.size - 1); \
			newSortElement.key = newEdge.v0->p.y; \
			AM_DYNARRAY_PUSH_BACK_LIGHT(rasterizer->sortGEL64, newSortElement) \
		}

	rasterizer->gelTmp.size = 0;
	rasterizer->sortGEL64.size = 0;
	k = 0;
	for (i = 0; i < (AMint32)rasterizer->contourPts.size; ++i) {
		k0 = k;
		// elaborate the i-th contour
		for (j = 0; j < rasterizer->contourPts.data[i] - 1; ++j) {
			ADD_EDGE(k, k + 1)
			k++;
		}
		// create last closing edge
		ADD_EDGE(k, k0)
		k++;
	}

	#undef ADD_EDGE
}

/*!
	\brief Setup (build and sort) the global edge list.
	\param rasterizer rasterizer whose global edge list is to setup.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amRasGELSetup(AMRasterizer *rasterizer) {

	// allocate memory for the temporary global edge list
	if (rasterizer->gelTmp.capacity < rasterizer->vertices.size) {
		AM_DYNARRAY_CLEAR_RESERVE(rasterizer->gelTmp, AMSrfSpaceEdge, rasterizer->vertices.size)
		if (rasterizer->gelTmp.error) {
			rasterizer->gelTmp.error = AM_DYNARRAY_NO_ERROR;
			return AM_FALSE;
		}
	}
	// allocate memory for the final global edge list
	if (rasterizer->gel.capacity < rasterizer->vertices.size) {
		AM_DYNARRAY_CLEAR_RESERVE(rasterizer->gel, AMSrfSpaceEdge, rasterizer->vertices.size)
		if (rasterizer->gel.error) {
			rasterizer->gel.error = AM_DYNARRAY_NO_ERROR;
			return AM_FALSE;
		}
	}
	
	// allocate memory for sortGEL structures
	if (rasterizer->vertices.size < 65536) {
		if (rasterizer->sortGEL32.capacity < rasterizer->vertices.size) {
			AM_DYNARRAY_CLEAR_RESERVE(rasterizer->sortGEL32, AMSortGEL32, rasterizer->vertices.size)
			if (rasterizer->sortGEL32.error) {
				rasterizer->sortGEL32.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
		}
		amRasGEL32Build(rasterizer);
		amRasGEL32Sort(rasterizer);
	}
	else {
		if (rasterizer->sortGEL64.capacity < rasterizer->vertices.size) {
			AM_DYNARRAY_CLEAR_RESERVE(rasterizer->sortGEL64, AMSortGEL64, rasterizer->vertices.size)
			if (rasterizer->sortGEL64.error) {
				rasterizer->sortGEL64.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
		}
		amRasGEL64Build(rasterizer);
		amRasGEL64Sort(rasterizer);
	}
	return AM_TRUE;
}

/*!
	\brief Shell sort the active edge list.
	\param base pointer to the first element to sort.
	\param num number of elements to sort.
*/
void amRasAELSort(AMEdgeSweepDistance *base,
				  const AMint32 num) {

	AMint32 i, h;
	
	// NB: we always have an even number of segment, we treat common cases (2 and 4) outside the generic
	// sorting routines
	if (num == 2) {
		if (base[0].sweepDist > base[1].sweepDist) {
			AMEdgeSweepDistance v = base[0];
			base[0] = base[1];
			base[1] = v;
		}
	}
	else
	if (num == 4) {
		for (i = 0; i < 3; ++i) {
			for (h = i + 1; h < 4; ++h) {
				if (base[i].sweepDist > base[h].sweepDist) {
					AMEdgeSweepDistance v = base[i];
					base[i] = base[h];
					base[h] = v;
				}
			}
		}
	}
	else {
		for (h = 1; h < num / 9; h = 3 * h + 1) ;
		for (; h > 0; h /= 3) {
			for (i = h; i < num; ++i) {
				AMint32 j = i;
				AMEdgeSweepDistance v = base[i];
				while (j >= h && v.sweepDist < base[j - h].sweepDist) {
					base[j] = base[j - h];
					j -= h;
				}
				base[j] = v;
			}
		}
	}
}

/*!
	\brief Compute pixel coverage deltas using active edge sweepline distances, according to the specified fillrule.
	\param minX updated x coordinate of the first interested pixel in the scanline.
	\param maxX updated x coordinate of the last interested pixel in the scanline.
	\param rasterizer rasterizer containing active edges sweep line distances.
	\param fillRule input fillrule.
	\note used by the noaa rasterizer.
*/
void amRasCoverageComputeNoAA(AMuint16 *minX,
							  AMuint16 *maxX,
							  AMRasterizer *rasterizer,
							  const VGFillRule fillRule) {

	AMint32 i, even;
	AMuint16 dist;

	AM_ASSERT(rasterizer);
	AM_ASSERT((rasterizer->ael.size & 1) == 0);
	AM_ASSERT(rasterizer->ael.size >= 2);

	// calculate first segment out of the loop, and update minX
	dist = rasterizer->activeEdgeSweepDists.data[0].sweepDist;
	if ((dist & AM_RAS_FIXED_MASK) <= AM_RAS_FIXED_HALF) {
		rasterizer->coverageLineDeltas[dist >> AM_RAS_FIXED_PRECISION] += AM_RAS_MAX_COVERAGE;
		*minX = dist >> AM_RAS_FIXED_PRECISION;
	}
	else {
		rasterizer->coverageLineDeltas[(dist >> AM_RAS_FIXED_PRECISION) + 1] += AM_RAS_MAX_COVERAGE;
		*minX = (dist >> AM_RAS_FIXED_PRECISION) + 1;
	}

	if (fillRule == VG_EVEN_ODD) {

		even = 0;
		for (i = 1; i < (AMint32)rasterizer->activeEdgeSweepDists.size - 1; ++i) {

			dist = rasterizer->activeEdgeSweepDists.data[i].sweepDist;
			if (even) {
				if ((dist & AM_RAS_FIXED_MASK) <= AM_RAS_FIXED_HALF)
					rasterizer->coverageLineDeltas[dist >> AM_RAS_FIXED_PRECISION] += AM_RAS_MAX_COVERAGE;
				else
					rasterizer->coverageLineDeltas[(dist >> AM_RAS_FIXED_PRECISION) + 1] += AM_RAS_MAX_COVERAGE;
			}
			else {
				if ((dist & AM_RAS_FIXED_MASK) <= AM_RAS_FIXED_HALF)
					rasterizer->coverageLineDeltas[dist >> AM_RAS_FIXED_PRECISION] -= AM_RAS_MAX_COVERAGE;
				else
					rasterizer->coverageLineDeltas[(dist >> AM_RAS_FIXED_PRECISION) + 1] -= AM_RAS_MAX_COVERAGE;
			}
			even = !even;
		}

		// calculate last segment out of the loop, and update maxX
		dist = rasterizer->activeEdgeSweepDists.data[i].sweepDist;
		if ((dist & AM_RAS_FIXED_MASK) <= AM_RAS_FIXED_HALF) {
			rasterizer->coverageLineDeltas[dist >> AM_RAS_FIXED_PRECISION] -= AM_RAS_MAX_COVERAGE;
			*maxX = dist >> AM_RAS_FIXED_PRECISION;
		}
		else {
			rasterizer->coverageLineDeltas[(dist >> AM_RAS_FIXED_PRECISION) + 1] -= AM_RAS_MAX_COVERAGE;
			*maxX = (dist >> AM_RAS_FIXED_PRECISION) + 1;
		}
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
			if ((dist & AM_RAS_FIXED_MASK) <= AM_RAS_FIXED_HALF)
				rasterizer->coverageLineDeltas[dist >> AM_RAS_FIXED_PRECISION] -= AM_RAS_MAX_COVERAGE;
			else
				rasterizer->coverageLineDeltas[(dist >> AM_RAS_FIXED_PRECISION) + 1] -= AM_RAS_MAX_COVERAGE;

			if (i == (AMint32)rasterizer->activeEdgeSweepDists.size)
				break;
			
			currentSign = rasterizer->activeEdgeSweepDists.data[i].edge->sign;
			
			dist = rasterizer->activeEdgeSweepDists.data[i].sweepDist;
			if ((dist & AM_RAS_FIXED_MASK) <= AM_RAS_FIXED_HALF)
				rasterizer->coverageLineDeltas[dist >> AM_RAS_FIXED_PRECISION] += AM_RAS_MAX_COVERAGE;
			else
				rasterizer->coverageLineDeltas[(dist >> AM_RAS_FIXED_PRECISION) + 1] += AM_RAS_MAX_COVERAGE;
			i++;
		}

		// update maxX
		*maxX = ((dist & AM_RAS_FIXED_MASK) <= AM_RAS_FIXED_HALF) ? dist >> AM_RAS_FIXED_PRECISION : (dist >> AM_RAS_FIXED_PRECISION) + 1;
	}
}

void amRasDirectFillNoAA(AMDrawingSurface *surface,
						 const AMRasterizer *rasterizer,
						 const AMuint16 y,
						 const VGFillRule fillRule,
						 AMPaintGen *paintGen) {
	
	AMEdgeSweepDistance *sweepDistances;
#if defined(AM_LITE_PROFILE)
	#define PIXELS_MEMSET amMemset16
	AMuint16 color = amPxlPack565(paintGen->paintColor32);
#else
	#define PIXELS_MEMSET amMemset32
	AMuint32 color = paintGen->paintColor32;
#endif
	PIXEL_TYPE *scrPixels = amSrfPixelsGet(surface);

	AM_ASSERT(surface);
	AM_ASSERT(rasterizer);
	AM_ASSERT(rasterizer->ael.size >= 2);
	AM_ASSERT((rasterizer->ael.size & 1) == 0);
	AM_ASSERT(rasterizer->ael.size == rasterizer->activeEdgeSweepDists.size);

	scrPixels += (amSrfHeightGet(surface) - (y >> AM_RAS_FIXED_PRECISION) - 1) * amSrfWidthGet(surface);
	sweepDistances = rasterizer->activeEdgeSweepDists.data;

	if (fillRule == VG_EVEN_ODD) {

		AMuint32 i;
		AMuint32 j = (AMuint32)rasterizer->activeEdgeSweepDists.size >> 1;
		// loop over edge couples
		for (i = j; i != 0; --i) {

			AMuint32 dist0 = sweepDistances[0].sweepDist;
			AMuint32 dist1 = sweepDistances[1].sweepDist;
			AMuint32 int_x0 = ((dist0 & AM_RAS_FIXED_MASK) <= AM_RAS_FIXED_HALF) ? (dist0 >> AM_RAS_FIXED_PRECISION) : ((dist0 >> AM_RAS_FIXED_PRECISION) + 1);
			AMuint32 int_x1 = ((dist1 & AM_RAS_FIXED_MASK) <= AM_RAS_FIXED_HALF) ? (dist1 >> AM_RAS_FIXED_PRECISION) : ((dist1 >> AM_RAS_FIXED_PRECISION) + 1);

			AM_ASSERT(int_x0 <= int_x1);
			AM_ASSERT(int_x1 <= (AMuint32)amSrfWidthGet(surface));
			PIXELS_MEMSET(&scrPixels[int_x0], color, int_x1 - int_x0);
			sweepDistances += 2;
		}
	}
	else {
		AMuint32 i = 1;
		AMuint32 j = (AMuint32)rasterizer->activeEdgeSweepDists.size;
		AMuint32 dist0, dist1, int_x0, int_x1;
		AMint32 currentSign;

		currentSign = sweepDistances[0].edge->sign;
		dist0 = sweepDistances[0].sweepDist;
		int_x0 = ((dist0 & AM_RAS_FIXED_MASK) <= AM_RAS_FIXED_HALF) ? (dist0 >> AM_RAS_FIXED_PRECISION) : ((dist0 >> AM_RAS_FIXED_PRECISION) + 1);

		while (i < j) {

			while (currentSign != 0 && i < j) {
				currentSign += sweepDistances[i].edge->sign;
				i++;
			}

			dist1 = sweepDistances[i - 1].sweepDist;
			int_x1 = ((dist1 & AM_RAS_FIXED_MASK) <= AM_RAS_FIXED_HALF) ? (dist1 >> AM_RAS_FIXED_PRECISION) : ((dist1 >> AM_RAS_FIXED_PRECISION) + 1);
			AM_ASSERT(int_x0 <= int_x1);
			AM_ASSERT(int_x1 <= (AMuint32)amSrfWidthGet(surface));
			PIXELS_MEMSET(&scrPixels[int_x0], color, int_x1 - int_x0);

			if (i == j)
				break;
			
			currentSign = sweepDistances[i].edge->sign;
			dist0 = sweepDistances[i].sweepDist;
			int_x0 = ((dist0 & AM_RAS_FIXED_MASK) <= AM_RAS_FIXED_HALF) ? (dist0 >> AM_RAS_FIXED_PRECISION) : ((dist0 >> AM_RAS_FIXED_PRECISION) + 1);
			i++;
		}
	}
	#undef PIXELS_MEMSET
}

/*!
	\brief Perform polygon drawing, using the non-antialiased rasterizer.
	\param surface destination drawing surface.
	\param rasterizer rasterizer containing polygon data and related structures.
	\param paintGen paint generator structure, needed to the specified scanline filler.
	\param filler scanline filler function to use during the drawing.
	\param fillRule fillrule to use.
	\param trackScanlines if AM_TRUE, the rasterizer keeps track of rasterized scanlines (used later for rendering on alpha mask).
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amRasDrawNoAA(AMDrawingSurface *surface,
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
	if ((rasterizer->gel.data[0].v0->p.y & AM_RAS_FIXED_MASK) >= AM_RAS_FIXED_HALF)
		y = (rasterizer->gel.data[0].v0->p.y & AM_RAS_INT_MASK) | AM_RAS_FIXED_HALF;
	else
		y = ((rasterizer->gel.data[0].v0->p.y & AM_RAS_INT_MASK) | AM_RAS_FIXED_HALF) - AM_RAS_FIXED_ONE;

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
			if (paintDescCanBypassFiller) {
				AM_ASSERT(minX > maxX);
				amRasDirectFillNoAA(surface, rasterizer, y, fillRule, paintGen);
			}
			else
				amRasCoverageComputeNoAA(&minX, &maxX, rasterizer, fillRule);

			if (minX <= maxX) {
				if (maxX >= amSrfWidthGet(surface)) {
					maxX = amSrfWidthGet(surface) - 1;
					if (minX >= amSrfWidthGet(surface))
						minX = amSrfWidthGet(surface) - 1;
				}
				// call the scanline filler
				filler(surface, paintGen, y >> AM_RAS_FIXED_PRECISION, minX, maxX);
				minX = AM_RAS_MAX_COORDINATE;
				maxX = 0;
			}
		// keep track of drawn scanlines
		#if (AM_OPENVG_VERSION >= 110)
			if (trackScanlines) {
				AM_DYNARRAY_PUSH_BACK_LIGHT(rasterizer->globalScanlineList, y >> AM_RAS_FIXED_PRECISION)
			}
		#endif
		}
		else {
			if (i < (AMint32)rasterizer->gel.size) {
				if ((rasterizer->gel.data[i].v0->p.y & AM_RAS_FIXED_MASK) >= AM_RAS_FIXED_HALF)
					y = ((rasterizer->gel.data[i].v0->p.y & AM_RAS_INT_MASK) | AM_RAS_FIXED_HALF) + AM_RAS_FIXED_ONE;
				else
					y = ((rasterizer->gel.data[i].v0->p.y & AM_RAS_INT_MASK) | AM_RAS_FIXED_HALF);
			}
		}

		if (y == AM_RAS_FIXED_HALF)
			break;
		y -= AM_RAS_FIXED_ONE;
	}

	return AM_TRUE;
}

#undef PIXEL_TYPE

#if defined (RIM_VG_SRC)
#pragma pop
#endif
