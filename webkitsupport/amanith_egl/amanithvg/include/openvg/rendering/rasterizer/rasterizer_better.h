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

#ifndef _RASTERIZER_BETTER_H
#define _RASTERIZER_BETTER_H

/*!
	\file rasterizer_better.h
	\brief Better rasterizer, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "rasterizer_common.h"

//! Merge sort helper structure, used to sort Bentley-Ottman events.
typedef struct _AMSrfSpaceMSortHelper {
	//! Key value (containing concatenated yx coordinates, in order to perform 32bit integer comparison).
	AMuint32 key;
	//! Index in the global unsorted events queue.
	AMuint16 idx;
	//! Size of the sub-ordered-array (descending/ascending chain).
	AMint16 size;
} AMSrfSpaceMSortHelper;
AM_DYNARRAY_DECLARE(AMSrfSpaceMSortHelperDynArray, AMSrfSpaceMSortHelper, _AMSrfSpaceMSortHelperDynArray)

//! Quick sort helper structure, used to sort Bentley-Ottman events.
typedef struct _AMSrfSpaceQSortHelper {
	//! Key value (containing concatenated yx coordinates, in order to perform 32bit integer comparison).
	AMuint32 key;
	//! Index in the global unsorted events queue.
	AMuint32 idx;
} AMSrfSpaceQSortHelper;
AM_DYNARRAY_DECLARE(AMSrfSpaceQSortHelperDynArray, AMSrfSpaceQSortHelper, _AMSrfSpaceQSortHelperDynArray)

//! Numerators (x, y) of an intersection point. Intersection points are represented in rational integer arithmetic.
typedef struct _AMSrfSpaceIntersectionNums {
	//! Numerator of the x coordinate.
	AMuint64 xNum;
	//! Numerator of the y coordinate.
	AMuint64 yNum;
} AMSrfSpaceIntersectionNums;
AM_DYNARRAY_DECLARE(AMSrfSpaceIntersectionNumsDynArray, AMSrfSpaceIntersectionNums, _AMSrfSpaceIntersectionNumsDynArray)

//! Bentley-Ottman event structure, in drawing surface space.
typedef struct _AMSrfSpaceEvent {
	//! For original events it's the position (x, y); for intersection events it's the denominator (intersection points are represented in rational integer arithmetic).
	AMSrfSpaceVertex den;
	//! For original events it's the first connected edge; for intersection events it's the first edge to swap.
	AMSrfSpaceEdge *edge0;
	//! For original events it's the second connected edge; for intersection events it's the second edge to swap.
	AMSrfSpaceEdge *edge1;
	/*!
		Intersection events only; the higher 2 bits are used to classify the event type (SWAP_EVENT | DISCARDED_SWAP),
		other bits are used to store the index to lookup the numerators array.
	*/
	AMuint32 flags;
} AMSrfSpaceEvent;
AM_DYNARRAY_DECLARE(AMSrfSpaceEventDynArray, AMSrfSpaceEvent, _AMSrfSpaceEventDynArray)

#endif
