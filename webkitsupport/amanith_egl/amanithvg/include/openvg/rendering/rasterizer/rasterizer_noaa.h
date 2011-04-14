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

#ifndef _RASTERIZER_NOAA_H
#define _RASTERIZER_NOAA_H

/*!
	\file rasterizer_noaa.h
	\brief Non-antialiased rasterizer, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "rasterizer_common.h"

//! Structure containing an edge and its sweepline distance.
typedef struct _AMEdgeSweepDistance {
	//! Edge.
	AMSrfSpaceEdge *edge;
	//! Sweepline distance.
	AMuint16 sweepDist;
} AMEdgeSweepDistance;
AM_DYNARRAY_DECLARE(AMEdgeSweepDistanceDynArray, AMEdgeSweepDistance, _AMEdgeSweepDistanceDynArray)

//! Quick sort helper, when number of vertices is less than 65536.
typedef struct _AMSortGEL32 {
#if defined(AM_BIG_ENDIAN)
	// big endian machines (e.g. PowerPC).
	//! Key to use for comparisons.
	AMuint16 key;
	//! Associated index value in the global edge list.
	AMuint16 idx;
#elif defined(AM_LITTLE_ENDIAN)
	// little endian machines (e.g. Intel).
	//! Associated index value in the global edge list.
	AMuint16 idx;
	//! Key to use for comparisons.
	AMuint16 key;
#else
	#error Unreachable point, please define target machine endianess (check config.h inclusion).
#endif
} AMSortGEL32;
AM_DYNARRAY_DECLARE(AMSortGEL32DynArray, AMSortGEL32, _AMSortGEL32DynArray)

//! Quick sort helper, when number of vertices is more than 65535.
typedef struct _AMSortGEL64 {
	//! Key to use for comparisons.
	AMuint32 key;
	//! Associated index value in the global edge list.
	AMuint32 idx;
} AMSortGEL64;
AM_DYNARRAY_DECLARE(AMSortGEL64DynArray, AMSortGEL64, _AMSortGEL64DynArray)

#endif
