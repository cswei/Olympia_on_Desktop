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
	\file vgstroke.c
	\brief Stroking functions, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgcontext.h"
#include "intersect.h"

#if defined(AM_FIXED_POINT_PIPELINE)
	#define AM_STROKE_PUSH_POINT(_arrayDst, _newPoint) \
	{ \
		AMVect2x tmpPoint; \
		AM_VECT2_SET(&tmpPoint, amFloatToFixed1616((_newPoint).x), amFloatToFixed1616((_newPoint).y)) \
		AM_DYNARRAY_PUSH_BACK((_arrayDst), AMVect2x, tmpPoint) \
	}
#else
	#define AM_STROKE_PUSH_POINT(_arrayDst, _newPoint) AM_DYNARRAY_PUSH_BACK((_arrayDst), AMVect2f, (_newPoint))
#endif


// Pixel threshold under which the stroker doesn't produce intersections (not safe stroking).
#define AM_STROKE_WIDTH_NO_INTERSECTION_THRESHOLD	1.5f

//! Descriptor for a dashing segment.
typedef struct _AMDashSeg {
	//! Index, inside the flattening array, corresponding to the dash segment start.
	AMint32 k0;
	//! Index, inside the flattening array, corresponding to the dash segment end (k1 = (k0 + 1) % flattening.size).
	AMint32 k1;
	//! Normalized direction of this segment.
	AMVect2f normDir;
} AMDashSeg;

/*!
	\brief Check if there was a memory error during stroke generation.
*/
AM_INLINE AMbool amStrokeMemoryError(AMContext *context) {

	// check for memory errors
	if (context->strokeAuxPts.error || context->strokeAuxPtsDx.error || context->strokeAuxPtsPerContour.error)
		return AM_TRUE;
	return AM_FALSE;
}

AM_INLINE void amStrokeMemoryErrorReset(AMContext *context) {

	context->strokeAuxPts.error = AM_DYNARRAY_NO_ERROR;
	context->strokeAuxPtsDx.error = AM_DYNARRAY_NO_ERROR;
	context->strokeAuxPtsPerContour.error = AM_DYNARRAY_NO_ERROR;
}

/*!
	\brief Calculate an hash value associated with the given dash pattern.
	\param dashPattern input dash pattern.
	\return hash value associated with \a dashPattern.
*/
AM_INLINE AMuint32 amStrokeDashPatternHash(const AMFloatDynArray *dashPattern) {

	return amHashCalculate(dashPattern->data, (AMuint32)dashPattern->size);
}

/*!
	\brief Check is one or more stroke parameters have been changed from previously stored ones.
	\param strokeCacheDesc input/output descriptor that contains a set of stroke parameters.
	\param _context pointer to a AMContext structure, containing the actual OpenVG stroke parameters and patched dash pattern.
	\return AM_TRUE is one or more stroke parameters have been changed, else AM_FALSE.
	\note if stroke parameters have been changed, \a strokeCacheDesc is updated according to new stroke values.
*/
AMbool amStrokeChanged(AMStrokeCacheDesc *strokeCacheDesc,
					   const void *_context) {

	const AMContext *context = (const AMContext *)_context;
	AMuint32 tmpHash;

	AM_ASSERT(strokeCacheDesc);
	AM_ASSERT(context);

	tmpHash = amStrokeDashPatternHash(&context->patchedDashPattern);

	if (strokeCacheDesc->dashPatternSize != (VGuint)context->patchedDashPattern.size ||
	#if defined(VG_MZT_separable_cap_style)
		strokeCacheDesc->startCapStyle != context->startCapStyle ||
		strokeCacheDesc->endCapStyle != context->endCapStyle ||
	#else
		strokeCacheDesc->capStyle != context->startCapStyle ||
	#endif
		strokeCacheDesc->joinStyle != context->joinStyle ||
		strokeCacheDesc->miterLimit != context->miterLimit ||
		strokeCacheDesc->lineWidth != context->strokeLineWidth ||
		strokeCacheDesc->dashPhase != context->dashPhase ||
		strokeCacheDesc->dashPatternHash != tmpHash) {

		strokeCacheDesc->dashPatternSize = (VGuint)context->patchedDashPattern.size;
	#if defined(VG_MZT_separable_cap_style)
		strokeCacheDesc->startCapStyle = context->startCapStyle;
		strokeCacheDesc->endCapStyle = context->endCapStyle;
	#else
		strokeCacheDesc->capStyle = context->startCapStyle;
	#endif
		strokeCacheDesc->joinStyle = context->joinStyle;
		strokeCacheDesc->miterLimit = context->miterLimit;
		strokeCacheDesc->lineWidth = context->strokeLineWidth;
		strokeCacheDesc->dashPhase = context->dashPhase;
		strokeCacheDesc->dashPatternHash = tmpHash;
		return AM_TRUE;
	}
	else
		return AM_FALSE;
}

/*!
	\brief Given a progressive index, it returns the corresponding dash value, its index inside the dash
	pattern array and a flag indicating if it's empty or not.
	\param empty output flag indicating if the requested dash value represents an empty dash trait.
	\param arrayIndex output index inside the dash pattern array.
	\param requestedIndex input progressive index.
	\param dashDesc input descriptor of the "normalized" dash pattern.
	\param context input context containing the dash pattern.
	\return the requested dash value.
	\pre requestedIndex >= 0.
*/
AMfloat amStrokeDashValueGet(AMbool *empty,
							 AMint32 *arrayIndex,
							 const AMint32 requestedIndex,
							 const AMDashDesc *dashDesc,
							 const AMContext *context) {

	AM_ASSERT(dashDesc);
	AM_ASSERT(context);
	AM_ASSERT(empty);
	AM_ASSERT(arrayIndex);
	AM_ASSERT(requestedIndex >= 0);

	if (requestedIndex == 0) {
		*arrayIndex = dashDesc->firstDashIndex;
		*empty = dashDesc->firstEmpty;
		return dashDesc->firstDashValue;
	}
	else {
		*arrayIndex = (AMint32)((dashDesc->firstDashIndex + requestedIndex) % context->patchedDashPattern.size);
		*empty = (*arrayIndex & 1) ? AM_TRUE : AM_FALSE;
		return context->patchedDashPattern.data[*arrayIndex];
	}
}

/*!
	\brief Normalize the dash pattern, given a phase.
	\param dashDesc output "normalized" dash pattern descriptor.
	\param requestedPhase input requested dash phase.
	\param context input context containing the dash pattern array.
	\pre patched dash pattern array must contain an even number of dash values.
*/
void amStrokeDashPatternNormalize(AMDashDesc *dashDesc,
								  const AMfloat requestedPhase,
								  const AMContext *context) {

	AMint32 i, j;
	AMfloat phase, patchedDashPhase;

	AM_ASSERT(context);
	AM_ASSERT(dashDesc);

	j = (AMint32)context->patchedDashPattern.size;
	// patched dash pattern must contain an even number of dash values
	AM_ASSERT((j & 1) == 0);

	if (j == 0) {
		dashDesc->firstDashIndex = 0;
		dashDesc->firstDashValue = 0.0f;
		dashDesc->firstEmpty = AM_TRUE;
		return;
	}

	// calculate requestedPhase modulo dashPatternSum
	if (requestedPhase > 0.0f)
		patchedDashPhase = requestedPhase - context->dashPatternSum * amFloorf(requestedPhase / context->dashPatternSum);
	else
		patchedDashPhase = context->dashPatternSum * amCeilf(-requestedPhase / context->dashPatternSum) + requestedPhase;

	phase = patchedDashPhase;
	i = 0;
	while (i < j) {
		phase -= context->patchedDashPattern.data[i];
		if (phase < 0.0f) {
			dashDesc->firstDashValue = -phase;
			dashDesc->firstDashIndex = i;
			dashDesc->firstEmpty = (i & 1) ? AM_TRUE : AM_FALSE;
			break;
		}
		else
		if (phase == 0.0f) {
			// if odd, we have eaten a whole empty dash value
			if (i & 1) {
				if (i == j - 1)
					i = -1;
				dashDesc->firstDashValue = context->patchedDashPattern.data[i + 1];
				dashDesc->firstDashIndex = i + 1;
			}
			else {
				// we have eaten a whole "not empty" dash value
				dashDesc->firstDashValue = 0.0f;
				dashDesc->firstDashIndex = i;
			}
			dashDesc->firstEmpty = AM_FALSE;
			break;
		}
		i++;
	}
}

/*!
	\brief Generate a circle arc given a center, a start-end points, angle to span and direction (cw/ccw).
	\param outVertices array that will contain the output points.
	\param center input circle arc center.
	\param start input circle arc start point.
	\param end input circle arc end point.
	\param spanAngle input angle to span, in radians.
	\param ccw AM_TRUE for counter-clockwise direction, AM_FALSE for clockwise direction.
	\param roundJoinAuxCoef coefficient used to calculate number of points to flatten the circle arc.
*/
void amStrokeCircleArcGenerate(AM_RAS_INPUT_VERTICES_ARRAY_TYPE *outVertices,
							   const AMVect2f *center,
							   const AMVect2f *start,
							   const AMVect2f *end,
							   const AMfloat spanAngle,
							   const AMbool ccw,
							   const AMfloat roundJoinAuxCoef) {

	AMint32 n;
	AMfloat nFloat;

	AM_ASSERT(outVertices);
	AM_ASSERT(center);
	AM_ASSERT(start);
	AM_ASSERT(end);

	nFloat = spanAngle * roundJoinAuxCoef;
	n = (AMint32)nFloat;

	if (n <= 1) {
		
		AMVect2f p;

		AM_VECT2_SET(&p, (start->x + end->x) * 0.5f, (start->y + end->y) * 0.5f)
		AM_STROKE_PUSH_POINT(*outVertices, p)
	}
	else
	if (n == 2) {
		AM_STROKE_PUSH_POINT(*outVertices, (*start))
		AM_STROKE_PUSH_POINT(*outVertices, (*end))
	}
	else {
		AMfloat deltaAngle, cosDelta, sinDelta;
		AMVect2f p, q, r;
		AMint32 i;

		// max number of points for circle arcs
		n = AM_MIN(n, AM_MAX_POINTS_TO_FLATTEN_DEGENERATIONS);

		// generate points
		deltaAngle = ccw ? (spanAngle / (n - 1)) : (-spanAngle / (n - 1));
		amSinCosf(&sinDelta, &cosDelta, deltaAngle);

		p.x = start->x - center->x;
		p.y = start->y - center->y;

		AM_STROKE_PUSH_POINT(*outVertices, (*start))

		for (i = 0; i < n - 2; ++i) {
			q.x = p.x * cosDelta - p.y * sinDelta;
			q.y = p.y * cosDelta + p.x * sinDelta;
			r.x = q.x + center->x;
			r.y = q.y + center->y;
			AM_STROKE_PUSH_POINT(*outVertices, r)
			p = q;
		}
		AM_STROKE_PUSH_POINT(*outVertices, (*end))
	}
}

/*!
	\brief Close a sub-polygon of the stroke.
	\param context context containing stroke sub-polygons.
*/
void amStrokePieceClose(AMContext *context) {

	AMint32 i;
	AMuint32 newCapacity, newSize;

	if (context->strokeAuxPts.size - context->strokeAuxPtsOldSize <= 2 &&
		context->strokeAuxPtsDx.size == 0) {
		context->strokeAuxPts.size = context->strokeAuxPtsOldSize;
		return;
	}

	if (context->strokeAuxPtsDx.size > 0) {
		// allocate memory to store the "sum" of dx array
		newSize = (AMuint32)context->strokeAuxPts.size;
		newCapacity = (AMuint32)(newSize + context->strokeAuxPtsDx.size);
		if (context->strokeAuxPts.capacity < newCapacity) {
			AM_DYNARRAY_CLEAR_RESERVE(context->strokeAuxPts, AM_RAS_INPUT_VERTEX_TYPE, newCapacity)
		}
		if (context->strokeAuxPts.error)
			return;
		// append dx array
		for (i = 0; i < (AMint32)context->strokeAuxPtsDx.size; ++i)
			context->strokeAuxPts.data[newSize + i] = context->strokeAuxPtsDx.data[context->strokeAuxPtsDx.size - i - 1];
		context->strokeAuxPts.size = newCapacity;
	}

	// push the new contour
	AM_DYNARRAY_PUSH_BACK(context->strokeAuxPtsPerContour, AMint32, (AMint32)(context->strokeAuxPts.size - context->strokeAuxPtsOldSize))
	// rewind temporary sx-dx arrays
	context->strokeAuxPtsDx.size = 0;
	context->strokeAuxPtsOldSize = (AMuint32)context->strokeAuxPts.size;
}

/*!
	\brief Generate points for a stroke cap, given an application point, a direction and the cap style.
	\param context context containing stroke parameters and temporary stroke sub-polygons.
	\param applicationPoint input location where the cap is applied.
	\param direction input normalized direction along which the cap is applied.
	\param capStyle input cap style.
	\param roundJoinAuxCoef coefficient used to calculate number of points to flatten circle arcs (round caps).
*/
void amStrokeCapGenerate(AMContext *context,
						 const AMVect2f *applicationPoint,
						 const AMVect2f *direction,
						 const VGCapStyle capStyle,
						 const AMfloat roundJoinAuxCoef) {

	AMVect2f normPerp, pSx, pDx;

	AM_ASSERT(applicationPoint);
	AM_ASSERT(direction);
	AM_ASSERT(context);

	// NB: direction MUST be normalized outside this function; 'normPerp' is the perpendicular direction
	// in CCW respect to 'direction'
	AM_VECT2_SET(&normPerp, -direction->y, direction->x)
	AM_VECT2_MADD(&pSx, applicationPoint, context->strokeLineThickness, &normPerp)
	AM_VECT2_MADD(&pDx, applicationPoint, -context->strokeLineThickness, &normPerp)

	if (context->lastJoinSeparated) {
		AM_STROKE_PUSH_POINT(context->strokeAuxPts, context->strokeRightPoint)
		AM_STROKE_PUSH_POINT(context->strokeAuxPts, context->strokeLeftPoint)
	}
	switch (capStyle) {
		case VG_CAP_BUTT:
			AM_STROKE_PUSH_POINT(context->strokeAuxPts, pSx)
			AM_STROKE_PUSH_POINT(context->strokeAuxPts, pDx)
			break;

		case VG_CAP_ROUND:
			amStrokeCircleArcGenerate(&context->strokeAuxPts, applicationPoint, &pSx, &pDx, AM_PI, AM_FALSE, roundJoinAuxCoef);
			break;
		
		case VG_CAP_SQUARE:
			AM_VECT2_SELF_MADD(&pSx, context->strokeLineThickness, direction)
			AM_VECT2_SELF_MADD(&pDx, context->strokeLineThickness, direction)
			AM_STROKE_PUSH_POINT(context->strokeAuxPts, pSx)
			AM_STROKE_PUSH_POINT(context->strokeAuxPts, pDx)
			break;

		default:
			AM_ASSERT(0 == 1);
			break;
	}
}

/*!
	\brief On smooth joins (emulated through a round join on the external side and an intersection point
	on the internal side), this function generates a join using two separated pieces; if needed, the previous
	contour piece will be closed.
	\param context context containing stroke parameters and temporary stroke sub-polygons.
	\param applicationPoint input location where the join is applied.
	\param inDirection incoming join normalized direction.
	\param outDirection outcoming join normalized direction.
	\param roundJoinAuxCoef coefficient used to calculate number of points to flatten circle arcs (round joins).
	\note this function is used for smooth interpolated joins only, not for real path joins.
*/
void amStrokeJoinSmoothTwoPieces(AMContext *context,
								 const AMVect2f *applicationPoint,
								 const AMVect2f *inDirection,
								 const AMVect2f *outDirection,
								 const AMfloat roundJoinAuxCoef) {

	AMVect2f sxIn, dxIn, sxOut, dxOut, perpendicularInDir, perpendicularOutDir;
	AM_RAS_INPUT_VERTEX_TYPE sxNext, dxNext, centerNext;
	AMfloat roundJoinAngle;
	AMint32 ccw = (AM_VECT2_CROSS(inDirection, outDirection) > 0.0f);

	if (amStrokeMemoryError(context))
		return;

	// NB: inDirection and outDirection MUST be normalized outside this function
	// perpendicular direction in CCW respect to 'inDirection'
	AM_VECT2_SET(&perpendicularInDir, -inDirection->y, inDirection->x)
	// perpendicular direction in CCW respect to 'outDirection'
	AM_VECT2_SET(&perpendicularOutDir, -outDirection->y, outDirection->x)

	AM_VECT2_MADD(&sxIn, applicationPoint, context->strokeLineThickness, &perpendicularInDir)
	AM_VECT2_MADD(&dxIn, applicationPoint, -context->strokeLineThickness, &perpendicularInDir)
	AM_VECT2_MADD(&sxOut, applicationPoint, context->strokeLineThickness, &perpendicularOutDir)
	AM_VECT2_MADD(&dxOut, applicationPoint, -context->strokeLineThickness, &perpendicularOutDir)
	roundJoinAngle = amAcosf(AM_VECT2_DOT(&perpendicularInDir, &perpendicularOutDir));

	if (context->strokeAuxPtsDx.size > 0) {
		AM_ASSERT(context->strokeAuxPts.size > 0);
		sxNext = context->strokeAuxPts.data[context->strokeAuxPts.size - 1];
		dxNext = context->strokeAuxPtsDx.data[context->strokeAuxPtsDx.size - 1];
		AM_VECT2_SET(&centerNext, (sxNext.x + dxNext.x) / 2, (sxNext.y + dxNext.y) / 2)
		amStrokePieceClose(context);
	}
	else {
	#if defined(AM_FIXED_POINT_PIPELINE)
		AM_VECT2_SET(&sxNext, amFloatToFixed1616(context->strokeLeftPoint.x), amFloatToFixed1616(context->strokeLeftPoint.y))
		AM_VECT2_SET(&dxNext, amFloatToFixed1616(context->strokeRightPoint.x), amFloatToFixed1616(context->strokeRightPoint.y))
		AM_VECT2_SET(&centerNext, amFloatToFixed1616(context->strokeMiddlePoint.x), amFloatToFixed1616(context->strokeMiddlePoint.y))
	#else
		sxNext = context->strokeLeftPoint;
		dxNext = context->strokeRightPoint;
		centerNext = context->strokeMiddlePoint;
	#endif
	}

	// previously contour has been closed, now we have to ensure stroke continuity
	if (context->strokeAuxPts.size == context->strokeAuxPtsOldSize) {
		AM_DYNARRAY_PUSH_BACK(context->strokeAuxPts, AM_RAS_INPUT_VERTEX_TYPE, dxNext)
		AM_DYNARRAY_PUSH_BACK(context->strokeAuxPts, AM_RAS_INPUT_VERTEX_TYPE, centerNext)
		AM_DYNARRAY_PUSH_BACK(context->strokeAuxPts, AM_RAS_INPUT_VERTEX_TYPE, sxNext)
	}

	// external smooth interpolated join piece
	AM_STROKE_PUSH_POINT(context->strokeAuxPts, sxIn)
	if (!ccw) {
		amStrokeCircleArcGenerate(&context->strokeAuxPts, applicationPoint, &sxIn, &sxOut, roundJoinAngle, AM_FALSE, roundJoinAuxCoef);
		AM_STROKE_PUSH_POINT(context->strokeAuxPts, sxOut)
		AM_STROKE_PUSH_POINT(context->strokeAuxPts, *applicationPoint)
	}
	else {
		AM_STROKE_PUSH_POINT(context->strokeAuxPts, *applicationPoint)
		AM_STROKE_PUSH_POINT(context->strokeAuxPts, dxOut)
		amStrokeCircleArcGenerate(&context->strokeAuxPts, applicationPoint, &dxOut, &dxIn, roundJoinAngle, AM_FALSE, roundJoinAuxCoef);
	}
	AM_STROKE_PUSH_POINT(context->strokeAuxPts, dxIn)
	amStrokePieceClose(context);

	// internal smooth interpolated join piece
	AM_STROKE_PUSH_POINT(context->strokeAuxPts, *applicationPoint)
	if (!ccw) {
		AM_STROKE_PUSH_POINT(context->strokeAuxPts, dxIn)
		amStrokeCircleArcGenerate(&context->strokeAuxPts, applicationPoint, &dxIn, &dxOut, roundJoinAngle, AM_FALSE, roundJoinAuxCoef);
		AM_STROKE_PUSH_POINT(context->strokeAuxPts, dxOut)
	}
	else {
		AM_STROKE_PUSH_POINT(context->strokeAuxPts, sxOut)
		amStrokeCircleArcGenerate(&context->strokeAuxPts, applicationPoint, &sxOut, &sxIn, roundJoinAngle, AM_FALSE, roundJoinAuxCoef);
		AM_STROKE_PUSH_POINT(context->strokeAuxPts, sxIn)
	}
	amStrokePieceClose(context);
	// keep track of useful points for the next iteration
	context->strokeLeftPoint = sxOut;
	context->strokeRightPoint = dxOut;
	context->strokeMiddlePoint = *applicationPoint;
	context->lastJoinSeparated = AM_TRUE;
}

/*!
	\brief Generate points for a stroke join.
	\param context context containing stroke parameters and temporary stroke sub-polygons.
	\param applicationPoint input location where the join is applied.
	\param inDirection incoming join normalized direction.
	\param outDirection outcoming join normalized direction.
	\param inLength length of the incoming join direction (non-dashed joins only).
	\param outLength length of the outcoming join direction (non-dashed joins only).
	\param dashedStroke AM_TRUE if the stroke is dashed, AM_FALSE if stroke is solid.
	\param roundJoinAuxCoef coefficient used to calculate number of points to flatten circle arcs (round joins).
	\param isRealJoin AM_TRUE if the join occurs between two path segments, AM_FALSE if the join occurs
	between two flatten segments inside the same path segment.
*/
void amStrokeJoinGenerate(AMContext *context,
						  const AMVect2f *applicationPoint,
						  const AMVect2f *inDirection,
						  const AMVect2f *outDirection,
						  const AMfloat inLength,
						  const AMfloat outLength,
						  const AMbool dashedStroke,
						  const AMfloat roundJoinAuxCoef,
						  const AMbool isRealJoin) {

	AMRay2f internalInRay, internalOutRay;
	AMfloat intParam[2], roundJoinAngle, screenStrokeWidth;
	AMbool intFound;
	AMint32 lastInternalIdx;
	AMuint32 intFlags;
	AMVect2f internalInDir, internalOutDir, perpendicularInDir, perpendicularOutDir;
	AMVect2f externalInPoint, externalOutPoint, internalIntersection, externalIntersection, tmp0, tmp1;
	AM_RAS_INPUT_VERTICES_ARRAY_TYPE *internalPts, *externalPts;
	AMint32 ccw = (AM_VECT2_CROSS(inDirection, outDirection) > 0.0f);
	VGJoinStyle joinStyle = isRealJoin ? context->joinStyle : VG_JOIN_ROUND;

	AM_VECT2_SET(&internalIntersection, 0.0f, 0.0f);
	
	// NB: inDirection and outDirection MUST be normalized outside this function
	// perpendicular direction in CCW respect to 'inDirection'
	AM_VECT2_SET(&perpendicularInDir, -inDirection->y, inDirection->x)
	// perpendicular direction in CCW respect to 'outDirection'
	AM_VECT2_SET(&perpendicularOutDir, -outDirection->y, outDirection->x)

	lastInternalIdx = -1;
	if (ccw) {
		AM_VECT2_MUL(&internalInDir, context->strokeLineThickness, &perpendicularInDir)
		AM_VECT2_MUL(&internalOutDir, context->strokeLineThickness, &perpendicularOutDir)
		internalPts = &context->strokeAuxPts;
		externalPts = &context->strokeAuxPtsDx;
		tmp0 = context->strokeLeftPoint;
		tmp1 = context->strokeRightPoint;

		if (context->strokeAuxPts.size > context->strokeAuxPtsOldSize)
			lastInternalIdx = (AMint32)context->strokeAuxPts.size - 1;
	}
	else {
		AM_VECT2_MUL(&internalInDir, -context->strokeLineThickness, &perpendicularInDir)
		AM_VECT2_MUL(&internalOutDir, -context->strokeLineThickness, &perpendicularOutDir)
		internalPts = &context->strokeAuxPtsDx;
		externalPts = &context->strokeAuxPts;
		tmp0 = context->strokeRightPoint;
		tmp1 = context->strokeLeftPoint;

		if (context->strokeAuxPtsDx.size > 0)
			lastInternalIdx = (AMint32)context->strokeAuxPtsDx.size - 1;
	}
	
	AM_VECT2_ADD(&internalInRay.origin, applicationPoint, &internalInDir)
	AM_VECT2_NEG(&internalInRay.direction, inDirection)
	AM_VECT2_ADD(&internalOutRay.origin, applicationPoint, &internalOutDir)
	internalOutRay.direction = *outDirection;
	AM_VECT2_REFLECT(&externalInPoint, &internalInRay.origin, applicationPoint)
	AM_VECT2_REFLECT(&externalOutPoint, &internalOutRay.origin, applicationPoint)

	AM_ASSERT(context->strokeLineWidth >= 0.0f);
	screenStrokeWidth = AM_MAX(context->pathUserToSurfaceScale[0], context->pathUserToSurfaceScale[1]) * context->strokeLineWidth;

#if defined RIM_VG_SRC
	intParam[0] = 0.0f;
	intParam[1] = 0.0f;
#endif
	intFound = dashedStroke ? AM_FALSE : amRayRayIntersect(intParam, &intFlags, &internalInRay, &internalOutRay);
	
	// internal join side
	if (intFound &&
		intParam[0] >= 0.0f && intParam[0] <= inLength &&
		intParam[1] >= 0.0f && intParam[1] <= outLength) {

		AMVect2f internalPrevious;
		AMint32 ccw2;

		AM_VECT2_MADD(&internalIntersection, &internalInRay.origin, intParam[0], &internalInRay.direction)

		if (screenStrokeWidth < AM_STROKE_WIDTH_NO_INTERSECTION_THRESHOLD) {
			AM_STROKE_PUSH_POINT(*internalPts, internalIntersection)
			goto externalJoin;
		}

		if (lastInternalIdx > 0) {

			AMVect2f dir;

			AM_VECT2_SUB(&dir, &internalIntersection, &internalPts->data[lastInternalIdx])
			// check if internal points are going in the same direction of the path
			if (AM_VECT2_DOT(outDirection, &dir) <= 0.0f)
				goto internalJoinSafe;
		}

		// check if internalOutRay.origin is covered by the previous stroke segment extrusion
		AM_VECT2_MADD(&internalPrevious, &internalOutRay.origin, inLength, inDirection)

		ccw2 = ((internalInRay.origin.x - applicationPoint->x) * (internalPrevious.y - applicationPoint->y) -
				(internalInRay.origin.y - applicationPoint->y) * (internalPrevious.x - applicationPoint->x)) > 0.0f;

		if (ccw2 != ccw) {
			AM_STROKE_PUSH_POINT(*internalPts, internalIntersection)
			goto externalJoin;
		}
		// in all other cases, do a safe internal join
	}
	else
		intFound = AM_FALSE;

internalJoinSafe:
	if (screenStrokeWidth >= AM_STROKE_WIDTH_NO_INTERSECTION_THRESHOLD) {
		// safe internal join
		if (!isRealJoin) {
			amStrokeJoinSmoothTwoPieces(context, applicationPoint, inDirection, outDirection, roundJoinAuxCoef);
			return;
		}
		AM_STROKE_PUSH_POINT(*internalPts, internalInRay.origin)
		AM_STROKE_PUSH_POINT(*internalPts, *applicationPoint)
		AM_STROKE_PUSH_POINT(*internalPts, internalOutRay.origin)
	}
	else {
		AM_STROKE_PUSH_POINT(*internalPts, internalInRay.origin)
		AM_STROKE_PUSH_POINT(*internalPts, internalOutRay.origin)
	}


externalJoin:
	// ensure stroke continuity
	if (context->lastJoinSeparated) {
		AM_STROKE_PUSH_POINT(*externalPts, tmp0)
		AM_STROKE_PUSH_POINT(*externalPts, tmp1)
	}

	// external join side
	switch (joinStyle) {
		case VG_JOIN_MITER:

			if (!intFound) {

				AMRay2f externalInRay, externalOutRay;

				externalInRay.origin = externalInPoint;
				externalInRay.direction = *inDirection;
				externalOutRay.origin = externalOutPoint;
				AM_VECT2_NEG(&externalOutRay.direction, outDirection)
				
				intFound = amRayRayIntersect(intParam, &intFlags, &externalInRay, &externalOutRay);
				if (intFound) {
					// calculate miter point as the intersection between two external rays
					AM_VECT2_MADD(&externalIntersection, &externalInRay.origin, intParam[0], &externalInRay.direction)
				}
			}
			else {
				// calculate miter point as the internal intersection point reflection around applicationPoint
				AM_VECT2_REFLECT(&externalIntersection, &internalIntersection, applicationPoint)
			}

			if (intFound &&
				AM_VECT2_SQR_DISTANCE(&externalIntersection, applicationPoint) <= context->miterMulThicknessSqr) {
				// if the external intersection was found and passed the miter limit test, push it
				AM_STROKE_PUSH_POINT(*externalPts, externalIntersection)
			}
			else {
				AM_STROKE_PUSH_POINT(*externalPts, externalInPoint)
				AM_STROKE_PUSH_POINT(*externalPts, externalOutPoint)
			}
			break;

		case VG_JOIN_ROUND:
			roundJoinAngle = amAcosf(AM_VECT2_DOT(&perpendicularInDir, &perpendicularOutDir));
			amStrokeCircleArcGenerate(externalPts, applicationPoint, &externalInPoint, &externalOutPoint, roundJoinAngle, ccw, roundJoinAuxCoef);
			break;

		case VG_JOIN_BEVEL:
			AM_STROKE_PUSH_POINT(*externalPts, externalInPoint)
			AM_STROKE_PUSH_POINT(*externalPts, externalOutPoint)
			break;

		default:
			AM_ASSERT(0 == 1);
			break;
	}

	context->lastJoinSeparated = AM_FALSE;
}

/*!
	\brief Generate solid (non-dashed) stroke polygons.
	\param context context containing stroke parameters and temporary stroke sub-polygons.
	\param ptsPerSegment updated pointer to the array containing number of points for each path segment.
	\param pointsItBegin pointer to the first flattening point of a sub-contour.
	\param pointsItEnd pointer to the last flattening point of a sub-contour.
	\param closed AM_TRUE if the sub-contour is closed, else AM_FALSE.
	\param roundJoinAuxCoef coefficient used to calculate number of points to flatten circle arcs (round joins/caps).
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amStrokeSolidGenerate(AMContext *context,
						   AMint32 **ptsPerSegment,
						   const AMVect2f *pointsItBegin,
						   const AMVect2f *pointsItEnd,
						   const AMbool closed,
						   const AMfloat roundJoinAuxCoef) {

	const AMVect2f *it0, *it1, *it2;
	AMVect2f lastPoint, normInDir, normOutDir;
	AM_RAS_INPUT_VERTEX_TYPE sxPoint, dxPoint;
	AMfloat inLength, outLength;
	AMint32 joinCounter;

	AM_ASSERT(context);
	AM_ASSERT(ptsPerSegment);
	AM_ASSERT(pointsItBegin);
	AM_ASSERT(pointsItEnd);

	AM_VECT2_SET(&sxPoint, 0, 0)
	AM_VECT2_SET(&dxPoint, 0, 0)
	
	it0 = pointsItBegin;
	it1 = pointsItBegin + 1;
	it2 = pointsItBegin + 2;
	lastPoint = *(pointsItEnd - 1);

	// rewind temporary sx-dx arrays
	context->strokeAuxPtsDx.size = 0;
	context->lastJoinSeparated = AM_FALSE;

	// extract the first "point per segment" value, as well the starting tangent
	joinCounter = *(*ptsPerSegment);
	(*ptsPerSegment)++;
	AM_ASSERT(joinCounter > 0);
	
	if (!closed) {
		// draw start cap
		amVect2fNormDirection(&normOutDir, it0, it1);
		amStrokeCapGenerate(context, it0, &normOutDir, context->startCapStyle, roundJoinAuxCoef);
		// check for memory errors
		if (amStrokeMemoryError(context)) {
			amStrokeMemoryErrorReset(context);
			return AM_FALSE;
		}
	}
	else {
		// draw join at the closing point
		inLength = amVect2fNormDirection(&normInDir, it0, &lastPoint);
		outLength = amVect2fNormDirection(&normOutDir, it1, it0);
		amStrokeJoinGenerate(context, it0, &normInDir, &normOutDir, inLength, outLength, AM_FALSE, roundJoinAuxCoef, AM_TRUE);
		// check for memory errors
		if (amStrokeMemoryError(context)) {
			amStrokeMemoryErrorReset(context);
			return AM_FALSE;
		}
		AM_ASSERT(context->strokeAuxPtsOldSize < context->strokeAuxPts.size);
		sxPoint = context->strokeAuxPts.data[context->strokeAuxPtsOldSize];
		AM_ASSERT(context->strokeAuxPtsDx.size > 0);
		dxPoint = context->strokeAuxPtsDx.data[0];
	}
	joinCounter--;

	while (it2 != pointsItEnd) {

		inLength = amVect2fNormDirection(&normInDir, it1, it0);
		outLength = amVect2fNormDirection(&normOutDir, it2, it1);
		if (joinCounter == 0) {
			amStrokeJoinGenerate(context, it1, &normInDir, &normOutDir, inLength, outLength, AM_FALSE, roundJoinAuxCoef, AM_TRUE);
			// new joinCounter
			joinCounter = *(*ptsPerSegment);
			(*ptsPerSegment)++;
			AM_ASSERT(joinCounter > 0);
		}
		else
			amStrokeJoinGenerate(context, it1, &normInDir, &normOutDir, inLength, outLength, AM_FALSE, roundJoinAuxCoef, AM_FALSE);

		// check for memory errors
		if (amStrokeMemoryError(context)) {
			amStrokeMemoryErrorReset(context);
			return AM_FALSE;
		}
		// next join
		joinCounter--;
		it0 = it1;
		it1 = it2;
		it2++;
	}

	if (!closed) {
		// draw end cap
		amVect2fNormDirection(&normInDir, it1, it0);
		amStrokeCapGenerate(context, it1, &normInDir, context->endCapStyle, roundJoinAuxCoef);
		// check for memory errors
		if (amStrokeMemoryError(context)) {
			amStrokeMemoryErrorReset(context);
			return AM_FALSE;
		}
	}
	else {
		// the join is real
		inLength = amVect2fNormDirection(&normInDir, it1, it0);
		outLength = amVect2fNormDirection(&normOutDir, pointsItBegin, it1);
		// line segment and join back to the start
		amStrokeJoinGenerate(context, it1, &normInDir, &normOutDir, inLength, outLength, AM_FALSE, roundJoinAuxCoef, AM_TRUE);
		// closing points
		AM_DYNARRAY_PUSH_BACK(context->strokeAuxPts, AM_RAS_INPUT_VERTEX_TYPE, sxPoint)
		AM_DYNARRAY_PUSH_BACK(context->strokeAuxPtsDx, AM_RAS_INPUT_VERTEX_TYPE, dxPoint)
		// check for memory errors
		if (amStrokeMemoryError(context)) {
			amStrokeMemoryErrorReset(context);
			return AM_FALSE;
		}
		// eat the join
		if (joinCounter == 0)
			(*ptsPerSegment)++;
	}
	amStrokePieceClose(context);
	// check for memory errors
	if (amStrokeMemoryError(context)) {
		amStrokeMemoryErrorReset(context);
		return AM_FALSE;
	}
	return AM_TRUE;
}

// given an index 'idx', it returns 2 "consecutive" indexes in the contour
void fixIndex(AMint32 *res0,
			  AMint32 *res1,
			  const AMint32 idx,
			  const AMint32 count,
			  const AMbool closed) {

	AM_ASSERT(idx >= 0 && idx <= count - 1);
	AM_ASSERT(count >= 2);

	if (idx == count - 1) {
		if (!closed) {
			*res0 = idx - 1;
			*res1 = idx;
		}
		else {
			*res0 = count - 1;
			*res1 = 0;
		}
	}
	else {
		*res0 = idx;
		*res1 = idx + 1;
	}

	AM_ASSERT(*res0 >= 0 && *res0 <= count - 1);
	AM_ASSERT(*res1 >= 0 && *res1 <= count - 1);
}

// given an index 'idx', it returns 3 "consecutive" indexes in the contour
void fixIndex2(AMint32 *res0,
			   AMint32 *res1,
			   AMint32 *res2,
			   const AMint32 idx,
			   const AMint32 count) {

	AM_ASSERT(idx >= 0 && idx <= count - 1);
	AM_ASSERT(count >= 2);

	if (idx == count - 1) {
		// for an open contour, we must not have a join on the last point (but only a cap)
		*res0 = count - 2;
		*res1 = count - 1;
		*res2 = 0;
	}
	else
	if (idx == 0) {
		// take in care of possible coincidence between first and last point
		*res0 = count - 1;
		*res1 = 0;
		*res2 = 1;
	}
	else {
		*res0 = idx - 1;
		*res1 = idx;
		*res2 = idx + 1;
	}

	AM_ASSERT(*res0 >= 0 && *res0 <= count - 1);
	AM_ASSERT(*res1 >= 0 && *res1 <= count - 1);
	AM_ASSERT(*res2 >= 0 && *res2 <= count - 1);
}

/*!
	\brief Given a dashing segment descriptor, it returns the descriptor for the next dash segment.
	\param nextSeg output descriptor for the next dash segment.
	\param length output length of the next dash segment.
	\param curSeg input descriptor for the current dash segment.
	\param baseArray input array of points (of the currently stroked sub-contour).
	\param count number of flattened points (of the currently stroked sub-contour).
	\param closed AM_TRUE if the sub-contour is closed, else AM_FALSE.
	\return AM_FALSE if all flattened points have been stroked, else AM_TRUE.
*/
AMbool amStrokeDashSegmentNext(AMDashSeg *nextSeg,
							   AMfloat *length,
							   const AMDashSeg *curSeg,
							   const AMVect2f *baseArray,
							   const AMint32 count,
							   const AMbool closed) {

	AM_ASSERT(curSeg->k0 >= 0 && curSeg->k0 <= count - 1);
	AM_ASSERT(curSeg->k1 >= 0 && curSeg->k1 <= count - 1);
	AM_ASSERT(count >= 2);

	if (curSeg->k1 == count - 1) {
		if (!closed)
			return AM_FALSE;
		nextSeg->k0 = curSeg->k1;
		nextSeg->k1 = 0;
	}
	else {
		// if the path is closed and we have already considered last segment, the path is finished
		if (closed && curSeg->k0 > curSeg->k1)
			return AM_FALSE;
		nextSeg->k0 = curSeg->k1;
		nextSeg->k1 = nextSeg->k0 + 1;
	}

	AM_ASSERT(nextSeg->k0 >= 0 && nextSeg->k0 <= count - 1);
	AM_ASSERT(nextSeg->k1 >= 0 && nextSeg->k1 <= count - 1);
	*length = amVect2fNormDirection(&nextSeg->normDir, &baseArray[nextSeg->k1], &baseArray[nextSeg->k0]);
	return AM_TRUE;
}

/*!
	\brief Generate a single dash piece.
	\param context context containing stroke parameters and temporary stroke sub-polygons.
	\param joinCounter updated counter for real joins.
	\param ptsPerSegment updated pointer to the array containing number of points for each path segment.
	\param ptsPerSegmentBase pointer to the first element of the original array containing number of points for each path segment.
	\param baseArray input array of points (of the currently stroked sub-contour).
	\param ptsCount number of flattened points (of the currently stroked sub-contour).
	\param p0 start point of the dash piece.
	\param i0 index of the flattening segment where p0 falls.
	\param p1 end point of the dash piece.
	\param i1 index of the flattening segment where p1 falls.
	\param closed AM_TRUE if the sub-contour is closed, else AM_FALSE.
	\param merging AM_TRUE if the dash piece wraps the sub-contour, else AM_FALSE (closed sub-contours only).
	\param roundJoinAuxCoef coefficient used to calculate number of points to flatten circle arcs (round joins/caps).
*/
void amStrokeSingleDashGenerate(AMContext *context,
								AMint32 *joinCounter,
								AMint32 **ptsPerSegment,
								AMint32 *ptsPerSegmentBase,
								const AMVect2f *baseArray,
								const AMlong ptsCount,
								const AMVect2f *p0,
								const AMint32 i0,
								const AMVect2f *p1,
								const AMint32 i1,
								const AMbool closed,
								const AMbool merging,
								const AMfloat roundJoinAuxCoef) {

	AMVect2f inDirection, outDirection;
	AMint32 k0, k1, k2, i, n;
	AMint32 *ptsPerSegmentBookmark = NULL;

	AM_ASSERT(context);

	// rewind temporary sx-dx arrays
	context->strokeAuxPtsDx.size = 0;

	if (i0 == i1)
		n = (merging) ? (AMint32)ptsCount : 0;
	else
		n = (i0 < i1) ? (i1 - i0) : (ptsCount - i0 + i1);

	// draw start cap
	context->lastJoinSeparated = AM_FALSE;
	fixIndex(&k0, &k1, i0, ptsCount, closed);
	amVect2fNormDirection(&outDirection, &baseArray[k0], &baseArray[k1]);
	amStrokeCapGenerate(context, p0, &outDirection, context->startCapStyle, roundJoinAuxCoef);

	i = i0;
	for (; n != 0; --n) {

		i++;
		// wrap around
		if (i == ptsCount)
			i = 0;

		fixIndex2(&k0, &k1, &k2, i, ptsCount);
		amVect2fNormDirection(&inDirection, &baseArray[k1], &baseArray[k0]);
		amVect2fNormDirection(&outDirection, &baseArray[k2], &baseArray[k1]);

		if (*joinCounter == 0) {
			amStrokeJoinGenerate(context, &baseArray[k1], &inDirection, &outDirection, 0.0f, 0.0f, AM_TRUE, roundJoinAuxCoef, AM_TRUE);

			if (i == 0) {
				AM_ASSERT(merging);
				ptsPerSegmentBookmark = *ptsPerSegment;
				*ptsPerSegment = ptsPerSegmentBase;
			}

			*joinCounter = *(*ptsPerSegment);
			(*ptsPerSegment)++;
			AM_ASSERT(*joinCounter > 0);
		}
		else
			amStrokeJoinGenerate(context, &baseArray[k1], &inDirection, &outDirection, 0.0f, 0.0f, AM_TRUE, roundJoinAuxCoef, AM_FALSE);

		(*joinCounter)--;
	}

	// draw end cap
	fixIndex(&k0, &k1, i1, ptsCount, closed);
	amVect2fNormDirection(&inDirection, &baseArray[k1], &baseArray[k0]);
	amStrokeCapGenerate(context, p1, &inDirection, context->endCapStyle, roundJoinAuxCoef);

	// return the new pointer to "points per segment" array
	if (ptsPerSegmentBookmark)
		*ptsPerSegment = ptsPerSegmentBookmark;

	amStrokePieceClose(context);
}

/*!
	\brief Update real join counter according to "eaten" joins by an empty dash piece.
	\param ptsPerSegment updated pointer to the array containing number of points for each path segment.	
	\param joinCounter updated counter for real joins.
	\param ptsCount number of flattened points (of the currently stroked sub-contour).
	\param i0 index of the flattening segment where the empty dash piece starts.
	\param i1 index of the flattening segment where the empty dash piece ends.
*/
void amStrokeJoinsEat(AMint32 **ptsPerSegment,
					  AMint32 *joinCounter,
					  const AMlong ptsCount,
					  const AMint32 i0,
					  const AMint32 i1) {

	AMint32 n = i1 - i0;

	AM_ASSERT(n >= 0);

	for (; n != 0; --n) {
		if (*joinCounter == 0) {
			// we have reached last segment, stop to read counters
			if (i1 == ptsCount /*&& n == 1*/) {
				AM_ASSERT(n == 1);
				continue;
			}
			else {
				*joinCounter = *(*ptsPerSegment);
				AM_ASSERT(*joinCounter > 0);
				(*ptsPerSegment)++;
			}
		}
		(*joinCounter)--;
	}
}

/*!
	\brief Generate dashed stroke polygons and returns the new phase.
	\param context context containing stroke parameters and temporary stroke sub-polygons.
	\param newDashPhase the new (output) phase for the next path sub-contour.
	\param ptsPerSegment updated pointer to the array containing number of points for each path segment.
	\param pointsItBegin pointer to the first flattening point of a sub-contour.
	\param pointsItEnd pointer to the last flattening point of a sub-contour.
	\param closed AM_TRUE if the sub-contour is closed, else AM_FALSE.
	\param roundJoinAuxCoef coefficient used to calculate number of points to flatten circle arcs (round joins/caps).
	\param dashDesc descriptor of the "normalized" (phase = 0) dash pattern.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amStrokeDashedGenerate(AMContext *context,
							  AMfloat *newDashPhase,
							   AMint32 **ptsPerSegment,
							   const AMVect2f *pointsItBegin,
							   const AMVect2f *pointsItEnd,
							   const AMbool closed,
							   const AMfloat roundJoinAuxCoef,
							   const AMDashDesc *dashDesc) {

	AMint32 requestedDashIdx, arrayDashIdx, mergingFinalIndex = -1, i;
	AMlong startIndex, ptsCount;
	AMbool empty, firstEmpty, mergingInfoSaved;
	AMVect2f p0, p1, startPoint, mergingFinalPoint;
	const AMVect2f *baseArray;
	AMDashSeg curSeg, nextSeg;
	AMfloat residual, dashPatVal = 0.0f;
	AMbool dashToExtract;
	AMint32 joinCounter;
	AMint32 *segmentsDescBase = *ptsPerSegment;

	#define SAVE_MERGING_INFO(_p, _idx) \
		mergingFinalPoint = _p; \
		mergingFinalIndex = _idx; \
		mergingInfoSaved = AM_TRUE;

	#define GEN_DASH(_startPoint, _startIndex, _finalPoint, _finalIndex, _merging) \
	{ \
		amStrokeSingleDashGenerate(context, &joinCounter, ptsPerSegment, segmentsDescBase, baseArray, ptsCount, _startPoint, _startIndex, _finalPoint, _finalIndex, closed, _merging, roundJoinAuxCoef); \
		if (amStrokeMemoryError(context)) { \
			amStrokeMemoryErrorReset(context); \
			return AM_FALSE; \
		} \
	}
	
	// contour must be made of at least 2 points
	ptsCount = (AMlong)(pointsItEnd - pointsItBegin);
	AM_ASSERT(ptsCount > 1);

	requestedDashIdx = 0;
	baseArray = pointsItBegin;
	curSeg.k0 = 0;
	curSeg.k1 = 1;
#if defined RIM_VG_SRC
    curSeg.normDir.x = 0.0f;
    curSeg.normDir.y = 0.0f;
#endif
	residual = amVect2fNormDirection(&curSeg.normDir, baseArray + 1, baseArray);
	startIndex = 0;
	dashToExtract = AM_TRUE;
	p0 = startPoint = *baseArray;
	// set flags to manage possible merging
	mergingInfoSaved = AM_FALSE;
	firstEmpty = dashDesc->firstEmpty;
	// extract the first "point per segment" value
	joinCounter = *(*ptsPerSegment);
	(*ptsPerSegment)++;
	AM_ASSERT(joinCounter > 0);
	joinCounter--;

	while (1) {

		// eat a dash entry
		if (dashToExtract) {
			dashPatVal = amStrokeDashValueGet(&empty, &arrayDashIdx, requestedDashIdx, dashDesc, context);
			// next dash pattern entry
			requestedDashIdx++;
		}
		residual -= dashPatVal;
		// merging condition:
		// - path must be closed
		// - first dash is not empty
		// - last dash is not empty

		if (residual == 0.0f) {

			AMbool haveNextSegment = amStrokeDashSegmentNext(&nextSeg, &residual, &curSeg, baseArray, ptsCount, closed);

			if (haveNextSegment) {
				AM_VECT2_SET(&p1, p0.x + dashPatVal * curSeg.normDir.x, p0.y + dashPatVal * curSeg.normDir.y)
				if (!empty) {
					// save merging information if needed
					if (closed && !firstEmpty && !mergingInfoSaved) {
						SAVE_MERGING_INFO(p1, curSeg.k0)
						amStrokeJoinsEat(ptsPerSegment, &joinCounter, ptsCount, 0, curSeg.k0);
					}
					else 
						GEN_DASH(&startPoint, startIndex, &p1, curSeg.k0, AM_FALSE)
				}
				else {
					if (!closed || firstEmpty || mergingInfoSaved)
						amStrokeJoinsEat(ptsPerSegment, &joinCounter, ptsCount, startIndex, curSeg.k0);
				}

				// now we have to update the current dash segment
				startPoint = p0 = baseArray[nextSeg.k0];
				startIndex = curSeg.k0;
				curSeg = nextSeg;
				dashToExtract = AM_TRUE;
			}
			else {
				if (!closed)
					break;
				else {
					if (!firstEmpty) {
						// N.B.: REFERENCE MISMATCH
						// If residual is 0 but the next dash value is 0, reference implementation does the merging.
						// Eg: MOVE_TO(50, 20), LINE_TO(150, 20), LINE_TO(150, 120), LINE_TO(50, 120), CLOSE_PATH()
						// dashPattern = 0.0f, 400.0f, 0.0f, 10.0f, phase = 0
						//
						// If residual is 0 but the next dash value is not 0, reference implementation does the
						// simple dash.
						// dashPattern = 0.0f, 400.0f, 5.0f, 10.0f, phase = 0

						// manage possible merging
						if (!empty) {
							AM_ASSERT(mergingInfoSaved);
							GEN_DASH(&startPoint, startIndex, &mergingFinalPoint, mergingFinalIndex, AM_TRUE)
						}
						else {
							AMint32 *curPtsPerSegment;

							amStrokeJoinsEat(ptsPerSegment, &joinCounter, ptsCount, startIndex, curSeg.k0);

							// keep track of the current pointer
							curPtsPerSegment = *ptsPerSegment;
							// start looking at the first "points per segment" entry
							*ptsPerSegment = segmentsDescBase;
							joinCounter = *(*ptsPerSegment);
							(*ptsPerSegment)++;
							AM_ASSERT(joinCounter > 0);
							joinCounter--;
							// draw the first dash
							GEN_DASH(pointsItBegin, 0, &mergingFinalPoint, mergingFinalIndex, AM_FALSE)
							// restore current pointer
							*ptsPerSegment = curPtsPerSegment;
						}
					}
					else {
						if (!empty)
							GEN_DASH(&startPoint, startIndex, &baseArray[curSeg.k1], curSeg.k0, AM_FALSE)
					}
					break;
				}
			}
		}
		else
		if (residual > 0.0f) {
			// find p1
			AM_VECT2_SET(&p1, p0.x + dashPatVal * curSeg.normDir.x, p0.y + dashPatVal * curSeg.normDir.y)
			if (!empty) {
				// save merging information if needed
				if (closed && !firstEmpty && !mergingInfoSaved) {
					SAVE_MERGING_INFO(p1, curSeg.k0)
					amStrokeJoinsEat(ptsPerSegment, &joinCounter, ptsCount, 0, curSeg.k0);
				}
				else
					GEN_DASH(&startPoint, startIndex, &p1, curSeg.k0, AM_FALSE)
			}
			else {
				if (!closed || firstEmpty || mergingInfoSaved)
					amStrokeJoinsEat(ptsPerSegment, &joinCounter, ptsCount, startIndex, curSeg.k0);
			}

			startPoint = p0 = p1;
			startIndex = curSeg.k0;
			dashToExtract = AM_TRUE;
		}
		else {
			// residual < 0
			dashPatVal = -residual;
			dashToExtract = AM_FALSE;

			// now we have to extract a new segment
			if (amStrokeDashSegmentNext(&nextSeg, &residual, &curSeg, baseArray, ptsCount, closed)) {
				p0 = baseArray[nextSeg.k0];
				curSeg = nextSeg;
			}
			else {
				if (!closed) {
					if (!empty)
						// draw last dash
						GEN_DASH(&startPoint, startIndex, &baseArray[curSeg.k1], curSeg.k0, AM_FALSE)
					else
						// last dash was empty, so we have to eat corresponding joins
						amStrokeJoinsEat(ptsPerSegment, &joinCounter, ptsCount, startIndex, curSeg.k0);
				}
				else {
					if (!firstEmpty) {
						// manage possible merging: take in care that is merging information have not been saved yet,
						// it means that the current dash value eats the whole path, so use drawSolidStroke
						if (!mergingInfoSaved) {
							*ptsPerSegment = segmentsDescBase;
							if (!amStrokeSolidGenerate(context, ptsPerSegment, pointsItBegin, pointsItEnd, closed, roundJoinAuxCoef))
								return AM_FALSE;
						}
						else {
							// manage possible merging
							if (!empty)
								GEN_DASH(&startPoint, startIndex, &mergingFinalPoint, mergingFinalIndex, AM_TRUE)
							else {
								AMint32 *curPtsPerSegment;

								amStrokeJoinsEat(ptsPerSegment, &joinCounter, ptsCount, startIndex, curSeg.k0);

								// keep track of the current pointer
								curPtsPerSegment = *ptsPerSegment;
								// start looking at the first "points per segment" entry
								*ptsPerSegment = segmentsDescBase;
								joinCounter = *(*ptsPerSegment);
								(*ptsPerSegment)++;
								AM_ASSERT(joinCounter > 0);
								joinCounter--;
								// draw first dash
								GEN_DASH(pointsItBegin, 0, &mergingFinalPoint, mergingFinalIndex, AM_FALSE)
								// restore current pointer
								*ptsPerSegment = curPtsPerSegment;
							}
						}
					}
					else {
						if (!empty)
							// draw last dash
							GEN_DASH(&startPoint, startIndex, &baseArray[curSeg.k1], curSeg.k0, AM_FALSE)
						else
							// eat joins
							amStrokeJoinsEat(ptsPerSegment, &joinCounter, ptsCount, startIndex, curSeg.k0);
					}
				}
				break;
			}
		}
	}

	// calculate the new phase for the next sub-contour
	if (context->dashPhaseReset == VG_TRUE)
		*newDashPhase = context->dashPhase;
	else {
		*newDashPhase = residual;
		for (i = 0; i <= arrayDashIdx; ++i)
			*newDashPhase += context->patchedDashPattern.data[i];
	}

	return AM_TRUE;

	#undef SAVE_MERGING_INFO
	#undef GEN_DASH
}

/*!
	\brief Generate polygons that realize the stroke.
	\param _context pointer to a AMContext structure, containing temporary (output) stroke structures.
	\param _slot pointer to a AMPathCacheSlot structure, specifying the cache slot that contains flattening points.
	\param roundJoinAuxCoef coefficient used to calculate number of points in round joins/caps.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
*/
AMbool amStrokeGenerate(void *_context,
						const void *_slot,
						const AMfloat roundJoinAuxCoef) {

	AMContext *context = (AMContext *)_context;
	const AMPathCacheSlot *slot = (const AMPathCacheSlot *)_slot;
	AMuint32 i, j;
	AMVect2f *itPts0, *itPts1;
	AMfloat dashPhase;
	AMDashDesc dashDesc;
	AMint32 *segmentsDesc;

	AM_ASSERT(slot);
	AM_ASSERT(context);

	j = (AMuint32)slot->ptsPerContour.size;
	// simulate a rewind operation
	context->strokeAuxPts.size = 0;
	context->strokeAuxPtsPerContour.size = 0;
	context->strokeAuxPtsOldSize = 0;

	if (j > 0) {
	#if defined(VG_MZT_statistics)
		AMuint32 startMS = amTimeGet();
		AMuint32 endMS;
	#endif

		itPts0 = itPts1 = slot->flattenPts.data;
		segmentsDesc = slot->ptsCountPerSegment.data;

		// solid stroke
		if (context->patchedDashPattern.size == 0) {
			for (i = 0; i < j; ++i) {
				itPts1 += slot->ptsPerContour.data[i];
				if (!amStrokeSolidGenerate(context, &segmentsDesc, itPts0, itPts1, slot->subPathsClosed.data[i], roundJoinAuxCoef))
					return AM_FALSE;
				// next sub-contour
				itPts0 = itPts1;
			}
		}
		else
		// dash stroke
		if (context->dashPatternSum > AM_EPSILON_FLOAT) {
			dashPhase = context->dashPhase;
			for (i = 0; i < j; ++i) {
				amStrokeDashPatternNormalize(&dashDesc, dashPhase, context);
				itPts1 += slot->ptsPerContour.data[i];
				if (!amStrokeDashedGenerate(context, &dashPhase, &segmentsDesc, itPts0, itPts1, slot->subPathsClosed.data[i], roundJoinAuxCoef, &dashDesc))
					return AM_FALSE;
				// next sub-contour
				itPts0 = itPts1;
			}
		}

	#if defined(VG_MZT_statistics)
		endMS = amTimeGet();
		context->statisticsInfo.strokerPointsCount += (AMuint32)context->strokeAuxPts.size;
		context->statisticsInfo.strokerTimeMS += (endMS - startMS);
	#endif
	}
	return AM_TRUE;
}

#undef AM_STROKE_PUSH_POINT

#if defined (RIM_VG_SRC)
#pragma pop
#endif

