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
	\file rasterizer_common.c
	\brief Common rasterizer utilities, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "rasterizer_common.h"

/*!
	\brief Push a vertex belonging to a vertical clipBox boundary, taking care to split it when needed, in order
	to ensure that edges don't have dx and dy greater than AM_RAS_HALF_MAX_COORDINATE.
	\param x x coordinate of the vertex to push.
	\param y y coordinate of the vertex to push.
	\param oldPoint last pushed vertex.
	\param verticesOut array containing vertices.
	\note it's an optimized push vertex that doesn't need to split a vertical edge, in the case it
	belongs to a clipBox boundary.
*/
void amRasVertexBoundaryPush(const AM_RAS_INPUT_VERTEX_COMPONENT_TYPE x,
							 const AM_RAS_INPUT_VERTEX_COMPONENT_TYPE y,
							 AMSrfSpaceVertex *oldPoint,
							 AMSrfSpaceVertexDynArray *verticesOut) {

	AMSrfSpaceVertex newPoint;

#if defined(AM_FIXED_POINT_PIPELINE)
	newPoint.p.x = (AMuint16)x;
	newPoint.p.y = (AMuint16)y;
#else
	AM_RAS_INPUT_VERTEX_TYPE tmp;

	AM_VECT2_SET(&tmp, x, y)
	amRasVertexFtoX(&newPoint, &tmp);
#endif

	if (newPoint.p.x == oldPoint->p.x) {
		AM_DYNARRAY_PUSH_BACK(*verticesOut, AMSrfSpaceVertex, newPoint)
	}
	else {
		if (amAbs((AMint32)newPoint.p.y - (AMint32)oldPoint->p.y) >= AM_RAS_HALF_MAX_COORDINATE ||
			amAbs((AMint32)newPoint.p.x - (AMint32)oldPoint->p.x) >= AM_RAS_HALF_MAX_COORDINATE) {

			AMSrfSpaceVertex mid;

			mid.p.x	= (AMuint16)(((AMint32)newPoint.p.x + (AMint32)oldPoint->p.x) >> 1);
			mid.p.y = (AMuint16)(((AMint32)newPoint.p.y + (AMint32)oldPoint->p.y) >> 1);
			AM_DYNARRAY_PUSH_BACK(*verticesOut, AMSrfSpaceVertex, mid)
		}
		AM_DYNARRAY_PUSH_BACK(*verticesOut, AMSrfSpaceVertex, newPoint)
	}
	*oldPoint = newPoint;
}

/*!
	\brief Push a vertex taking care to split it when needed, in order to ensure that edges don't have dx and dy
	greater than AM_RAS_HALF_MAX_COORDINATE.
	\param x x coordinate of the vertex to push.
	\param y y coordinate of the vertex to push.
	\param oldPoint last pushed vertex.
	\param verticesOut array containing vertices.
*/
void amRasVertexPush(const AM_RAS_INPUT_VERTEX_COMPONENT_TYPE x,
					 const AM_RAS_INPUT_VERTEX_COMPONENT_TYPE y,
					 AMSrfSpaceVertex *oldPoint,
					 AMSrfSpaceVertexDynArray *verticesOut) {

	AMSrfSpaceVertex newPoint;

#if defined(AM_FIXED_POINT_PIPELINE)
	newPoint.p.x = (AMuint16)x;
	newPoint.p.y = (AMuint16)y;
#else
	AM_RAS_INPUT_VERTEX_TYPE tmp;

	AM_VECT2_SET(&tmp, x, y)
	amRasVertexFtoX(&newPoint, &tmp);
#endif

	if (amAbs((AMint32)newPoint.p.y - (AMint32)oldPoint->p.y) >= AM_RAS_HALF_MAX_COORDINATE || amAbs((AMint32)newPoint.p.x - (AMint32)oldPoint->p.x) >= AM_RAS_HALF_MAX_COORDINATE) {

		AMSrfSpaceVertex mid;

		mid.p.x	= (AMuint16)(((AMint32)newPoint.p.x + (AMint32)oldPoint->p.x) >> 1);
		mid.p.y = (AMuint16)(((AMint32)newPoint.p.y + (AMint32)oldPoint->p.y) >> 1);
		AM_DYNARRAY_PUSH_BACK(*verticesOut, AMSrfSpaceVertex, mid)
	}

	AM_DYNARRAY_PUSH_BACK(*verticesOut, AMSrfSpaceVertex, newPoint)
	*oldPoint = newPoint;
}

#if defined(AM_FIXED_POINT_PIPELINE)
AM_INLINE AMbool amRasClipTLess(const AMfixed num0,
								const AMfixed den0,
								const AMfixed num1,
								const AMfixed den1) {

	AMint32 sgn0, sgn1;

	AM_ASSERT(den0 != 0);
	AM_ASSERT(den1 != 0);

	sgn0 = (num0 > 0) ? ((den0 > 0) ? +1 : -1) : ((den0 > 0) ? -1 : +1);
	sgn1 = (num1 > 0) ? ((den1 > 0) ? +1 : -1) : ((den1 > 0) ? -1 : +1);

	if (sgn0 < sgn1)
		return AM_TRUE;
	if (sgn0 > sgn1)
		return AM_FALSE;

	if (den0 > 0) {
		if (den1 > 0)
			return (INT64_LESSER(INT32_INT32_MUL(num0, den1), INT32_INT32_MUL(num1, den0)));
		return (INT64_GREATER(INT32_INT32_MUL(num0, den1), INT32_INT32_MUL(num1, den0)));
	}
	else {
		if (den1 > 0)
			return (INT64_GREATER(INT32_INT32_MUL(num0, den1), INT32_INT32_MUL(num1, den0)));
		return (INT64_LESSER(INT32_INT32_MUL(num0, den1), INT32_INT32_MUL(num1, den0)));
	}
}

AM_INLINE AMbool amRasClipTLessOne(const AMfixed num,
								   const AMfixed den) {

	AM_ASSERT(den != 0);

	if (num < 0) {
		if (den < 0)
			return (num > den);
		return AM_TRUE;
	}
	else {
		if (den < 0)
			return AM_TRUE;
		return (num < den);
	}
}

AM_INLINE AMbool amRasClipTLessEqualOne(const AMfixed num,
										const AMfixed den) {

	AM_ASSERT(den != 0);

	if (num < 0) {
		if (den < 0)
			return (num >= den);
		return AM_TRUE;
	}
	else {
		if (den < 0)
			return AM_TRUE;
		return (num <= den);
	}
}

#endif

/*!
	\brief Given a vertex and a clip box, it returns the bit code associated to the position on input
	vertex respect to the clip box.
	\param p input vertex.
	\param clipBox input clip box.
	\return associated clip flags.
*/
AM_INLINE AMuint32 amRasVertexClipFlags(const AM_RAS_INPUT_VERTEX_TYPE *p,
										const AM_RAS_CLIP_BOX_TYPE *clipBox) {
	
	// Regions clip flags.
	//
	//        |        |
	//  0110  |  0010  | 0011
	//        |        |
	// -------+--------+--------
	//        |        |
	//  0100  |  0000  | 0001
	//        |        |
	// -------+--------+--------
	//        |        |
	//  1100  |  1000  | 1001
	//        |        |
	return (p->x > clipBox->maxPoint.x) | ((p->y > clipBox->maxPoint.y) << 1) |	((p->x < clipBox->minPoint.x) << 2) | ((p->y < clipBox->minPoint.y) << 3);
}

/*!
	Given an edge p0-p1 and a clip box, it clips the edge against the box using the Liang-Barsky algorithm.
	\param p0 input edge start point.
	\param p1 input edge end point.
	\param clipBox input clip box.
	\param oldPoint last pushed vertex.
	\param verticesOut array containing vertices.
*/
void amRasEdgeClip(const AM_RAS_INPUT_VERTEX_TYPE *p0,
				   const AM_RAS_INPUT_VERTEX_TYPE *p1,
				   const AM_RAS_CLIP_BOX_TYPE *clipBox,
				   AMSrfSpaceVertex *oldPoint,
				   AMSrfSpaceVertexDynArray *verticesOut) {

#if defined(AM_FIXED_POINT_PIPELINE)
	AMfixed deltax, deltay, xin, xout, yin, yout;
	AMfixed tinxNum, tinyNum, tin1Num, tin1Den, tin2Num, tin2Den;

	deltax = p1->x - p0->x;
	if (deltax == 0)
		deltax = (p0->x > clipBox->minPoint.x ? -1 : 1);
	if (deltax > 0) {
		xin = clipBox->minPoint.x;
		xout = clipBox->maxPoint.x;
	}
	else {
		xin = clipBox->maxPoint.x;
		xout = clipBox->minPoint.x;
	}
	tinxNum = xin - p0->x;

	deltay = p1->y - p0->y;
	if (deltay == 0)
		deltay = (p0->y > clipBox->minPoint.y ? -1 : 1);
	if (deltay > 0) {
		yin = clipBox->minPoint.y;
		yout = clipBox->maxPoint.y;
	}
	else {
		yin = clipBox->maxPoint.y;
		yout = clipBox->minPoint.y;
	}
	tinyNum = yin - p0->y;

	// tinx < tiny
	if (amRasClipTLess(tinxNum, deltax, tinyNum, deltay)) {
		tin1Num = tinxNum;
		tin1Den = deltax;
		tin2Num = tinyNum;
		tin2Den = deltay;
	}
	else {
		tin1Num = tinyNum;
		tin1Den = deltay;
		tin2Num = tinxNum;
		tin2Den = deltax;
	}

	// tin1 <= 1
	if (amRasClipTLessEqualOne(tin1Num, tin1Den)) {
		// tin1 > 0
		if (INT64_GREATER_ZERO(INT32_INT32_MUL(tin1Num, tin1Den)))
			// turning vertex
			amRasVertexBoundaryPush(xin, yin, oldPoint, verticesOut);

		// tin2 <= 1
		if (amRasClipTLessEqualOne(tin2Num, tin2Den)) {

			AMfixed toutxNum = (xout - p0->x);
			AMfixed toutyNum = (yout - p0->y);
			AMfixed tout1Num, tout1Den;
			AMbool tin2Greater0 = (INT64_GREATER_ZERO(INT32_INT32_MUL(tin2Num, tin2Den)));
			AMbool toutxLessTouty = amRasClipTLess(toutxNum, deltax, toutyNum, deltay);

			if (toutxLessTouty) {
				tout1Num = toutxNum;
				tout1Den = deltax;
			}
			else {
				tout1Num = toutyNum;
				tout1Den = deltay;
			}

			// ((tin2 > 0) || (tout1 > 0))
			if (tin2Greater0 || (INT64_GREATER_ZERO(INT32_INT32_MUL(tout1Num, tout1Den)))) {
				// tin2 <= tout1
				if (!amRasClipTLess(tout1Num, tout1Den, tin2Num, tin2Den)) {
					// visible segment
					// tin2 > 0
					if (tin2Greater0) {
						// p0 outside window
						// tinx > tiny
						if (amRasClipTLess(tinyNum, deltay, tinxNum, deltax))
							// vertical boundary
							amRasVertexBoundaryPush(xin, p0->y + (AMfixed)(INT64_INT32_DIV(INT32_INT32_MUL(tinxNum, deltay), deltax)), oldPoint, verticesOut);
						else
							// horizontal boundary
							amRasVertexPush(p0->x + (AMfixed)(INT64_INT32_DIV(INT32_INT32_MUL(tinyNum, deltax), deltay)), yin, oldPoint, verticesOut);
					}

					// tout1 < 1
					if (amRasClipTLessOne(tout1Num, tout1Den)) {
						// p1 outside window
						// toutx < touty
						if (toutxLessTouty)
							// vertical boundary
							amRasVertexBoundaryPush(xout, p0->y + (AMfixed)(INT64_INT32_DIV(INT32_INT32_MUL(toutxNum, deltay), deltax)), oldPoint, verticesOut);
						else
							// horizontal boundary
							amRasVertexPush(p0->x + (AMfixed)(INT64_INT32_DIV(INT32_INT32_MUL(toutyNum, deltax), deltay)), yout, oldPoint, verticesOut);
					}
					else {
						if (amRasVertexClipFlags(p1, clipBox) == 0)
						// p1 inside window
						amRasVertexPush(p1->x, p1->y, oldPoint, verticesOut);
				}
				}
				else {
					// turning vertex
					// tinx > tiny
					if (amRasClipTLess(tinyNum, deltay, tinxNum, deltax))
						// second entry at x
						amRasVertexBoundaryPush(xin, yout, oldPoint, verticesOut);
					else
						// second entry at y
						amRasVertexBoundaryPush(xout, yin, oldPoint, verticesOut);
				}
			}
		}
	}
#else
	AMfloat deltax, deltay, xin, xout, yin, yout;
	AMfloat tinx, tiny, tin1, tin2;

	deltax = p1->x - p0->x;
	if (deltax == 0.0f)
		deltax = (p0->x > clipBox->minPoint.x ? -AM_EPSILON_FLOAT : AM_EPSILON_FLOAT);
	if (deltax > 0.0f) {
		xin = clipBox->minPoint.x;
		xout = clipBox->maxPoint.x;
	}
	else {
		xin = clipBox->maxPoint.x;
		xout = clipBox->minPoint.x;
	}

	deltay = p1->y - p0->y;
	if (deltay == 0.0f)
		deltay = (p0->y > clipBox->minPoint.y ? -AM_EPSILON_FLOAT : AM_EPSILON_FLOAT);
	if (deltay > 0.0f) {
		yin = clipBox->minPoint.y;
		yout = clipBox->maxPoint.y;
	}
	else {
		yin = clipBox->maxPoint.y;
		yout = clipBox->minPoint.y;
	}

	tinx = (xin - p0->x) / deltax;
	tiny = (yin - p0->y) / deltay;
	if (tinx < tiny) {
		tin1 = tinx;
		tin2 = tiny;
	}
	else {
		tin1 = tiny;
		tin2 = tinx;
	}

	if (tin1 <= 1.0f) {
		if (0.0f < tin1)
			// turning vertex
			amRasVertexBoundaryPush(xin, yin, oldPoint, verticesOut);

		if (tin2 <= 1.0f) {

			AMfloat toutx = (xout - p0->x) / deltax;
			AMfloat touty = (yout - p0->y) / deltay;
			AMfloat tout1 = (toutx < touty) ? toutx : touty;

			if ((tin2 > 0.0f) || (tout1 > 0.0f)) {
				if (tin2 <= tout1) {
					// visible segment
					if (tin2 > 0.0f) {
						// p0 outside window
						if (tinx > tiny)
							// vertical boundary
							amRasVertexBoundaryPush(xin, p0->y + tinx * deltay, oldPoint, verticesOut);
						else
							// horizontal boundary
							amRasVertexPush(p0->x + tiny * deltax, yin, oldPoint, verticesOut);
					}

					if (tout1 < 1.0f) {
						// p1 outside window
						if (toutx < touty)
							// vertical boundary
							amRasVertexBoundaryPush(xout, p0->y + toutx * deltay, oldPoint, verticesOut);
						else
							// horizontal boundary
							amRasVertexPush(p0->x + touty * deltax, yout, oldPoint, verticesOut);
					}
					else
						// p1 inside window
						amRasVertexPush(p1->x, p1->y, oldPoint, verticesOut);
				}
				else {
					// turning vertex
					if (tinx > tiny)
						// second entry at x
						amRasVertexBoundaryPush(xin, yout, oldPoint, verticesOut);
					else
						// second entry at y
						amRasVertexBoundaryPush(xout, yin, oldPoint, verticesOut);
				}
			}
		}
	}
#endif
}

/*!
	\brief Find the first vertex to start polygon clipping, given an input polygon vertex.
	If the input vertex lies inside the clipBox, it returns the vertex itself, else it returns the
	nearest clipBox corner.
	\param res output vertex.
	\param p input vertex.
	\param clipBox input clip box.
*/
void amRasVertexFindFirst(AM_RAS_INPUT_VERTEX_TYPE *res,
						  const AM_RAS_INPUT_VERTEX_TYPE *p,
						  const AM_RAS_CLIP_BOX_TYPE *clipBox) {

	if (p->x >= clipBox->minPoint.x) {
		if (p->x <= clipBox->maxPoint.x) {
			if (p->y >= clipBox->minPoint.y) {
				if (p->y <= clipBox->maxPoint.y) {
					*res = *p;
					return;
				}
				else {
					AM_VECT2_SET(res, clipBox->minPoint.x, clipBox->maxPoint.y)
					return;
				}
			}
			else {
				AM_VECT2_SET(res, clipBox->maxPoint.x, clipBox->minPoint.y)
				return;
			}
		}
		else {
			AM_VECT2_SET(res, clipBox->maxPoint.x, clipBox->minPoint.y)
			return;
		}
	}
	else {
		AM_VECT2_SET(res, clipBox->minPoint.x, clipBox->maxPoint.y)
		return;
	}
}

/*!
	\brief Liang-Barsky polygon clipper.
	\param verticesIn input polygon vertices.
	\param contourPtsIn input points per contour, for each sub-contour.
	\param verticesOut output (clipped) vertices.
	\param contourPtsOut output points per contour, for each sub-contour.
	\param matrix input transformation matrix.
	\param clipBox input clip box.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amRasPolygonTransformAndClip(AMSrfSpaceVertexDynArray *verticesOut,
								  AMInt32DynArray *contourPtsOut,
								  const AM_RAS_INPUT_VERTICES_ARRAY_TYPE *verticesIn,
								  const AMInt32DynArray *contourPtsIn,
								  const AM_RAS_MATRIX_TYPE *matrix,
								  const AMAABox2i *clipBox) {

	AMint32 i, j, k, q, oldSize, newSize;
	AMSrfSpaceVertex oldPoint;
	AMuint32 curFlags, newFlags;
	AM_RAS_INPUT_VERTEX_TYPE p0, p1, firstPoint, lastPoint;
	AM_RAS_CLIP_BOX_TYPE tmpClipBox;
	const AM_RAS_CLIP_BOX_TYPE *actualClipBox = &tmpClipBox;

#if defined(AM_FIXED_POINT_PIPELINE)
	AM_VECT2_SET(&tmpClipBox.minPoint, (AM_RAS_INPUT_VERTEX_COMPONENT_TYPE)(clipBox->minPoint.x << AM_RAS_FIXED_PRECISION), (AM_RAS_INPUT_VERTEX_COMPONENT_TYPE)(clipBox->minPoint.y << AM_RAS_FIXED_PRECISION))
	AM_VECT2_SET(&tmpClipBox.maxPoint, (AM_RAS_INPUT_VERTEX_COMPONENT_TYPE)(clipBox->maxPoint.x << AM_RAS_FIXED_PRECISION), (AM_RAS_INPUT_VERTEX_COMPONENT_TYPE)(clipBox->maxPoint.y << AM_RAS_FIXED_PRECISION))
#else
	AM_VECT2_SET(&tmpClipBox.minPoint, (AM_RAS_INPUT_VERTEX_COMPONENT_TYPE)clipBox->minPoint.x, (AM_RAS_INPUT_VERTEX_COMPONENT_TYPE)clipBox->minPoint.y)
	AM_VECT2_SET(&tmpClipBox.maxPoint, (AM_RAS_INPUT_VERTEX_COMPONENT_TYPE)clipBox->maxPoint.x, (AM_RAS_INPUT_VERTEX_COMPONENT_TYPE)clipBox->maxPoint.y)
#endif

	verticesOut->size = 0;
	contourPtsOut->size = 0;
	oldSize = 0;
	q = 0;
	for (i = 0; i < (AMint32)contourPtsIn->size; ++i) {
		
		// i-th contour is made of j vertices
		j = contourPtsIn->data[i];
		// process the contour
		amRasVertexTransform(&lastPoint, &verticesIn->data[q + j - 1], matrix);
		
		// find a useful oldPoint
		amRasVertexFindFirst(&firstPoint, &lastPoint, actualClipBox);
	#if defined(AM_FIXED_POINT_PIPELINE)
		oldPoint.p.x = (AMuint16)firstPoint.x;
		oldPoint.p.y = (AMuint16)firstPoint.y;
	#else
		amRasVertexFtoX(&oldPoint, &firstPoint);
	#endif
		// tag the first point as "completely inside"
		curFlags = amRasVertexClipFlags(&lastPoint, actualClipBox);
		// start processing the last edge
		p0 = lastPoint;
		for (k = 0; k < j - 1; ++k) {
			// transform the point
			amRasVertexTransform(&p1, &verticesIn->data[q + k], matrix);
			// flag the point
			newFlags = amRasVertexClipFlags(&p1, actualClipBox);
			if (newFlags == curFlags) {
				if (curFlags == 0)
					// new point is inside, push it and update oldPoint
					amRasVertexPush(p1.x, p1.y, &oldPoint, verticesOut);
			}
			else {
				amRasEdgeClip(&p0, &p1, actualClipBox, &oldPoint, verticesOut);
				curFlags = newFlags;
			}
			// next segment
			p0 = p1;
		}

		// process last segment
		p1 = lastPoint;
		newFlags = amRasVertexClipFlags(&p1, actualClipBox);
		if (newFlags == curFlags) {
			if (curFlags == 0)
				// new point is inside, push it and update oldPoint
				amRasVertexPush(p1.x, p1.y, &oldPoint, verticesOut);
		}
		else
			amRasEdgeClip(&p0, &p1, actualClipBox, &oldPoint, verticesOut);

		// check for degenerated contours
		if ((AMint32)verticesOut->size - oldSize < 3)
			// simulate a deletion
			verticesOut->size = oldSize;
		else {
			newSize = (AMint32)verticesOut->size;
			if (newSize > oldSize) {
				AM_DYNARRAY_PUSH_BACK(*contourPtsOut, AMint32, newSize - oldSize)
			}
			oldSize = newSize;
		}
		// next contour
		q += j;
	}

	if (verticesOut->error || contourPtsOut->error) {
		verticesOut->error = AM_DYNARRAY_NO_ERROR;
		contourPtsOut->error = AM_DYNARRAY_NO_ERROR;
		return AM_FALSE;
	}
	return AM_TRUE;
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

