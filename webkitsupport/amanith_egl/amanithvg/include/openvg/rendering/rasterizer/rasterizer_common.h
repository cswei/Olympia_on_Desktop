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

#ifndef _RASTERIZER_COMMON_H
#define _RASTERIZER_COMMON_H

/*!
	\file rasterizer_common.h
	\brief Common rasterizer utilities, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "aabox.h"
#include "dynarray.h"
#include "int64_math.h"

// fixed point precision constants
#if AM_SURFACE_MAX_DIMENSION > 4096 
	#error Drawing surface too big for the rasterizer
#elif AM_SURFACE_MAX_DIMENSION > 2048
	//! Drawing surface space coordinates represented in 12.4
	#define AM_RAS_FIXED_PRECISION	4
	//! Number of entries inside the modulesTable
	#define AM_RAS_MODULES_TABLE_ENTRIES 128
#elif AM_SURFACE_MAX_DIMENSION > 1024
	//! Drawing surface space coordinates represented in 11.5
	#define AM_RAS_FIXED_PRECISION	5
	//! Number of entries inside the modulesTable
	#define AM_RAS_MODULES_TABLE_ENTRIES 64
#else
	//! Drawing surface space coordinates represented in 10.6
	#define AM_RAS_FIXED_PRECISION	6
	//! Number of entries inside the modulesTable
	#define AM_RAS_MODULES_TABLE_ENTRIES 32
#endif

//! Maximum coverage of a single pixel (2^30).
#define AM_RAS_MAX_COVERAGE			1073741824
//! Right shift to get an 8 bit coverage (30 - 8 = 22).
#define AM_RAS_COVERAGE_PRECISION	22
//! 0.5 in AM_RAS_FIXED_PRECISION (fixed point).
#define AM_RAS_FIXED_HALF			(1 << (AM_RAS_FIXED_PRECISION - 1))
//! 1.0 in AM_RAS_FIXED_PRECISION (fixed point).
#define AM_RAS_FIXED_ONE			(1 << AM_RAS_FIXED_PRECISION)
//! 2.0 in AM_RAS_FIXED_PRECISION (fixed point).
#define AM_RAS_FIXED_TWO			(2 * AM_RAS_FIXED_ONE)
//! Area of a unit square, in AM_RAS_FIXED_PRECISION (fixed point).
#define AM_RAS_FIXED_ONE_SQR		(AM_RAS_FIXED_ONE * AM_RAS_FIXED_ONE)
//! Twice area of a unit square, in AM_RAS_FIXED_PRECISION (fixed point).
#define AM_RAS_FIXED_ONE_SQR_TWO	(2 * AM_RAS_FIXED_ONE_SQR)
//! Mask to extract the lower AM_RAS_FIXED_PRECISION bits.
#define AM_RAS_FIXED_MASK			((1 << AM_RAS_FIXED_PRECISION) - 1)
//! Mask to extract the higher (16 - AM_RAS_FIXED_PRECISION) bits.
#define AM_RAS_INT_MASK				(AM_MAX_UINT32 - AM_RAS_FIXED_MASK)
//! Factor to convert a float number in AM_RAS_FIXED_PRECISION representation.
#define AM_RAS_VERTEX_FTOX			((AMfloat)(1 << AM_RAS_FIXED_PRECISION))
//! Maximum, positive, 16 bit fixed point coordinate.
#define AM_RAS_MAX_COORDINATE		0xFFE0
//! Half maximum, positive, 16 bit fixed point coordinate.
#define AM_RAS_HALF_MAX_COORDINATE	0x7FF0
//! Precision bits to represent edge slopes.
#define AM_RAS_M_PRECISION			(29 - 2 * AM_RAS_FIXED_PRECISION)
//! Precision bits of modulesTable entries.
#define AM_RAS_MODULES_TABLE_PRECISION 19

//! Fixed point vertex, in drawing surface space.
typedef union _AMSrfSpaceVertex {
	struct {
	#if defined(AM_BIG_ENDIAN)
		// big endian machines (e.g. PowerPC).
		//! 16bit y component.
		AMuint16 y;
		//! 16bit x component.
		AMuint16 x;
	#elif defined(AM_LITTLE_ENDIAN)
		// little endian machines (e.g. Intel).
		//! 16bit x component.
		AMuint16 x;
		//! 16bit y component.
		AMuint16 y;
	#else
		#error Unreachable point, please define target machine endianess (check config.h inclusion).
	#endif
	} p;
	//! Alias to refer the whole 32bit vertex value.
	AMuint32 yx;
} AMSrfSpaceVertex;
AM_DYNARRAY_DECLARE(AMSrfSpaceVertexDynArray, AMSrfSpaceVertex, _AMSrfSpaceVertexDynArray)

//! Polygon edge, in drawing surface space.
typedef struct _AMSrfSpaceEdge {
	//! Pointer to the upper edge vertex.
	AMSrfSpaceVertex *v0;
	//! Pointer to the lower edge vertex.
	AMSrfSpaceVertex *v1;
	//! Original edge direction (+1 = upward, -1 = downward).
	AMint16 sign;
	//! Old (previous scanline) sweep line distance.
	AMuint16 oldSweepDist;
	//! Edge slope: dx / dy.
	AMint32 m;
} AMSrfSpaceEdge, *AMSrfSpaceEdgePtr;
AM_DYNARRAY_DECLARE(AMSrfSpaceEdgeDynArray, AMSrfSpaceEdge, _AMSrfSpaceEdgeDynArray)
AM_DYNARRAY_DECLARE(AMSrfSpaceEdgePtrDynArray, AMSrfSpaceEdgePtr, _AMSrfSpaceEdgePtrDynArray)

/*!
	\brief Vertex coordinates conversion, from float to fixed point.
	\param dst output (fixed point) vertex, in drawing surface space.
	\param src input (float) vertex, in drawing surface space.
*/
AM_INLINE void amRasVertexFtoX(AMSrfSpaceVertex *dst,
							   const AMVect2f *src) {

	dst->p.x = (AMuint16)(src->x * AM_RAS_VERTEX_FTOX);
	dst->p.y = (AMuint16)(src->y * AM_RAS_VERTEX_FTOX);
}

/*!
	\brief Lexicographic vertex comparison, in drawing surface space.
	\param v0 first input vertex.
	\param v1 second input vertex.
	\return -1 if v0 comes before v1, else +1.
*/
AM_INLINE AMint32 amRasVertexCmp(const AMSrfSpaceVertex *v0,
								 const AMSrfSpaceVertex *v1) {

	AM_ASSERT(v0);
	AM_ASSERT(v1);
	AM_ASSERT(v0->p.y != v1->p.y);

	return (v0->yx < v1->yx) ? 1 : -1;
}

/*!
	\brief Distance along x direction of the specified edge, at the specified y.
	\param y y coordinate where to evaluate distance.
	\param edge edge whose distance is to evaluate. 
	\return distance along x direction.
	\pre y must be inside [edge.v0.y; edge.v1.y] range.
*/
AM_INLINE AMuint16 amRasSweepLineDistance(const AMuint16 y,
										  const AMSrfSpaceEdge *edge) {

	AMuint16 dy;

	AM_ASSERT(y >= edge->v1->p.y && y <= edge->v0->p.y);
	dy = edge->v0->p.y - y;
	return edge->v0->p.x + ((edge->m * dy) >> 15);
}

/*!
	\brief Check if and edge has 0 length.
	\param edge the edge to check.
	\return 1 if the edge has 0 length, else 0.
*/
AM_INLINE AMint32 amRasEdgeZeroLength(const AMSrfSpaceEdge *edge) {

	AM_ASSERT(edge);

	return (edge->v0->yx == edge->v1->yx) ? 1 : 0;
}

/*!
	\brief Transform an object space vertex applying the given affine matrix.
	\param dst output transformed vertex.
	\param src input object space vertex.
	\param matrix the affine matrix to aplly.
*/
AM_INLINE void amRasVertexTransform(AM_RAS_INPUT_VERTEX_TYPE *dst,
									const AM_RAS_INPUT_VERTEX_TYPE *src,
									const AM_RAS_MATRIX_TYPE *matrix) {

#if defined(AM_FIXED_POINT_PIPELINE)
	AMint64 xa = INT32_INT32_MUL(matrix->a[0][0], src->x);
	AMint64 xb = INT32_INT32_MUL(matrix->a[0][1], src->y);
	AMint64 ya = INT32_INT32_MUL(matrix->a[1][0], src->x);
	AMint64 yb = INT32_INT32_MUL(matrix->a[1][1], src->y);
	//AMint64 x = (((xa + xb) >> 16) + matrix->a[0][2]) >> (15 - AM_RAS_FIXED_PRECISION);
	//AMint64 y = (((ya + yb) >> 16) + matrix->a[1][2]) >> (15 - AM_RAS_FIXED_PRECISION);
	AMint64 x = INT64_RSHIFT(INT64_INT32_ADD(INT64_RSHIFT(INT64_ADD(xa, xb), 16), matrix->a[0][2]), 15 - AM_RAS_FIXED_PRECISION);
	AMint64 y = INT64_RSHIFT(INT64_INT32_ADD(INT64_RSHIFT(INT64_ADD(ya, yb), 16), matrix->a[1][2]), 15 - AM_RAS_FIXED_PRECISION);
	AM_VECT2_SET(dst, (AMfixed)(INT64_INT32_CAST(x)), (AMfixed)(INT64_INT32_CAST(y)))
#else
	AM_MATRIX33_VECT2_MUL(dst, matrix, src)
#endif
}

// Liang-Barsky polygon clipper.
AMbool amRasPolygonTransformAndClip(AMSrfSpaceVertexDynArray *verticesOut,
								  AMInt32DynArray *contourPtsOut,
								  const AM_RAS_INPUT_VERTICES_ARRAY_TYPE *verticesIn,
								  const AMInt32DynArray *contourPtsIn,
								  const AM_RAS_MATRIX_TYPE *matrix,
								  const AMAABox2i *clipBox);

#endif
