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
	\file rasterizer_better.c
	\brief Better rasterizer, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "fillers.h"
#include "vgimage.h"
#include "pixel_utils.h"

#if defined(AM_LITE_PROFILE)
	#define PIXEL_TYPE AMuint16
#else
	#define PIXEL_TYPE AMuint32
#endif

// Bentley-Ottman event type
#define SWAP_EVENT						(1U << 31)
#define DISCARDED_SWAP					(1U << 30)
#define IS_SWAP_EVENT(_event)			((_event)->flags & SWAP_EVENT)
#define IS_DISCARDED_SWAP(_event)		((_event)->flags & DISCARDED_SWAP)
#define INTERSECTION_NUMS_IDX(_event)	((_event)->flags & 0x3FFFFFFF)

//! Structure to store edge slopes, for the minimum slice height unit.
typedef struct _AMDivMod {
	//! Integer part of the slope.
	AMint32 div;
	//! Decimal part of the slope.
	AMint32 mod;
} AMDivMod;

typedef struct _AMPixelAreaBetter {
	AMint32 int_x;
	AMint32 covDelta;
} AMPixelAreaBetter;

// table used to get division result and modulo
#include "divmod_table.c"

/*!
	\brief Check if an edge is upgoing, respect to a given vertex.
	\param edge input edge to check.
	\param vertex input vertex.
	\return 1 if edge is upgoing respect to the vertex, else 0.
*/

#if defined(RIM_VG_SRC)
extern void RasterMemsetC32(DWORD * dst,DWORD colour,DWORD count);
#endif

AM_INLINE AMint32 amRasEdgeUpGoing(const AMSrfSpaceEdge *edge,
								   const AMSrfSpaceVertex *vertex) {

	const AMSrfSpaceVertex *v0 = edge->v0;
	const AMSrfSpaceVertex *v1 = edge->v1;

	AM_ASSERT(edge);
	AM_ASSERT(vertex);

	if (v0->yx == vertex->yx) {
		// edges with 0-length must be considered upgoing
		return (v0->yx == v1->yx) ? 1 : 0;
	}
	else
		return 1;
}

/*!
	\brief Check if an edge is downgoing, respect to a given vertex.
	\param edge input edge to check.
	\param vertex input vertex.
	\return 1 if edge is downgoing respect to the vertex, else 0.
*/
AM_INLINE AMint32 amRasEdgeDownGoing(const AMSrfSpaceEdge *edge,
									 const AMSrfSpaceVertex *vertex) {

	const AMSrfSpaceVertex *v0 = edge->v0;

	AM_ASSERT(edge);
	AM_ASSERT(vertex);

	return (v0->yx == vertex->yx) ? 1 : 0;
}

/*!
	\brief Compare two events.
	\param rasterizer rasterizer containing the intersection numerators array.
	\param e0 first event to compare.
	\param e1 second event to compare.
	\return -1 if e0 comes before e1, +1 if e0 comes after e1, 0 if e0 and e1 are on the same geometric position.
	\note comparisons are performed using lexicographic order.
*/
AMint32 amRasEventsCmp(const AMRasterizer *rasterizer,
					   const AMSrfSpaceEvent *e0,
					   const AMSrfSpaceEvent *e1) {

	AM_ASSERT(IS_SWAP_EVENT(e1));

	if (!IS_SWAP_EVENT(e0)) {

		AMSrfSpaceIntersectionNums *numsE1 = &rasterizer->intersectionNumerators.data[INTERSECTION_NUMS_IDX(e1)];
		AMuint64 l = UINT32_UINT32_MUL(e0->den.p.y, e1->den.yx);

		if (UINT64_GREATER(l, numsE1->yNum))
			return -1;
		else
		if (UINT64_LESSER(l, numsE1->yNum))
			return 1;
		else {
			l = UINT32_UINT32_MUL(e0->den.p.x, e1->den.yx);
			if (UINT64_LESSER(l, numsE1->xNum))
				return -1;
			else
			if (UINT64_GREATER(l, numsE1->xNum))
				return 1;
			else
				return 0;
		}
	}
	else {
		// two swap event
		AMUnsigned64 a, b, c, d, tmp0, tmp1;
		AMuint32 e, f;

		AMSrfSpaceIntersectionNums *numsE0 = &rasterizer->intersectionNumerators.data[INTERSECTION_NUMS_IDX(e0)];
		AMSrfSpaceIntersectionNums *numsE1 = &rasterizer->intersectionNumerators.data[INTERSECTION_NUMS_IDX(e1)];

		AM_ASSERT(IS_SWAP_EVENT(e0));
		AM_ASSERT(IS_SWAP_EVENT(e1));

		tmp0.n = numsE0->yNum;
		a.n = UINT32_UINT32_MUL(e1->den.yx, tmp0.c.hi);
		b.n = UINT32_UINT32_MUL(e1->den.yx, tmp0.c.lo);
		e = a.c.lo + b.c.hi;
		if (e < a.c.lo) a.c.hi++;

		tmp1.n = numsE1->yNum;
		c.n = UINT32_UINT32_MUL(e0->den.yx, tmp1.c.hi);
		d.n = UINT32_UINT32_MUL(e0->den.yx, tmp1.c.lo);
		f = c.c.lo + d.c.hi;
		if (f < c.c.lo)	c.c.hi++;

		if (a.c.hi > c.c.hi) return -1;
		else
		if (a.c.hi < c.c.hi) return 1;
		else
		if (e > f) return -1;
		else
		if (e < f) return 1;
		else
		if (b.c.lo > d.c.lo) return -1;
		else
		if (b.c.lo < d.c.lo) return 1;
		else {
			// check for x coordinate
			tmp0.n = numsE0->xNum;
			a.n = UINT32_UINT32_MUL(e1->den.yx, tmp0.c.hi);
			b.n = UINT32_UINT32_MUL(e1->den.yx, tmp0.c.lo);
			e = a.c.lo + b.c.hi;
			if (e < a.c.lo)	a.c.hi++;
			tmp1.n = numsE1->xNum;
			c.n = UINT32_UINT32_MUL(e0->den.yx, tmp1.c.hi);
			d.n = UINT32_UINT32_MUL(e0->den.yx, tmp1.c.lo);
			f = c.c.lo + d.c.hi;
			if (f < c.c.lo)	c.c.hi++;

			if (a.c.hi < c.c.hi) return -1;
			else
			if (a.c.hi > c.c.hi) return 1;
			else
			if (e < f) return -1;
			else
			if (e > f) return 1;
			else
			if (b.c.lo < d.c.lo) return -1;
			else
			if (b.c.lo > d.c.lo) return 1;
			else
				return 0;
		}
	}
}

/*!
	\brief Quick sort helpers.
	\param base pointer to the first element to sort.
	\param num number of elements to sort.
*/
void amRasQHelpersSort(AMSrfSpaceQSortHelper *base,
					   const AMuint32 num) {

	#define TOO_SMALL_FOR_QSORT 8
	#define STACK_SIZE (8 * sizeof(void *) - 2)
	#define SWAP_HELPER(_a, _b) \
		swapTmp = *(_a); \
		*(_a) = *(_b); \
		*(_b) = swapTmp;

	AMSrfSpaceQSortHelper *lo = base;
	AMSrfSpaceQSortHelper *hi = base + (num - 1);
	AMSrfSpaceQSortHelper *mid, *loElement, *hiElement, *loStack[STACK_SIZE], *hiStack[STACK_SIZE], swapTmp;
	AMuint32 size;
	AMint32 stackPtr = 0;

	if (num < 2)
		return;

recurse:
	size = (AMuint32)(hi - lo) + 1;

	if (size <= TOO_SMALL_FOR_QSORT) {
		AMSrfSpaceQSortHelper *p, *max;
		while (hi > lo) {
			max = lo;
			for (p = lo + 1; p <= hi; ++p) {
				if (p->key  > max->key)
					max = p;
			}
			SWAP_HELPER(max, hi)
			hi--;
		}
	}
	else {
		mid = lo + (size >> 1);

		if (lo->key > mid->key) {
			SWAP_HELPER(lo, mid)
		}
		if (lo->key > hi->key) {
			SWAP_HELPER(lo, hi)
		}
		if (mid->key > hi->key) {
			SWAP_HELPER(mid, hi)
		}

		loElement = lo;
		hiElement = hi;

		for (;;) {
			if (mid > loElement) {
				do  {
					loElement++;
				} while (loElement < mid && loElement->key <= mid->key);
			}
			if (mid <= loElement) {
				do  {
					loElement++;
				} while (loElement <= hi && loElement->key <= mid->key);
			}
			do  {
				hiElement--;
			} while (hiElement > mid && hiElement->key > mid->key);

			if (hiElement < loElement)
				break;

			SWAP_HELPER(loElement, hiElement)

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
	#undef SWAP_HELPER
}

/*!
	\brief Sort the Bentley-Ottman events queue, using quick sort algorithm.
	\param rasterizer rasterizer containing the events queue.
	\param boxMax maximum y coordinate of the clip box.
*/
void amRasEventsQsort(AMRasterizer *rasterizer,
					  const AMuint16 boxMax) {

	AMint32 i;

	if (rasterizer->eventsQueueTmp.size < 3) {
		rasterizer->eventsQueue.size = 0;
		return;
	}

	// sort "quick sort helpers"
	amRasQHelpersSort(rasterizer->qsortHelpers.data, (AMuint32)rasterizer->qsortHelpers.size);
	// now fill the sorted events queue
	for (i = 0; i < (AMint32)rasterizer->qsortHelpers.size; ++i) {
		rasterizer->eventsQueue.data[i] = rasterizer->eventsQueueTmp.data[rasterizer->qsortHelpers.data[i].idx];
		rasterizer->eventsQueue.data[i].den.p.y = boxMax - rasterizer->eventsQueue.data[i].den.p.y;
	}
}

/*!
	\brief Merge sort helpers.
	\param base pointer to the first element to sort.
	\param num number of elements to sort.
*/
void amRasMHelpersSort(AMSrfSpaceMSortHelper *base,
					   const AMuint32 num) {

	#define TOO_SMALL_FOR_QSORT 8
	#define STACK_SIZE (8 * sizeof(void *) - 2)
	#define SWAP_HELPER(_a, _b) \
		swapTmp = *(_a); \
		*(_a) = *(_b); \
		*(_b) = swapTmp;

	AMSrfSpaceMSortHelper *lo = base;
	AMSrfSpaceMSortHelper *hi = base + (num - 1);
	AMSrfSpaceMSortHelper *mid, *loElement, *hiElement, *loStack[STACK_SIZE], *hiStack[STACK_SIZE], swapTmp;
	AMuint32 size;
	AMint32 stackPtr = 0;

	if (num < 2)
		return;

recurse:
	size = (AMuint32)(hi - lo) + 1;

	if (size <= TOO_SMALL_FOR_QSORT) {
		AMSrfSpaceMSortHelper *p, *max;
		while (hi > lo) {
			max = lo;
			for (p = lo + 1; p <= hi; ++p) {
				if (p->key >= max->key)
					max = p;
			}
			SWAP_HELPER(max, hi)
			hi--;
		}
	}
	else {
		mid = lo + (size >> 1);

		if (lo->key >= mid->key) {
			SWAP_HELPER(lo, mid)
		}
		if (lo->key >= hi->key) {
			SWAP_HELPER(lo, hi)
		}
		if (mid->key >= hi->key) {
			SWAP_HELPER(mid, hi)
		}

		loElement = lo;
		hiElement = hi;

		for (;;) {
			if (mid > loElement) {
				do  {
					loElement++;
				} while (loElement < mid && loElement->key < mid->key);
			}
			if (mid <= loElement) {
				do  {
					loElement++;
				} while (loElement <= hi && loElement->key < mid->key);
			}
			do  {
				hiElement--;
			} while (hiElement > mid && hiElement->key >= mid->key);

			if (hiElement < loElement)
				break;

			SWAP_HELPER(loElement, hiElement)

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
	#undef SWAP_HELPER
}

/*!
	\brief Sort the Bentley-Ottman events queue, using merge sort algorithm.
	\param rasterizer rasterizer containing the events queue.
	\param boxMax maximum y coordinate of the clip box.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
*/
AMbool amRasEventsMsort(AMRasterizer *rasterizer,
					  const AMuint16 boxMax) {

	AMint32 i, j0, j1, k;
	AMSrfSpaceVertex old;
	AMint32 curSize;
	AMSrfSpaceEvent *dstQueue;
	AMSrfSpaceMSortHelper *originalMSortHelpersArray, newHelper;

	if (rasterizer->eventsQueueTmp.size < 3) {
		rasterizer->eventsQueue.size = 0;
		return AM_TRUE;
	}

	rasterizer->msortHelpers.size = 0;
	k = 0;
	old.yx = rasterizer->eventsQueueTmp.data[0].den.yx;
	curSize = 0;
	i = 1;
	j0 = 0;

ordering_loop:
	while (i < (AMint32)rasterizer->eventsQueueTmp.size && rasterizer->eventsQueueTmp.data[i].den.yx >= old.yx) {
		old = rasterizer->eventsQueueTmp.data[i].den;
		i++;
		curSize++;
		j0++;
	}
	if (curSize > 0) {
		newHelper.key = rasterizer->eventsQueueTmp.data[k].den.yx;
		newHelper.idx = k;
		newHelper.size = curSize;
		AM_DYNARRAY_PUSH_BACK(rasterizer->msortHelpers, AMSrfSpaceMSortHelper, newHelper)
		k += curSize;
	}

	while (i < (AMint32)rasterizer->eventsQueueTmp.size && rasterizer->eventsQueueTmp.data[i].den.yx <= old.yx) {
		old = rasterizer->eventsQueueTmp.data[i].den;
		i++;
	}
	j1 = i;

	if (j1 - j0 > 0) {
		newHelper.key = rasterizer->eventsQueueTmp.data[(k + j1 - j0 - 1)].den.yx;
		newHelper.idx = (k + j1 - j0 - 1);
		newHelper.size = (j0 - j1);
		AM_DYNARRAY_PUSH_BACK(rasterizer->msortHelpers, AMSrfSpaceMSortHelper, newHelper)
		k -= (j0 - j1);
	}

	j0 = j1;
	curSize = 0;
		
	if (i < (AMint32)rasterizer->eventsQueueTmp.size) {
		old = rasterizer->eventsQueueTmp.data[i].den;
		goto ordering_loop;
	}

	// check for memory errors
	if (rasterizer->msortHelpers.error) {
		rasterizer->msortHelpers.error = AM_DYNARRAY_NO_ERROR;
		return AM_FALSE;
	}

	// sort "merge sort helpers" using a quicksort algorithm
	amRasMHelpersSort(rasterizer->msortHelpers.data, (AMuint32)rasterizer->msortHelpers.size);
	// save original base array
	originalMSortHelpersArray = rasterizer->msortHelpers.data;

	// now do the merging
	dstQueue = rasterizer->eventsQueue.data;
	while (rasterizer->msortHelpers.size > 0) {

		AMSrfSpaceEvent *minEvent = &rasterizer->eventsQueueTmp.data[rasterizer->msortHelpers.data[0].idx];
		// copy and store the minimum event, in its final destination
		dstQueue->den.p.x = minEvent->den.p.x;
		dstQueue->den.p.y = boxMax - minEvent->den.p.y;
		dstQueue->edge0 = minEvent->edge0;
		dstQueue->edge1 = minEvent->edge1;
		dstQueue->flags = minEvent->flags;
		dstQueue++;

		AM_ASSERT(rasterizer->msortHelpers.data[0].size != 0);

		if (rasterizer->msortHelpers.data[0].size > 0) {
			rasterizer->msortHelpers.data[0].size--;
			rasterizer->msortHelpers.data[0].idx++;
		}
		else {
			rasterizer->msortHelpers.data[0].size++;
			rasterizer->msortHelpers.data[0].idx--;
		}

		if (rasterizer->msortHelpers.data[0].size == 0) {
			// simulate a: AM_DYNARRAY_ERASE(rasterizer->msortHelpers, 0)
			rasterizer->msortHelpers.data++;
			rasterizer->msortHelpers.size--;
		}
		else {
			rasterizer->msortHelpers.data[0].key = rasterizer->eventsQueueTmp.data[rasterizer->msortHelpers.data[0].idx].den.yx;

			if (rasterizer->msortHelpers.size > 1) {

				AMuint32 count, count2;
				AMint32 i;
				AMSrfSpaceMSortHelper *mid, *first;

				first = &(rasterizer->msortHelpers.data[1]);
				count = (AMuint32)rasterizer->msortHelpers.size - 1;

				for (; 0 < count; )	{
					count2 = count >> 1;
					mid = first;
					mid += count2;
					if (mid->key < rasterizer->msortHelpers.data[0].key) {
						first = ++mid;
						count -= count2 + 1;
					}
					else
						count = count2;
				}
				i = (AMint32)(first - &(rasterizer->msortHelpers.data[1]));

				// avoid to copy the element on itself
				if (i > 0) {

					AMint32 j;
					AMSrfSpaceMSortHelper tmp = rasterizer->msortHelpers.data[0];
					AMSrfSpaceMSortHelper *data = rasterizer->msortHelpers.data;

					for (j = i; j != 0; --j) {
						data[0] = data[1];
						data++;
					}
					rasterizer->msortHelpers.data[i] = tmp;
				}
			}
		}
	}
	// restore original base array
	rasterizer->msortHelpers.data = originalMSortHelpersArray;
	return AM_TRUE;
}

/*!
	\brief Compare an edge with a vertex, along the horizontal sweepline passing through curEventPos.
	\param edge input edge to compare.
	\param curEventPos input vertex.
	\return -1 if the sweepline distance of the edge is lesser than curEventPos.x,
	+1 if the sweepline distance of the edge is greater than curEventPos.x,
	0 if the sweepline distance of the edge is equal to curEventPos.x.
	\pre curEventPos.y must be in the [edge.v0.y; edge.v1.y] range.
*/
AM_INLINE AMint32 amRasEdgeSweepCmp(const AMSrfSpaceEdge *edge,
									const AMSrfSpaceVertex *curEventPos) {

	AM_ASSERT(curEventPos->p.y <= edge->v0->p.y && curEventPos->p.y >= edge->v1->p.y);

	// edge is vertical
	if (edge->v0->p.x == edge->v1->p.x) {
		if (edge->v0->p.x < curEventPos->p.x)
			return -1;
		else
		if (edge->v0->p.x > curEventPos->p.x)
			return 1;
		else
			return 0;
	}
	else {
		if (edge->v0->p.x >= edge->v1->p.x) {
			// edge has a positive m
			if (curEventPos->p.x >= edge->v1->p.x) {
				// dy > 0, dx > 0
				AMuint32 dx = (AMuint32)edge->v0->p.x - (AMuint32)edge->v1->p.x;
				AMuint32 dy = (AMuint32)edge->v0->p.y - (AMuint32)edge->v1->p.y;
				// delta >= 0
				AMuint32 delta = (AMuint32)curEventPos->p.y - (AMuint32)edge->v1->p.y;
				AMuint32 left = delta * dx;
				AMuint32 right = dy * (AMuint32)(curEventPos->p.x - edge->v1->p.x);

				if (left < right)
					return -1;
				else
				if (left > right)
					return 1;
				else
					return 0;
			}
			else
				return 1;
		}
		else {
			// edge has a negative m, include the sign(-) inside dy
			if (curEventPos->p.x > edge->v1->p.x)
				return -1;
			else {
				// dy > 0, dx > 0
				AMuint32 dx = (AMuint32)edge->v1->p.x - (AMuint32)edge->v0->p.x;
				AMuint32 dy = (AMuint32)edge->v0->p.y - (AMuint32)edge->v1->p.y;
				// delta >= 0
				AMuint32 delta = (AMuint32)curEventPos->p.y - (AMuint32)edge->v1->p.y;
				AMuint32 left = delta * dx;
				AMuint32 right = dy * (AMuint32)(edge->v1->p.x - curEventPos->p.x);

				if (left > right)
					return -1;
				else
				if (left < right)
					return 1;
				else
					return 0;
			}
		}
	}
}

/*!
	\brief Compare two edges according to their slopes (used when amRasEdgeSweepCmp(e0, e1->v0) returns 0).
	\param e0 first input edge.
	\param e1 second input edge.
	\return -1 if e0 slope is greater than e1 slope, else +1.
*/
AMint32 amRasEdgesSlopeCmp(const AMSrfSpaceEdge *e0,
						   const AMSrfSpaceEdge *e1) {

	const AMSrfSpaceVertex *e0v1 = e0->v1;
	const AMSrfSpaceVertex *e1v0 = e1->v0;
	const AMSrfSpaceVertex *e1v1 = e1->v1;
	AMuint32 left, right;

	// NB: curEventPos MUST be equal to e0->v0 OR to e1->v0
	if (e1v1->p.x < e1v0->p.x) {
		if (e0v1->p.x < e1v0->p.x) {
			// left > 0
			left = (e1v0->p.x - e0v1->p.x) * (e1v0->p.y - e1v1->p.y);
			// right > 0
			right = (e1v0->p.x - e1v1->p.x) * (e1v0->p.y - e0v1->p.y);
			return (left > right) ? -1 : 1;
		}
		else
			return 1;
	}
	else
	if (e1v1->p.x > e1v0->p.x) {
		if (e0v1->p.x <= e1v0->p.x)
			return -1;
		else {
			// left > 0
			left = (e0v1->p.x - e1v0->p.x) * (e1v0->p.y - e1v1->p.y);
			// right > 0
			right = (e1v1->p.x - e1v0->p.x) * (e1v0->p.y - e0v1->p.y);
			return (left < right) ? -1 : 1;
		}
	}
	else {
		// e1v1.x == e1v0.x
		return (e0v1->p.x <= e1v0->p.x) ? -1 : 1;
	}
}

/*!
	\brief Compare two edges along the sweepline passing through e1.v0.
	\param e0 first input edge.
	\param e1 second input edge.
	\return -1 if the sweepline distance of e0 is lesser than the sweepline distance of e1,
	+1 if the sweepline distance of e0 is greater than the sweepline distance of e1.
	\pre e1.v0.y must be inside the [e0.v0.y; e0.v1.y] range.
	\note if sweepline distances are the same, edges are compared using their slopes.
*/
AM_INLINE AMint32 amRasEdgesCmp(const AMSrfSpaceEdge *e0,
								const AMSrfSpaceEdge *e1) {

	const AMSrfSpaceVertex *e0v0 = e0->v0;
	const AMSrfSpaceVertex *e1v0 = e1->v0;  // this is the current event position

	// edge0 v0 is equal to the event
	if (e0v0->yx == e1v0->yx)
		return amRasEdgesSlopeCmp(e0, e1);
	// edge0 v0 is not equal to the event
	else {
		AMint32 cmp = amRasEdgeSweepCmp(e0, e1v0);

		return (cmp == 0) ? amRasEdgesSlopeCmp(e0, e1) : cmp;
	}
}

/*!
	\brief Insert a specified edge inside the active edge list.
	\param rasterizer the rasterizer that contains the active edge list.
	\param edge the edge to be inserted.
	\return index inside the active edge list where the edge has been inserted.
*/
AMint32 amRasEdgeInsert(AMRasterizer *rasterizer,
						AMSrfSpaceEdge *edge) {

	AMuint32 count, count2;
	AMint32 i;
	AMSrfSpaceEdgePtr *mid, *first;

	AM_ASSERT(edge);
	AM_ASSERT(rasterizer);
	
	first = &(rasterizer->ael.data[0]);
	count = (AMuint32)rasterizer->ael.size;

	for (; 0 < count; )	{
		count2 = count >> 1;
		mid = first;
		mid += count2;
		if (amRasEdgesCmp(*mid, edge) < 0) {
			first = ++mid;
			count -= count2 + 1;
		}
		else
			count = count2;
	}
	i = (AMint32)(first - &(rasterizer->ael.data[0]));

	AM_DYNARRAY_INSERT(rasterizer->ael, AMSrfSpaceEdgePtr, i, edge)
	return i;
}

/*!
	\brief Check if two different events represent equivalent swap events.
	\param ev0 first input event.
	\param ev1 second input event.
	\return 1 if specified events represent equivalent swap events, else 0.
*/
AM_INLINE AMint32 amRasEventsSameSwap(const AMSrfSpaceEvent *ev0,
									  const AMSrfSpaceEvent *ev1) {

	if (IS_SWAP_EVENT(ev0) && IS_SWAP_EVENT(ev1)) {
		if ((ev0->edge0 == ev1->edge0 && ev0->edge1 == ev1->edge1) ||
			(ev0->edge0 == ev1->edge1 && ev0->edge1 == ev1->edge0))
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

/*!
	\brief Insert a swap event inside the sorted event queue.
	\param rasterizer rasterizer containing the event queue.
	\param ev event to be inserted.
	\param currentEventIndex index of the current event, used as the first queue element in the binary search.
*/
void amRasEventInsert(AMRasterizer *rasterizer,
					  const AMSrfSpaceEvent *ev,
					  const AMint32 currentEventIndex) {

	AMuint32 count, count2;
	AMint32 i;
	AMSrfSpaceEvent *mid, *first;
	AMint32 cmp;

	// discard the new event if geometrically it is before than the current elaborated event
	cmp = amRasEventsCmp(rasterizer, &rasterizer->eventsQueue.data[currentEventIndex], ev);
	if (cmp > 0)
		return;
	else
	// if the new event is geometrically after the current event, we can start searching insertion index
	// from currentEventIndex
	if (cmp < 0) {
		first = &(rasterizer->eventsQueue.data[currentEventIndex]);
		count = (AMuint32)rasterizer->eventsQueue.size - (AMuint32)currentEventIndex;
	}
	else {
		first = &(rasterizer->eventsQueue.data[0]);
		count = (AMuint32)rasterizer->eventsQueue.size;
	}

	for (; 0 < count; )	{
		count2 = count >> 1;
		mid = first;
		mid += count2;
		if (amRasEventsCmp(rasterizer, mid, ev) < 0) {
			first = ++mid;
			count -= count2 + 1;
		}
		else
			count = count2;
	}
	i = (AMint32)(first - &(rasterizer->eventsQueue.data[0]));

	while (i < (AMint32)rasterizer->eventsQueue.size && amRasEventsCmp(rasterizer, &rasterizer->eventsQueue.data[i], ev) == 0) {
		// we have found an identical event (NOT PREVIOUSLY DISCARDED), so skip the newEvent
		if (amRasEventsSameSwap(ev, &rasterizer->eventsQueue.data[i]) && !IS_DISCARDED_SWAP(&rasterizer->eventsQueue.data[i]))
			return;
		i++;
	}

	// now we are sure that the new event can be inserted
	AM_DYNARRAY_INSERT(rasterizer->eventsQueue, AMSrfSpaceEvent, i, *ev)
}

/*!
	\brief Check the existence of an intersection between two edges; if found, insert a new swap event in
	the event queue.
	\param rasterizer rasterizer containing the event queue.
	\param edgeSx input left edge.
	\param edgeDx input right edge.
	\param curEventIndex index of the current event.
*/
AM_INLINE void amRasEdgesIntersect(AMRasterizer *rasterizer,
								   AMSrfSpaceEdge *edgeSx,
								   AMSrfSpaceEdge *edgeDx,
								   const AMint32 curEventIndex) {

	const AMSrfSpaceVertex *p1 = edgeSx->v0;
	const AMSrfSpaceVertex *p2 = edgeSx->v1;
	const AMSrfSpaceVertex *p3 = edgeDx->v0;
	const AMSrfSpaceVertex *p4 = edgeDx->v1;

	AMint32 p2p1x = (AMint32)p2->p.x - (AMint32)p1->p.x;
	AMint32 p2p1y = (AMint32)p2->p.y - (AMint32)p1->p.y;
	AMint32 p3p1x = (AMint32)p3->p.x - (AMint32)p1->p.x;
	AMint32 p3p1y = (AMint32)p3->p.y - (AMint32)p1->p.y;
	AMint32 p4p1x = (AMint32)p4->p.x - (AMint32)p1->p.x;
	AMint32 p4p1y = (AMint32)p4->p.y - (AMint32)p1->p.y;
	AMint32 area0 = p2p1x * p3p1y - p2p1y * p3p1x;
	AMint32 area1 = p2p1x * p4p1y - p2p1y * p4p1x;
	AMint32 p2p3x, p2p3y, p4p3x, p4p3y;

	AM_ASSERT(rasterizer);
	AM_ASSERT(edgeSx);
	AM_ASSERT(edgeDx);
	AM_ASSERT(curEventIndex >= 0 && curEventIndex < (AMint32)rasterizer->eventsQueue.size);

	if ((area0 >= 0 && area1 >= 0) || (area0 <= 0 && area1 <= 0))
		return;

	p2p3x = (AMint32)p2->p.x - (AMint32)p3->p.x;
	p2p3y = (AMint32)p2->p.y - (AMint32)p3->p.y;
	p4p3x = (AMint32)p4->p.x - (AMint32)p3->p.x;
	p4p3y = (AMint32)p4->p.y - (AMint32)p3->p.y;
	area0 = p3p1x * p4p3y - p3p1y * p4p3x;
	area1 = p4p3x * p2p3y - p4p3y * p2p3x;
	if ((area0 >= 0 && area1 >= 0) || (area0 <= 0 && area1 <= 0))
		return;
	// intersection found
	else {
		AMSrfSpaceEvent newEvent;
		AMSrfSpaceIntersectionNums newEventNumerators;

		AMint64 numX = INT32_INT32_MUL(area0, p2p1x);
		AMint64 numY = INT32_INT32_MUL(area0, p2p1y);
		AMint32 den = p2p1x * p4p3y - p2p1y * p4p3x;

		if (den < 0) {
			newEventNumerators.xNum = INT64_UINT64_CAST(INT64_NEG(INT64_ADD(numX, INT32_INT32_MUL(den, p1->p.x))));
			newEventNumerators.yNum = INT64_UINT64_CAST(INT64_NEG(INT64_ADD(numY, INT32_INT32_MUL(den, p1->p.y))));
			newEvent.den.yx = (AMuint32)(-den);
			// gcc warning: comparison between signed and unsigned (do not worry about it)
			AM_ASSERT(newEvent.den.yx == (-den));
		}
		else {
			newEventNumerators.xNum = INT64_UINT64_CAST(INT64_ADD(numX, INT32_INT32_MUL(den, p1->p.x)));
			newEventNumerators.yNum = INT64_UINT64_CAST(INT64_ADD(numY, INT32_INT32_MUL(den, p1->p.y)));
			newEvent.den.yx = (AMuint32)den;
			// gcc warning: comparison between signed and unsigned (do not worry about it)
			AM_ASSERT(newEvent.den.yx == (den));
		}
		// insert the new event
		newEvent.edge0 = edgeSx;
		newEvent.edge1 = edgeDx;
		newEvent.flags = SWAP_EVENT | (AMuint32)rasterizer->intersectionNumerators.size;
		AM_DYNARRAY_PUSH_BACK(rasterizer->intersectionNumerators, AMSrfSpaceIntersectionNums, newEventNumerators)
		if (!rasterizer->intersectionNumerators.error)
		    amRasEventInsert(rasterizer, &newEvent, curEventIndex);
	}
}

/*!
	\brief Remove a specified edge from the active edge list.
	\param rasterizer rasterizer containing the active edge list.
	\param edge the edge to be removed.
	\param curEventIndex index of the current event.
*/
void amRasEdgeRemove(AMRasterizer *rasterizer,
					 const AMSrfSpaceEdge *edge,
					 const AMint32 curEventIndex) {

	AMint32 i;

	// edge MUST not be horizontal
	AM_ASSERT(edge->v0->p.y != edge->v1->p.y);

	for (i = 0; i < (AMint32)rasterizer->ael.size; ++i) {
		if (rasterizer->ael.data[i] == edge) {
			if (i - 1 >= 0 && i + 1 < (AMint32)rasterizer->ael.size)
				amRasEdgesIntersect(rasterizer, rasterizer->ael.data[i - 1], rasterizer->ael.data[i + 1], curEventIndex);
			AM_DYNARRAY_ERASE(rasterizer->ael, (AMuint32)i)
			break;
		}
	}
}

/*!
	\brief Find indexes, inside the active edge list, corresponding to two consecutive edges.
	\param i0 output index, corresponding to edge0.
	\param i1 output index, corresponding to edge1.
	\param rasterizer rasterizer containing the active edge list.
	\param edge0 first input edge.
	\param edge1 second input edge.
	\return 1 if consecutive indexes have been found, else 0.
*/
AMint32 amRasEdgesFindIndexes(AMint32 *i0,
							  AMint32 *i1,
							  AMRasterizer *rasterizer,
							  const AMSrfSpaceEdge *edge0,
							  const AMSrfSpaceEdge *edge1) {

	AMint32 j, q0 = -1;

	// dictionary must be made of at least 2 edges
	AM_ASSERT(rasterizer->ael.size >= 2);

	for (j = 0; j < (AMint32)rasterizer->ael.size; ++j) {
		if (rasterizer->ael.data[j] == edge0) {
			q0 = j;
			break;
		}
	}
	AM_ASSERT(q0 != -1);

	if (q0 == 0) {
		if (rasterizer->ael.data[1] == edge1) {
			// we have found the two consecutive edges
			*i0 = 0;
			*i1 = 1;
			return 1;
		}
		else
			return 0;
	}
	else
	if (q0 == (AMint32)rasterizer->ael.size - 1) {
		if (rasterizer->ael.data[q0 - 1] == edge1) {
			*i0 = q0 - 1;
			*i1 = q0;
			return 1;
		}
		else
			return 0;
	}
	else {
		if (rasterizer->ael.data[q0 - 1] == edge1) {
			*i0 = q0 - 1;
			*i1 = q0;
			return 1;
		}
		else
		if (rasterizer->ael.data[q0 + 1] == edge1) {
			*i0 = q0;
			*i1 = q0 + 1;
			return 1;
		}
		else
			return 0;
	}
}

/*!
	\brief Given a swap event, it swaps (inside the active edge list) the two edges that intersect at the event.
	\param rasterizer rasterizer containing the active edge list.
	\param ev input swap event.
	\param curEventIndex index of the current event.
*/
void amRasEventDoSwap(AMRasterizer *rasterizer,
					  const AMSrfSpaceEvent *ev,
					  const AMint32 curEventIndex) {

	AMint32 q0, q1;

	AM_ASSERT(ev == &rasterizer->eventsQueue.data[curEventIndex]);

	// find edges indexes, it's q0 < q1 (indexes are sorted)
	if (amRasEdgesFindIndexes(&q0, &q1, rasterizer, ev->edge0, ev->edge1)) {

		AMSrfSpaceEdge *tmpEdge;

		if (q0 - 1 >= 0)
			amRasEdgesIntersect(rasterizer, rasterizer->ael.data[q0 - 1], rasterizer->ael.data[q1], curEventIndex);
		if (q1 + 1 < (AMint32)rasterizer->ael.size)
			amRasEdgesIntersect(rasterizer, rasterizer->ael.data[q0], rasterizer->ael.data[q1 + 1], curEventIndex);

		tmpEdge = rasterizer->ael.data[q0];
		rasterizer->ael.data[q0] = rasterizer->ael.data[q1];
		rasterizer->ael.data[q1] = tmpEdge;
	}
	else
		rasterizer->eventsQueue.data[curEventIndex].flags |= DISCARDED_SWAP;
}

/*!
	\brief Span positive coverage deltas along pixels between xUp and xDown.
	\param rasterizer rasterizer containing the coverage line deltas.
	\param xUp x coordinate of the intersection between the interested edge with the slice top border.
	\param xDown x coordinate of the intersection between the interested edge with the slice bottom border.
	\param sliceHeight height of the current slice.
*/
void amRasCoverageAdd(AMRasterizer *rasterizer,
					  const AMuint16 xUp,
					  const AMuint16 xDown,
					  const AMuint16 sliceHeight) {

	AMint32 j0, j1, xLeft, xRight, n, a, b;
	AMint32 *covLine;

	if (xDown < xUp) {
		xLeft = xDown;
		xRight = xUp;
	}
	else {
		xLeft = xUp;
		xRight = xDown;
	}

	a = xLeft & AM_RAS_FIXED_MASK;
	b = xRight & AM_RAS_FIXED_MASK;
	j0 = (xLeft >> AM_RAS_FIXED_PRECISION);
	j1 = (xRight >> AM_RAS_FIXED_PRECISION);

	AM_ASSERT(j0 <= j1);
	AM_ASSERT(j0 >= 0);

	n = j1 - j0;
	covLine = &rasterizer->coverageLineDeltas[j0];

	// j0 == j1, same pixel
	if (n == 0) {

		AMint32 areaLast = (a + b) * (sliceHeight << AM_RAS_M_PRECISION);
		AMint32 areaFirst = (AM_RAS_FIXED_TWO * (sliceHeight << AM_RAS_M_PRECISION)) - areaLast;
		AMint32 tmp0, tmp1;

		// avoid read/write dependencies
		tmp0 = covLine[0];
		tmp1 = covLine[1];
		tmp0 += areaFirst;
		tmp1 += areaLast;
		// first pixel
		*covLine++ = tmp0;
		// last pixel
		*covLine = tmp1;
	}
	else {
		AMint32 m, mod, areaFirst, area, dx, tmp;

		dx = xRight - xLeft;
		if (dx < 2048) {
			m = sliceHeight * divModTable[dx].div;
			mod = sliceHeight * divModTable[dx].mod;
		}
		else {
			m = (sliceHeight << AM_RAS_M_PRECISION) / dx;
			mod = (sliceHeight << AM_RAS_M_PRECISION) % dx;
			mod *= AM_RAS_FIXED_TWO;
		}

		// first pixel
		tmp = *covLine;
		areaFirst = (AM_RAS_FIXED_ONE - a) * (AM_RAS_FIXED_ONE - a) * m;
		tmp += areaFirst;
		*covLine++ = tmp;

		n -= 2;
		if (n >= 0) {
			AMint32 mod0, mod1;
			
			// distribute module across all the pixels, in order to improve coverage distribution along the edge
			if (n < AM_RAS_MODULES_TABLE_ENTRIES) {
				mod0 = ((mod >> AM_RAS_FIXED_PRECISION) * modulesTable[n]) >> (AM_RAS_MODULES_TABLE_PRECISION - AM_RAS_FIXED_PRECISION);
				mod1 = mod - n * mod0;
			}
			else {
				mod0 = mod / n;
				mod1 = mod % n;
			}

			// second pixel
			tmp = *covLine;
			area = (AM_RAS_FIXED_ONE_SQR_TWO - a * a) * m;
			tmp += area;
			*covLine++ = tmp;
			// intermediate pixels
			area = m * AM_RAS_FIXED_ONE_SQR_TWO + mod0;
			for (; n != 0; --n)
				*covLine++ += area;
			// last but one pixel
			tmp = *covLine;
			area = (AM_RAS_FIXED_ONE_SQR_TWO - (AM_RAS_FIXED_ONE - b) * (AM_RAS_FIXED_ONE - b)) * m + mod1;
			tmp += area;
			*covLine++ = tmp;
			// last pixel
			tmp = *covLine;
			area = (m * b * b);
			tmp += area;
			*covLine = tmp;
		}
		else {
			AMint32 tmp0, tmp1;
			AMint32 areaLast = m * b * b;

			area = (AM_RAS_FIXED_TWO * (sliceHeight << AM_RAS_M_PRECISION)) - areaFirst - areaLast;
			tmp0 = covLine[0];
			tmp1 = covLine[1];
			tmp0 += area;
			tmp1 += areaLast;
			// second pixel
			*covLine++ = tmp0;
			// last pixel
			*covLine = tmp1;
		}
	}
}

/*!
	\brief Span negative coverage deltas along pixels between xUp and xDown.
	\param rasterizer rasterizer containing the coverage line deltas.
	\param xUp x coordinate of the intersection between the interested edge with the slice top border.
	\param xDown x coordinate of the intersection between the interested edge with the slice bottom border.
	\param sliceHeight height of the current slice.
*/
void amRasCoverageSub(AMRasterizer *rasterizer,
					  const AMuint16 xUp,
					  const AMuint16 xDown,
					  const AMuint16 sliceHeight) {

	AMint32 j0, j1, xLeft, xRight, n, a, b;
	AMint32 *covLine;

	if (xDown < xUp) {
		xLeft = xDown;
		xRight = xUp;
	}
	else {
		xLeft = xUp;
		xRight = xDown;
	}

	a = xLeft & AM_RAS_FIXED_MASK;
	b = xRight & AM_RAS_FIXED_MASK;
	j0 = (xLeft >> AM_RAS_FIXED_PRECISION);
	j1 = (xRight >> AM_RAS_FIXED_PRECISION);

	AM_ASSERT(j0 <= j1);
	AM_ASSERT(j0 >= 0);

	n = j1 - j0;
	covLine = &rasterizer->coverageLineDeltas[j0];

	// j0 == j1, same pixel
	if (n == 0) {

		AMint32 areaLast = (a + b) * (sliceHeight << AM_RAS_M_PRECISION);
		AMint32 areaFirst = (AM_RAS_FIXED_TWO * (sliceHeight << AM_RAS_M_PRECISION)) - areaLast;
		AMint32 tmp0, tmp1;

		// avoid read/write dependencies
		tmp0 = covLine[0];
		tmp1 = covLine[1];
		tmp0 -= areaFirst;
		tmp1 -= areaLast;
		// first pixel
		*covLine++ = tmp0;
		// last pixel
		*covLine = tmp1;
	}
	else {
		AMint32 m, mod, areaFirst, area, dx, tmp;

		dx = xRight - xLeft;
		if (dx < 2048) {
			m = sliceHeight * divModTable[dx].div;
			mod = sliceHeight * divModTable[dx].mod;
		}
		else {
			m = (sliceHeight << AM_RAS_M_PRECISION) / dx;
			mod = (sliceHeight << AM_RAS_M_PRECISION) % dx;
			mod *= AM_RAS_FIXED_TWO;
		}

		// first pixel
		tmp = *covLine;
		areaFirst = (AM_RAS_FIXED_ONE - a) * (AM_RAS_FIXED_ONE - a) * m;
		tmp -= areaFirst;
		*covLine++ = tmp;

		n -= 2;
		if (n >= 0) {
			AMint32 mod0, mod1;

			// distribute module across all the pixels, in order to improve coverage distribution along the edge
			if (n < AM_RAS_MODULES_TABLE_ENTRIES) {
				mod0 = ((mod >> AM_RAS_FIXED_PRECISION) * modulesTable[n]) >> (AM_RAS_MODULES_TABLE_PRECISION - AM_RAS_FIXED_PRECISION);
				mod1 = mod - n * mod0;
			}
			else {
				mod0 = mod / n;
				mod1 = mod % n;
			}

			// second pixel
			tmp = *covLine;
			area = (AM_RAS_FIXED_ONE_SQR_TWO - a * a) * m;
			tmp -= area;
			*covLine++ = tmp;
			// intermediate pixels
			area = m * AM_RAS_FIXED_ONE_SQR_TWO + mod0;
			for (; n != 0; --n)
				*covLine++ -= area;
			// last but one pixel
			tmp = *covLine;
			area = (AM_RAS_FIXED_ONE_SQR_TWO - (AM_RAS_FIXED_ONE - b) * (AM_RAS_FIXED_ONE - b)) * m + mod1;
			tmp -= area;
			*covLine++ = tmp;
			// last pixel
			tmp = *covLine;
			area = (m * b * b);
			tmp -= area;
			*covLine = tmp;
		}
		else {
			AMint32 tmp0, tmp1;
			AMint32 areaLast = m * b * b;

			area = (AM_RAS_FIXED_TWO * (sliceHeight << AM_RAS_M_PRECISION)) - areaFirst - areaLast;
			tmp0 = covLine[0];
			tmp1 = covLine[1];
			tmp0 -= area;
			tmp1 -= areaLast;
			// second pixel
			*covLine++ = tmp0;
			// last pixel
			*covLine = tmp1;
		}
	}
}

/*!
	\brief Compute pixel coverage deltas along the current slice, according to the specified fillrule.
	\param minX updated x coordinate of the first interested pixel in the slice.
	\param maxX updated x coordinate of the last interested pixel in the slice.
	\param rasterizer rasterizer containing active edge list.
	\param yUp y coordinate of the slice top border.
	\param yDown y coordinate of the slice bottom border.
	\param fillRule input fillrule.
	\note used by the better rasterizer.
*/
void amRasCoverageComputeBetter(AMuint16 *minX,
								AMuint16 *maxX,
								AMRasterizer *rasterizer,
								const AMuint16 yUp,
								const AMuint16 yDown,
								const VGFillRule fillRule) {

	AMint32 i;
	AMuint16 oldxUp, oldxDown;
	AMuint16 xUp, xDown;
	AMuint16 sliceHeight = yUp - yDown;

	#define FIX_ORDER \
		if (xUp < oldxUp) \
			xUp = oldxUp; \
		oldxUp = xUp; \
		if (xDown < oldxDown) \
			xDown = oldxDown; \
		oldxDown = xDown;

	AM_ASSERT(minX);
	AM_ASSERT(maxX);
	AM_ASSERT(rasterizer);
	AM_ASSERT(yUp >= yDown);
	AM_ASSERT((rasterizer->ael.size & 1) == 0);
	AM_ASSERT(rasterizer->ael.size >= 2);

	oldxUp = xUp = rasterizer->ael.data[0]->oldSweepDist;
	oldxDown = xDown = amRasSweepLineDistance(yDown, rasterizer->ael.data[0]);
	rasterizer->ael.data[0]->oldSweepDist = xDown;
	amRasCoverageAdd(rasterizer, xUp, xDown, sliceHeight);
	// update min
	if (xUp < *minX)
		*minX = xUp;
	if (xDown < *minX)
		*minX = xDown;

	if (fillRule == VG_EVEN_ODD) {

		AMint32 j = ((AMint32)rasterizer->ael.size - 2);

		for (i = 1; i < j; i += 2) {
			// sub coverage
			xUp = rasterizer->ael.data[i]->oldSweepDist;
			xDown = amRasSweepLineDistance(yDown, rasterizer->ael.data[i]);
			rasterizer->ael.data[i]->oldSweepDist = xDown;
			FIX_ORDER
			amRasCoverageSub(rasterizer, xUp, xDown, sliceHeight);
			// add coverage
			xUp = rasterizer->ael.data[i + 1]->oldSweepDist;
			xDown = amRasSweepLineDistance(yDown, rasterizer->ael.data[i + 1]);
			rasterizer->ael.data[i + 1]->oldSweepDist = xDown;
			FIX_ORDER
			amRasCoverageAdd(rasterizer, xUp, xDown, sliceHeight);
		}

		AM_ASSERT(i == j + 1);
		xUp = rasterizer->ael.data[i]->oldSweepDist;
		xDown = amRasSweepLineDistance(yDown, rasterizer->ael.data[i]);
		rasterizer->ael.data[i]->oldSweepDist = xDown;
		FIX_ORDER
		amRasCoverageSub(rasterizer, xUp, xDown, sliceHeight);
	}
	else {

		AMint32 currentSign = rasterizer->ael.data[0]->sign;

		i = 1;
		while (i < (AMint32)rasterizer->ael.size) {

			while (currentSign != 0 && i < (AMint32)rasterizer->ael.size) {
				currentSign += rasterizer->ael.data[i]->sign;
				xUp = rasterizer->ael.data[i]->oldSweepDist;
				xDown = rasterizer->ael.data[i]->oldSweepDist = amRasSweepLineDistance(yDown, rasterizer->ael.data[i]);
				i++;
			}
			FIX_ORDER
			amRasCoverageSub(rasterizer, xUp, xDown, sliceHeight);

			if (i == (AMint32)rasterizer->ael.size)
				break;
			
			currentSign = rasterizer->ael.data[i]->sign;
			xUp = rasterizer->ael.data[i]->oldSweepDist;
			xDown = amRasSweepLineDistance(yDown, rasterizer->ael.data[i]);
			rasterizer->ael.data[i]->oldSweepDist = xDown;
			FIX_ORDER
			amRasCoverageAdd(rasterizer, xUp, xDown, sliceHeight);
			i++;
		}
	}
	// update max
	if (xUp > *maxX)
		*maxX = xUp;
	if (xDown > *maxX)
		*maxX = xDown;

	#undef FIX_ORDER
}

void amRasDirectFillEdgeBetterSrc(const AMuint32 xLeft,
								  const AMuint32 xRight,
								  PIXEL_TYPE *scanLine,
								  const AMint32 addMask,
								  const AMint32 negate,
								  const AMuint32 color) {

	AMint32 int_l = xLeft >> AM_RAS_FIXED_PRECISION;
	AMint32 int_r = xRight >> AM_RAS_FIXED_PRECISION;
	AMint32 fract_l = xLeft & AM_RAS_FIXED_MASK;
	AMint32 fract_r = xRight & AM_RAS_FIXED_MASK;
	AMint32 n = int_r - int_l;

#if defined(AM_LITE_PROFILE)
	#define WRITE_AA_PIXEL_SRC(_cov) { \
		AMuint32 DcaDa = amPxlUnpack565(*scanLine); \
		AMint32 tmpCov = addMask + (((_cov) ^ (AMuint32)(-negate)) + negate); \
		AM_ASSERT((_cov) >= 0 && (_cov) <= AM_RAS_MAX_COVERAGE); \
		AM_ASSERT(tmpCov >= 0 && tmpCov <= AM_RAS_MAX_COVERAGE); \
		*scanLine = amPxlPack565(amPxlLerp(((AMuint32)tmpCov) >> AM_RAS_COVERAGE_PRECISION, DcaDa, color)); \
	}
#else
	#define WRITE_AA_PIXEL_SRC(_cov) { \
		AMuint32 DcaDa = *scanLine; \
		AMint32 tmpCov = addMask + (((_cov) ^ (AMuint32)(-negate)) + negate); \
		AM_ASSERT((_cov) >= 0 && (_cov) <= AM_RAS_MAX_COVERAGE); \
		AM_ASSERT(tmpCov >= 0 && tmpCov <= AM_RAS_MAX_COVERAGE); \
		*scanLine = amPxlLerp(((AMuint32)tmpCov) >> AM_RAS_COVERAGE_PRECISION, DcaDa, color); \
	}
#endif

	// go to the first interested pixel
	scanLine += int_l;

	if (n == 0) {
		if (((fract_l | fract_r) != 0) || (!negate)) {
		    // edge0 lives inside a pixel
		    AMint32 areaLast = (fract_l + fract_r) * (AM_RAS_FIXED_ONE << AM_RAS_M_PRECISION);
		    AMint32 cov = (AM_RAS_FIXED_TWO * (AM_RAS_FIXED_ONE << AM_RAS_M_PRECISION)) - areaLast;
		    // write antialias pixel
		    WRITE_AA_PIXEL_SRC(cov)
	    }
	}
	else {
		AMint32 m, mod, areaFirst, dx, cov;

		dx = xRight - xLeft;
		if (dx < 2048) {
			m = AM_RAS_FIXED_ONE * divModTable[dx].div;
			mod = AM_RAS_FIXED_ONE * divModTable[dx].mod;
		}
		else {
			m = (AM_RAS_FIXED_ONE << AM_RAS_M_PRECISION) / dx;
			mod = (AM_RAS_FIXED_ONE << AM_RAS_M_PRECISION) % dx;
			mod *= AM_RAS_FIXED_TWO;
		}

		// first pixel
		cov = areaFirst = (AM_RAS_FIXED_ONE - fract_l) * (AM_RAS_FIXED_ONE - fract_l) * m;
		WRITE_AA_PIXEL_SRC(cov)
		scanLine++;

		n -= 2;
		if (n >= 0) {
			AMint32 incr;

			// second pixel
			cov += (AM_RAS_FIXED_ONE_SQR_TWO - fract_l * fract_l) * m;
			WRITE_AA_PIXEL_SRC(cov)
			scanLine++;

			// intermediate pixels
			incr = m * AM_RAS_FIXED_ONE_SQR_TWO;
			for (; n != 0; --n) {
				cov += incr;
				WRITE_AA_PIXEL_SRC(cov)
				scanLine++;
			}

			// last but one pixel
			cov += (AM_RAS_FIXED_ONE_SQR_TWO - (AM_RAS_FIXED_ONE - fract_r) * (AM_RAS_FIXED_ONE - fract_r)) * m + mod;
			WRITE_AA_PIXEL_SRC(cov)
		}
		else {
			AMint32 areaLast = m * fract_r * fract_r;

			cov += (AM_RAS_FIXED_TWO * (AM_RAS_FIXED_ONE << AM_RAS_M_PRECISION)) - areaFirst - areaLast;
			WRITE_AA_PIXEL_SRC(cov)
		}
	}

	#undef WRITE_AA_PIXEL_SRC
}

void amRasDirectFillEdgeCoupleBetterSrc(AMDrawingSurface *surface,
										const AMint32 xLeft0,
										const AMint32 xRight0,
										const AMint32 xLeft1,
										const AMint32 xRight1,
										PIXEL_TYPE *scanLine,
										const AMuint32 color) {

	AMint32 cov, x, xMax, areaLast;
	AMPixelAreaBetter areaDelta0[6], areaDelta1[6];
	AMint32 int_l, int_r, fract_l, fract_r, n;
	AMuint32 e0, e1;

	#define SETUP_EDGE(_xLeft, _xRight, _areaIncr) \
		int_l = (_xLeft) >> AM_RAS_FIXED_PRECISION; \
		int_r = (_xRight) >> AM_RAS_FIXED_PRECISION; \
		fract_l = (_xLeft) & AM_RAS_FIXED_MASK; \
		fract_r = (_xRight) & AM_RAS_FIXED_MASK; \
		n = int_r - int_l; \
		if (n == 0) { \
			areaLast = (fract_l + fract_r) * (AM_RAS_FIXED_ONE << AM_RAS_M_PRECISION); \
			(_areaIncr)[0].int_x = int_l; \
			(_areaIncr)[0].covDelta = (AM_RAS_FIXED_TWO * (AM_RAS_FIXED_ONE << AM_RAS_M_PRECISION)) - areaLast; \
			(_areaIncr)[1].int_x = int_l + 1; \
			(_areaIncr)[1].covDelta = areaLast; \
			(_areaIncr)[2].int_x = int_l - 1; \
		} \
		else { \
			AMint32 m, mod, dx; \
			dx = (_xRight) - (_xLeft); \
			if (dx < 2048) { \
				m = AM_RAS_FIXED_ONE * divModTable[dx].div; \
				mod = AM_RAS_FIXED_ONE * divModTable[dx].mod; \
			} \
			else { \
				m = (AM_RAS_FIXED_ONE << AM_RAS_M_PRECISION) / dx; \
				mod = (AM_RAS_FIXED_ONE << AM_RAS_M_PRECISION) % dx; \
				mod *= AM_RAS_FIXED_TWO; \
			} \
			(_areaIncr)[0].int_x = int_l; \
			(_areaIncr)[0].covDelta = (AM_RAS_FIXED_ONE - fract_l) * (AM_RAS_FIXED_ONE - fract_l) * m; \
			n -= 2; \
			if (n > 0) { \
				AMint32 mod0, mod1; \
				if (n < AM_RAS_MODULES_TABLE_ENTRIES) { \
					mod0 = ((mod >> AM_RAS_FIXED_PRECISION) * modulesTable[n]) >> (AM_RAS_MODULES_TABLE_PRECISION - AM_RAS_FIXED_PRECISION); \
					mod1 = mod - n * mod0; \
				} \
				else { \
					mod0 = mod / n; \
					mod1 = mod % n; \
				} \
				(_areaIncr)[1].int_x = int_l + 1; \
				(_areaIncr)[1].covDelta = (AM_RAS_FIXED_ONE_SQR_TWO - fract_l * fract_l) * m; \
				(_areaIncr)[2].int_x = int_l + 2; \
				(_areaIncr)[2].covDelta = m * AM_RAS_FIXED_ONE_SQR_TWO + mod0; \
				(_areaIncr)[3].int_x = int_r; \
				(_areaIncr)[3].covDelta = (AM_RAS_FIXED_ONE_SQR_TWO - (AM_RAS_FIXED_ONE - fract_r) * (AM_RAS_FIXED_ONE - fract_r)) * m + mod1; \
				(_areaIncr)[4].int_x = int_r + 1; \
				(_areaIncr)[4].covDelta = (m * fract_r * fract_r); \
				(_areaIncr)[5].int_x = int_l - 1; \
			} \
			else \
			if (n == 0) { \
				AM_ASSERT(int_l + 2 == int_r); \
				(_areaIncr)[1].int_x = int_l + 1; \
				(_areaIncr)[1].covDelta = (AM_RAS_FIXED_ONE_SQR_TWO - fract_l * fract_l) * m; \
				(_areaIncr)[2].int_x = int_r; \
				(_areaIncr)[2].covDelta = (AM_RAS_FIXED_ONE_SQR_TWO - (AM_RAS_FIXED_ONE - fract_r) * (AM_RAS_FIXED_ONE - fract_r)) * m + mod; \
				(_areaIncr)[3].int_x = int_r + 1; \
				(_areaIncr)[3].covDelta = (m * fract_r * fract_r); \
				(_areaIncr)[4].int_x = int_l - 1; \
			} \
			else { \
				AM_ASSERT(int_l + 1 == int_r); \
				areaLast = m * fract_r * fract_r; \
				(_areaIncr)[1].int_x = int_r; \
				(_areaIncr)[1].covDelta = (AM_RAS_FIXED_TWO * (AM_RAS_FIXED_ONE << AM_RAS_M_PRECISION)) - (_areaIncr)[0].covDelta - areaLast; \
				(_areaIncr)[2].int_x = int_r + 1; \
				(_areaIncr)[2].covDelta = areaLast; \
				(_areaIncr)[3].int_x = int_l - 1; \
			} \
		}

#if defined(AM_LITE_PROFILE)
	#define WRITE_AA_PIXEL_SRC(_x, _cov) { \
		AMuint32 DcaDa = amPxlUnpack565(scanLine[(_x)]); \
		scanLine[(_x)] = amPxlPack565(amPxlLerp(((AMuint32)(_cov)) >> AM_RAS_COVERAGE_PRECISION, DcaDa, color)); \
	}
#else
	#define WRITE_AA_PIXEL_SRC(_x, _cov) { \
		AMuint32 DcaDa = scanLine[(_x)]; \
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
			if (x < areaDelta0[e0 + 1].int_x - 1)
				areaDelta0[e0].int_x++;
			else
				e0++;
		}
		// edge1 contribute
		if (x == areaDelta1[e1].int_x) {
			cov -= areaDelta1[e1].covDelta;
			if (x < areaDelta1[e1 + 1].int_x - 1)
				areaDelta1[e1].int_x++;
			else
				e1++;

			if (cov <= 0)
				continue;
		}
		// write the pixel
		WRITE_AA_PIXEL_SRC(x, cov);
	}

	#undef SETUP_EDGE
	#undef WRITE_AA_PIXEL_SRC
}

AMbool amRasDirectFillBetter(AMDrawingSurface *surface,
							 const AMRasterizer *rasterizer,
							 const AMuint16 y,
							 const VGFillRule fillRule,
							 AMPaintGen *paintGen) {

	PIXEL_TYPE *scrPixels;
	AMSrfSpaceEdgePtr *edges;
	AMuint32 i, j;
	AMuint32 xDown0, xDown1;
	AMuint32 xLeft0, xRight0, xLeft1, xRight1;
	AMuint32 int_x0, int_x1;
	AMuint16 yDown = y;
	AMuint32 int_y = yDown >> AM_RAS_FIXED_PRECISION;
	AMuint32 color = paintGen->paintColor32;

	AM_ASSERT(surface);
	AM_ASSERT(rasterizer);
	AM_ASSERT(rasterizer->ael.size >= 2);
	AM_ASSERT((rasterizer->ael.size & 1) == 0);

	edges = rasterizer->ael.data;
	// check preconditions to realize the fast fill
	if (fillRule == VG_NON_ZERO && (edges[0]->sign + edges[1]->sign) != 0)
		return AM_FALSE;

	xDown0 = amRasSweepLineDistance(yDown, edges[0]);
	xDown1 = amRasSweepLineDistance(yDown, edges[1]);
	// intersections are not safe in this fast pipeline
	if (xDown0 > xDown1)
		return AM_FALSE;

	xRight1 = AM_MAX(edges[1]->oldSweepDist, xDown1);
	int_x1 = xRight1 >> AM_RAS_FIXED_PRECISION;

	edges += 2;
	j = (AMuint32)rasterizer->ael.size >> 1;
	for (i = j - 1; i != 0; --i) {

		AMSrfSpaceEdge *edge0 = *edges++;
		AMSrfSpaceEdge *edge1 = *edges++;

		xDown0 = amRasSweepLineDistance(yDown, edge0);
		xLeft0 = AM_MIN(edge0->oldSweepDist, xDown0);
		// projections must lie in two different pixels
		int_x0 = xLeft0 >> AM_RAS_FIXED_PRECISION;
		if (int_x0 <= int_x1)
			return AM_FALSE;

		if (fillRule == VG_NON_ZERO && (edge0->sign + edge1->sign) != 0)
			return AM_FALSE;

		xDown1 = amRasSweepLineDistance(yDown, edge1);
		// intersections are not safe in this fast pipeline
		if (xDown0 > xDown1)
			return AM_FALSE;

		xRight1 = AM_MAX(edge1->oldSweepDist, xDown1);
		int_x1 = xRight1 >> AM_RAS_FIXED_PRECISION;
	}

	edges = rasterizer->ael.data;
	AM_ASSERT(int_y < (AMuint32)amSrfHeightGet(surface));
	scrPixels = amSrfPixelsGet(surface) + (amSrfHeightGet(surface) - int_y - 1) * amSrfWidthGet(surface);

	for (i = j; i != 0 ; --i) {

		AMuint32 int_r0, int_l1;
		// extract two new edges
		AMSrfSpaceEdge *edge0 = *edges++;
		AMSrfSpaceEdge *edge1 = *edges++;

		// edge 0
		xDown0 = amRasSweepLineDistance(yDown, edge0);
		if (edge0->m < 0) {
			xLeft0 = xDown0;
			xRight0 = edge0->oldSweepDist;
		}
		else {
			xLeft0 = edge0->oldSweepDist;
			xRight0 = xDown0;
		}
		int_r0 = xRight0 >> AM_RAS_FIXED_PRECISION;
		AM_ASSERT((xLeft0 >> AM_RAS_FIXED_PRECISION) <= int_r0);
		AM_ASSERT(int_r0 < (AMuint32)amSrfWidthGet(surface) + 1);
		// edge 1
		xDown1 = amRasSweepLineDistance(yDown, edge1);
		if (edge1->m < 0) {
			xLeft1 = xDown1;
			xRight1 = edge1->oldSweepDist;
		}
		else {
			xLeft1 = edge1->oldSweepDist;
			xRight1 = xDown1;
		}
		int_l1 = xLeft1 >> AM_RAS_FIXED_PRECISION;
		AM_ASSERT(int_l1 <= (xRight1 >> AM_RAS_FIXED_PRECISION));
		AM_ASSERT((xRight1 >> AM_RAS_FIXED_PRECISION) < (AMuint32)amSrfWidthGet(surface) + 1);

		if (int_r0 < int_l1) {
			// edges do not overlap (in x-direction)
			amRasDirectFillEdgeBetterSrc(xLeft0, xRight0, scrPixels, 0, 0, color);
		#if defined(AM_LITE_PROFILE)
			amMemset16(&scrPixels[int_r0 + 1], amPxlPack565(paintGen->paintColor32), int_l1 - int_r0 - 1);
		#else
			amMemset32(&scrPixels[int_r0 + 1], paintGen->paintColor32, int_l1 - int_r0 - 1);
		#endif
			amRasDirectFillEdgeBetterSrc(xLeft1, xRight1, scrPixels, AM_RAS_MAX_COVERAGE, 1, color);
		}
		else {
			// fix order due to possible imprecisions
			if (xLeft0 > xLeft1)
				xLeft0 = xLeft1;
			if (xRight0 > xRight1)
				xRight0 = xRight1;
			amRasDirectFillEdgeCoupleBetterSrc(surface, xLeft0, xRight0, xLeft1, xRight1, scrPixels, color);
		}

		edge0->oldSweepDist = xDown0;
		edge1->oldSweepDist = xDown1;
	}

	return AM_TRUE;
}

/*!
	\brief Setup all data structures (e.g. event queue) needed to start the Bentley-Ottman.
	\param rasterizer rasterizer containing Bentley-Ottman structures.
	\param clipBox input clip box.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amRasBentleyOttmanSetup(AMRasterizer *rasterizer,
							   const AMAABox2i *clipBox) {

	AMint32 i, j, k, k0;
	AMint32 edgeDy;
	AMuint16 boxMaxY;
	AMSrfSpaceVertex *v0, *v1;
	AMSrfSpaceEdge newEdge, *newEdgePtr;

	#define ADD_EDGE_M(_k0, _k1) \
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
			AM_DYNARRAY_PUSH_BACK_LIGHT(rasterizer->gel, newEdge) \
			newEdgePtr = &rasterizer->gel.data[rasterizer->gel.size - 1]; \
		} \
		else \
			newEdgePtr = NULL; \
		rasterizer->eventsQueueTmp.data[_k0].den.p.x = v0->p.x; \
		rasterizer->eventsQueueTmp.data[_k0].den.p.y = boxMaxY - v0->p.y; \
		rasterizer->eventsQueueTmp.data[_k0].edge1 = newEdgePtr; \
		rasterizer->eventsQueueTmp.data[_k0].flags = 0; \
		rasterizer->eventsQueueTmp.data[_k1].edge0 = rasterizer->eventsQueueTmp.data[_k0].edge1;

	#define ADD_EDGE_Q(_k0, _k1) \
		ADD_EDGE_M(_k0, _k1) \
		rasterizer->qsortHelpers.data[_k0].key = rasterizer->eventsQueueTmp.data[_k0].den.yx; \
		rasterizer->qsortHelpers.data[_k0].idx = _k0;


	rasterizer->boxMaxY = boxMaxY = (AMuint16)(clipBox->maxPoint.y << AM_RAS_FIXED_PRECISION);

	// allocate memory for the unsorted events queue
	if (rasterizer->eventsQueueTmp.capacity < rasterizer->vertices.size) {
		AM_DYNARRAY_CLEAR_RESERVE(rasterizer->eventsQueueTmp, AMSrfSpaceEvent, rasterizer->vertices.size)
		if (rasterizer->eventsQueueTmp.error) {
			rasterizer->eventsQueueTmp.error = AM_DYNARRAY_NO_ERROR;
			return AM_FALSE;
		}
	}
	rasterizer->eventsQueueTmp.size = rasterizer->vertices.size;
	// allocate memory for the sorted events queue
	if (rasterizer->eventsQueue.capacity < rasterizer->vertices.size) {
		AM_DYNARRAY_CLEAR_RESERVE(rasterizer->eventsQueue, AMSrfSpaceEvent, rasterizer->vertices.size)
		if (rasterizer->eventsQueue.error) {
			rasterizer->eventsQueue.error = AM_DYNARRAY_NO_ERROR;
			return AM_FALSE;
		}
	}
	rasterizer->eventsQueue.size = rasterizer->vertices.size;
	// allocate memory for the global edge list
	if (rasterizer->gel.capacity < rasterizer->vertices.size) {
		AM_DYNARRAY_CLEAR_RESERVE(rasterizer->gel, AMSrfSpaceEdge, rasterizer->vertices.size)
		if (rasterizer->gel.error) {
			rasterizer->gel.error = AM_DYNARRAY_NO_ERROR;
			return AM_FALSE;
		}
	}
	rasterizer->gel.size = 0;
	
	k = 0;
	if (rasterizer->vertices.size < 32768) {
		for (i = 0; i < (AMint32)rasterizer->contourPts.size; ++i) {
			k0 = k;
			// elaborate the i-th contour
			for (j = 0; j < rasterizer->contourPts.data[i] - 1; ++j) {
				ADD_EDGE_M(k, k + 1)
				k++;
			}
			// create last closing edge
			ADD_EDGE_M(k, k0)
			k++;
		}
		// merge sort events queue
		if (!amRasEventsMsort(rasterizer, boxMaxY))
			return AM_FALSE;
	}
	else {
		// allocate memory for qsort helpers
		if (rasterizer->qsortHelpers.capacity < rasterizer->vertices.size) {
			AM_DYNARRAY_CLEAR_RESERVE(rasterizer->qsortHelpers, AMSrfSpaceQSortHelper, rasterizer->vertices.size)
			if (rasterizer->qsortHelpers.error) {
				rasterizer->qsortHelpers.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
		}
		rasterizer->qsortHelpers.size = rasterizer->vertices.size;

		for (i = 0; i < (AMint32)rasterizer->contourPts.size; ++i) {
			k0 = k;
			// elaborate the i-th contour
			for (j = 0; j < rasterizer->contourPts.data[i] - 1; ++j) {
				ADD_EDGE_Q(k, k + 1)
				k++;
			}
			// create last closing edge
			ADD_EDGE_Q(k, k0)
			k++;
		}
		// quick sort events queue
		amRasEventsQsort(rasterizer, boxMaxY);
	}

	return AM_TRUE;

	#undef ADD_EDGE_M
	#undef ADD_EDGE_Q
}

/*!
	\brief Perform polygon drawing, using the better rasterizer.
	\param surface destination drawing surface.
	\param rasterizer rasterizer containing polygon data and related structures.
	\param paintGen paint generator structure, needed to the specified scanline filler.
	\param filler scanline filler function to use during the drawing.
	\param fillRule fill rule to use.
	\param clipBox input clipBox.
	\param trackScanlines if AM_TRUE, the rasterizer keeps track of rasterized scanlines (used later for rendering on alpha mask).
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amRasDrawBetter(AMDrawingSurface *surface,
					   AMRasterizer *rasterizer,
					   AMPaintGen *paintGen,
					   AMScanlineFillerFunction filler,
					   const VGFillRule fillRule,
					   const AMAABox2i *clipBox,
					   const AMbool trackScanlines) {

	AMint32 i, j;
	AMSrfSpaceEvent oldEvent;
	AMuint16 newY, oldY, minX, maxX;
	AMbool paintDescCanBypassFiller;

	// setup Bentley-Ottman structures (edges, events queue)
	if (!amRasBentleyOttmanSetup(rasterizer, clipBox))
		return AM_FALSE;

#if (AM_OPENVG_VERSION >= 110)
	rasterizer->globalScanlineList.size = 0;
#endif

	// ensure at least 3 events
	if (rasterizer->eventsQueue.size < 3)
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
	oldY = rasterizer->eventsQueue.data[0].den.p.y;

	// empty active edge table (dictionary)
	rasterizer->ael.size = 0;
	// empty intersection numerators array
	rasterizer->intersectionNumerators.size = 0;

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
	while (i < (AMint32)rasterizer->eventsQueue.size) {

		// scanconverter
		oldEvent = rasterizer->eventsQueue.data[i];
		if (IS_SWAP_EVENT(&oldEvent)) {

			AMSrfSpaceIntersectionNums *w = &rasterizer->intersectionNumerators.data[INTERSECTION_NUMS_IDX(&oldEvent)];

			AM_ASSERT(oldEvent.den.yx != 0);
			AM_ASSERT(!rasterizer->intersectionNumerators.error);

			newY = (AMuint16)(UINT64_UINT32_DIV(w->yNum, oldEvent.den.yx));
		}
		else
			newY = oldEvent.den.p.y;

		if (newY != oldY) {

			AMint32 y, rewind;
			AMint32 y0, y1, y0fr;

			AM_ASSERT(!rasterizer->ael.error);

reDraw:
			rewind = 0;
			y = newY;
			y0 = oldY >> AM_RAS_FIXED_PRECISION;
			y1 = y >> AM_RAS_FIXED_PRECISION;
			y0fr = oldY & AM_RAS_FIXED_MASK;

			if (y0 != y1) {
				if (y0 - y1 > 1) {
					rewind = 1;
					y = (y0 << AM_RAS_FIXED_PRECISION);
					if (y0fr == 0)
						y -= AM_RAS_FIXED_ONE;
				}
				else {
					if (y0fr != 0) {
						y = (y0 << AM_RAS_FIXED_PRECISION);
						rewind = 1;
					}
				}
			}

			if (rasterizer->ael.size > 0) {
				// number of edges must be even
				AM_ASSERT((rasterizer->ael.size & 1) == 0);
				// try to bypass the coverage calculation + filler, if possible
				if (paintDescCanBypassFiller && (oldY & AM_RAS_FIXED_MASK) == 0 && (y & AM_RAS_FIXED_MASK) == 0) {
					AM_ASSERT(minX > maxX);
					if (!amRasDirectFillBetter(surface, rasterizer, y, fillRule, paintGen))
						amRasCoverageComputeBetter(&minX, &maxX, rasterizer, oldY, y, fillRule);
				}
				else
					amRasCoverageComputeBetter(&minX, &maxX, rasterizer, oldY, y, fillRule);
			}

			oldY = y;

			if ((y & AM_RAS_FIXED_MASK) == 0) {

				if (minX <= maxX) {
					
					minX >>= AM_RAS_FIXED_PRECISION;
					maxX >>= AM_RAS_FIXED_PRECISION;

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

			// if an integer scanline was elaborated, we have to re-process this event (that is not yet processed)
			if (rewind && rasterizer->ael.size > 0)
				goto reDraw;
			else
				oldY = newY;
		}
		
		// if it's a swap event simply swap edges in the dictionary
		if (IS_SWAP_EVENT(&rasterizer->eventsQueue.data[i]))
			amRasEventDoSwap(rasterizer, &rasterizer->eventsQueue.data[i], i);
		else {
			AMint32 startIndex, endIndex, k;
			AMSrfSpaceVertex startPos;
			AMSrfSpaceEvent ev, startEvent;

			startEvent = rasterizer->eventsQueue.data[i];
			startPos = rasterizer->eventsQueue.data[i].den;
			startIndex = i;
			do {
				ev = rasterizer->eventsQueue.data[i];
				// if it's not a swap event, remove its upgoing edges
				if (!IS_SWAP_EVENT(&ev)) {
					// remove upgoing edges from dictionary
					if (ev.edge0 && amRasEdgeUpGoing(ev.edge0, &ev.den))
						amRasEdgeRemove(rasterizer, ev.edge0, i);
					if (ev.edge1 && amRasEdgeUpGoing(ev.edge1, &ev.den))
						amRasEdgeRemove(rasterizer, ev.edge1, i);
				}
				i++;
			} while (i < (AMint32)rasterizer->eventsQueue.size &&
					(!IS_SWAP_EVENT(&rasterizer->eventsQueue.data[i]) &&
					(rasterizer->eventsQueue.data[i].den.yx == startPos.yx)));
			k = i;

			// now do all swap events
			while (i < (AMint32)rasterizer->eventsQueue.size &&
				   IS_SWAP_EVENT(&rasterizer->eventsQueue.data[i]) &&
				   amRasEventsCmp(rasterizer, &startEvent, &rasterizer->eventsQueue.data[i]) == 0) {
				// this swap event is equal to geometrically startPos, so num/den is a grid vertex
				amRasEventDoSwap(rasterizer, &rasterizer->eventsQueue.data[i], i);
				i++;
			}

			endIndex = i;

			// now add all downgoing edges
			for (i = startIndex; i < k; ++i) {
				ev = rasterizer->eventsQueue.data[i];
				if (!IS_SWAP_EVENT(&ev)) {
					if (ev.edge0 && amRasEdgeDownGoing(ev.edge0, &ev.den)) {
						j = amRasEdgeInsert(rasterizer, ev.edge0);
						// check for memory errors
						if (rasterizer->ael.error)
							break;
						if (j - 1 >= 0)
							amRasEdgesIntersect(rasterizer, rasterizer->ael.data[j - 1], rasterizer->ael.data[j], i);
						if (j + 1 < (AMint32)rasterizer->ael.size)
							amRasEdgesIntersect(rasterizer, rasterizer->ael.data[j], rasterizer->ael.data[j + 1], i);
					}
					if (ev.edge1 && amRasEdgeDownGoing(ev.edge1, &ev.den)) {
						j = amRasEdgeInsert(rasterizer, ev.edge1);
						// check for memory errors
						if (rasterizer->ael.error)
							break;
						if (j - 1 >= 0)
							amRasEdgesIntersect(rasterizer, rasterizer->ael.data[j - 1], rasterizer->ael.data[j], i);
						if (j + 1 < (AMint32)rasterizer->ael.size)
							amRasEdgesIntersect(rasterizer, rasterizer->ael.data[j], rasterizer->ael.data[j + 1], i);
					}
				}
			}
			// -1 must be considered because below there is an i++
			i = endIndex - 1;
		}
		// next event
		i++;

			// check for memory errors
			if (rasterizer->ael.error || rasterizer->eventsQueue.error || rasterizer->intersectionNumerators.error) {
				rasterizer->ael.error = AM_DYNARRAY_NO_ERROR;
				rasterizer->eventsQueue.error = AM_DYNARRAY_NO_ERROR;
				rasterizer->intersectionNumerators.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
	}

	if (minX <= maxX) {
		minX >>= AM_RAS_FIXED_PRECISION;
		maxX >>= AM_RAS_FIXED_PRECISION;
		if (maxX >= amSrfWidthGet(surface)) {
			maxX = amSrfWidthGet(surface) - 1;
			if (minX >= amSrfWidthGet(surface))
				minX = amSrfWidthGet(surface) - 1;
		}
	#if (AM_OPENVG_VERSION >= 110)
		if (trackScanlines) {
			AM_DYNARRAY_PUSH_BACK_LIGHT(rasterizer->globalScanlineList, oldY >> AM_RAS_FIXED_PRECISION)
		}
	#endif
		filler(surface, paintGen, oldY >> AM_RAS_FIXED_PRECISION, minX, maxX);
	}

	return AM_TRUE;
}

#undef SWAP_EVENT
#undef DISCARDED_SWAP
#undef IS_SWAP_EVENT
#undef IS_DISCARDED_SWAP
#undef INTERSECTION_NUMS_IDX
#undef PIXEL_TYPE

#if defined (RIM_VG_SRC)
#pragma pop
#endif

