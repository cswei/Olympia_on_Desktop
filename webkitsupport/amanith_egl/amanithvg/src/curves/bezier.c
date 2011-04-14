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
	\file bezier.c
	\brief Quadratic and cubic Bezier curves, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/
#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif


#include "bezier.h"
#include "integration.h"

/*!
	\brief Quadratic Bezier constructor given 3 control points. It calculates first derivative coefficients too.
	\param dst output Bezier.
	\param p0 first control point.
	\param p1 second control point.
	\param p2 third control point.
*/
void amBezier2fSet(AMBezier2f *dst,
				   const AMVect2f *p0,
				   const AMVect2f *p1,
				   const AMVect2f *p2) {

	dst->p0 = *p0;
	dst->p1 = *p1;
	dst->p2 = *p2;

	AM_VECT2_MUL(&dst->d0, -2, p1)
	AM_VECT2_SELF_ADD(&dst->d0, p0)
	AM_VECT2_SELF_ADD(&dst->d0, p2)
	AM_VECT2_SELF_MUL(&dst->d0, 2)
	AM_VECT2_SUB(&dst->d1, p1, p0)
	AM_VECT2_SELF_MUL(&dst->d1, 2)
}

/*!
	\brief Quadratic Bezier evaluation. It evaluates a quadratic Bezier position given the local parameter.
	\param dst output coordinates.
	\param curve input Bezier curve.
	\param u local parameter.
	\pre 0.0f <= u <= 1.0f
*/
void amBezier2fEval(AMVect2f *dst,
					const AMBezier2f *curve,
					const AMfloat u) {

	AMfloat uSqr = u * u;
	AMfloat u1 = 1.0f - u;
	AMfloat u1Sqr = u1 * u1;
	AMfloat u2 = 2.0f * u * u1;

	AM_ASSERT(dst);
	AM_ASSERT(curve);
	AM_ASSERT(u >= 0.0f && u <= 1.0f);

	dst->x = u1Sqr * curve->p0.x + u2 * curve->p1.x + uSqr * curve->p2.x;
	dst->y = u1Sqr * curve->p0.y + u2 * curve->p1.y + uSqr * curve->p2.y;
}

/*!
	\brief Quadratic Bezier tangent evaluation. It evaluates a quadratic Bezier tangent vector given the local parameter.
	\param dst output tangent vector.
	\param curve input Bezier curve.
	\param u local parameter.
	\pre 0.0f <= u <= 1.0f
*/
void amBezier2fTan(AMVect2f *dst,
				   const AMBezier2f *curve,
				   const AMfloat u) {

	AM_ASSERT(dst);
	AM_ASSERT(curve);
	AM_ASSERT(u >= 0.0f && u <= 1.0f);

	AM_VECT2_MUL(dst, u, &curve->d0)
	AM_VECT2_SELF_ADD(dst, &curve->d1)
}

/*!
	\brief Quadratic Bezier speed evaluation callback. It evaluates the length of the quadratic Bezier tangent vector, given the local parameter.
	\param u local parameter.
	\param userData input Bezier curve, specified as a void pointer.
	\return the length of the quadratic Bezier tangent vector.
	\pre 0.0f <= u <= 1.0f
	\note curve is specified as a void pointer, in order to have a more flexible way to use it as a callback inside
	the Romberg integrator function.
*/
AMfloat amBezier2fSpeedCallback(const AMfloat u,
								void *userData) {

	const AMBezier2f *curve = (const AMBezier2f *)userData;
	AMVect2f tangent;

	amBezier2fTan(&tangent, curve, u);
	return amSqrtf(AM_VECT2_SQR_LENGTH(&tangent));
}

/*!
	\brief Quadratic Bezier length evaluation. It evaluates a quadratic Bezier length given a local parameter range.
	\param curve input Bezier curve.
	\param u0 min local parameter.
	\param u1 max local parameter.
	\return the length of the Bezier trait.
	\pre 0.0f <= u0 <= u1 <= 1.0f
*/
AMfloat amBezier2fLen(const AMBezier2f *curve,
					  const AMfloat u0,
					  const AMfloat u1) {

	AMfloat res;

	AM_ASSERT(curve);
	AM_ASSERT(u0 >= 0.0f && u0 <= 1.0f);
	AM_ASSERT(u1 >= 0.0f && u1 <= 1.0f);
	AM_ASSERT(u0 <= u1);

	amRombergf(&res, u0, u1, amBezier2fSpeedCallback, (void *)curve, AM_EPSILON_FLOAT);
	return res;
}

/*!
	\brief Given len = amBezier2fLen(curve, 0, u), this function calculates u.
	This function uses the Newton method to find the numerical root solution. This method has a quadratic convergence.
	\param result the output local parameter.
	\param curve input Bezier curve.
	\param len the curve length between 0 and the calculated local parameter.
	\return AM_TRUE if u was found in less than MAX_ITERATIONS steps (15), else AM_FALSE.
*/
AMbool amBezier2fParam(AMfloat *result,
					   const AMBezier2f *curve,
					   const AMfloat len) {

	#define MAX_ITERATIONS 15
	AMfloat totalLen, localLen, pivot, error, precision, l;
	AMVect2f tangent;
	AMuint32 i;

	AM_ASSERT(curve);
	AM_ASSERT(result);

	precision = 2.0f * AM_EPSILON_FLOAT;

	// check for out of range parameter
	if (len <= 0.0f) {
		*result = 0.0f;
		return AM_TRUE;
	}
	totalLen = amBezier2fLen(curve, 0.0f, 1.0f);
	if (len >= totalLen) {
		*result = 1.0f;
		return AM_TRUE;
	}

	// Newton's method for root searching; here we have to find root of function f = Length(u) - CurvePos
	// u is the value we are searching for
	localLen = len / totalLen;
	// here's a good starting point
	pivot = localLen;
	for (i = 0; i < MAX_ITERATIONS; ++i) {
		error = amBezier2fLen(curve, 0.0f, pivot) - len;
		// test relative error
		if (amAbsf(error / totalLen) <= precision) {
			*result = pivot;
			return AM_TRUE;
		}
		amBezier2fTan(&tangent, curve, pivot);
		l = AM_VECT2_SQR_LENGTH(&tangent);
		AM_ASSERT(l > AM_EPSILON_FLOAT);
		pivot -= error / amSqrtf(l);
		if (pivot < 0.0f)
			pivot = 0.001f;
		else
		if (pivot > 1.0f)
			pivot = 0.999f;
	}
	*result = pivot;
	return AM_FALSE;
	#undef MAX_ITERATIONS
}

/*!
	\brief Given a monotone quadratic Bezier curve, this function flattens it.
	\param points the output points array.
	\param curve input Bezier curve.
	\param params flattening thresholds.
	\param includeLastPoint if AM_TRUE the output array will contain the last control point too.
*/
void amBezier2fFlattenParabolic(AMVect2fDynArray *points,
								const AMBezier2f *curve,
								const AMFlattenParams *params,
								const AMbool includeLastPoint) {

	AMBezier2f bez;
	AMfloat dx, dy, d, t, t1, y2, vx, vy, w;

	AM_ASSERT(curve);
	AM_ASSERT(points);

	#define POINT_LERP(dest, pt0, w0, pt1, w1) \
		dest.x = pt0.x * w0 + pt1.x * w1; \
		dest.y = pt0.y * w0 + pt1.y * w1;

	bez.p0 = curve->p0;
	bez.p1 = curve->p1;
	bez.p2 = curve->p2;

	AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, curve->p0)
	w = params->two_sqrt_flatness;

	while (1) {
		
		dx = bez.p1.x - bez.p0.x;
		dy = bez.p1.y - bez.p0.y;			
		d = amSqrtf(dx * dx + dy * dy);
		if (d <= AM_EPSILON_FLOAT)
			break;

		vx = bez.p2.x - bez.p0.x;
		vy = bez.p2.y - bez.p0.y;
		y2 = (vx * (-dy) + vy * dx) / d;
		y2 = amAbsf(y2);
		if (y2 <= AM_EPSILON_FLOAT)
			break;
		t = w / amSqrtf(y2);
		if (t + AM_EPSILON_FLOAT >= 1.0f)
			break;

		// right cut
		t1 = 1.0f - t;
		POINT_LERP(bez.p0, bez.p0, t1, bez.p1, t)
		POINT_LERP(bez.p1, bez.p1, t1, bez.p2, t)
		POINT_LERP(bez.p0, bez.p0, t1, bez.p1, t)

		AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, bez.p0)
	}
	if (includeLastPoint) {
		AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, curve->p2)
	}

	#undef POINT_LERP
}

/*!
	\brief Given a quadratic Bezier curve, this function flattens it.
	\param points the output points array.
	\param curve input Bezier curve.
	\param params flattening thresholds.
	\param includeLastPoint if AM_TRUE the output array will contain the last control point too.
	\note the output points array will always include points of maximum.
*/
void amBezier2fFlatten(AMVect2fDynArray *points,
					   const AMBezier2f *curve,
					   const AMFlattenParams *params,
					   const AMbool includeLastPoint) {

	AMfloat x1, x2, y2, vx, vy, x, y, d, l, boxW, boxH, dmax;
	AMVect2f p01, p02;
	AMAABox2f tmpBox;

#define POINT_LERP(dest, pt0, w0, pt1, w1) \
	dest.x = pt0.x * w0 + pt1.x * w1; \
	dest.y = pt0.y * w0 + pt1.y * w1;

#define MAX_ABS_DIST(_dst, _p) \
	x = amAbsf((_p).x); \
	y = amAbsf((_p).y); \
	if (x >= y ) \
		_dst = x; \
	else \
		_dst = y;

	AM_AABOX2_SET(&tmpBox, &curve->p0, &curve->p1)
	AM_AABOX2_EXTEND_TO_INCLUDE(&tmpBox, &curve->p2)
	boxW = AM_AABOX2_WIDTH(&tmpBox);
	boxH = AM_AABOX2_HEIGHT(&tmpBox);
	dmax = AM_MAX(boxW, boxH);
	// p0, p1, p2 coincident
	if (dmax <= AM_EPSILON_FLOAT) {
		if (includeLastPoint) {
			AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, curve->p0)
		}
		return;
	}
	// take care of degenerated curves
	if (dmax < AM_BEZIER_DEGENERATION_THRESHOLD) {

		AMint32 n = AM_MIN(params->degenerative_curve_segments, AM_MAX_POINTS_TO_FLATTEN_DEGENERATIONS);
		AMfloat t = 0.0f;

		d = 1.0f / n;
		for (; n!= 0; --n) {
			amBezier2fEval(&p01, curve, t);
			AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, p01)
			t += d;
		}

		if (includeLastPoint) {
			AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, curve->p2)
		}
		return;
	}

	AM_VECT2_SUB(&p01, &curve->p1, &curve->p0)
	AM_VECT2_SUB(&p02, &curve->p2, &curve->p0)
	p01.x /= dmax;
	p01.y /= dmax;
	p02.x /= dmax;
	p02.y /= dmax;
	MAX_ABS_DIST(d, p01)
	// p0, p1 coincident
	if (d <= AM_EPSILON_FLOAT) {

		MAX_ABS_DIST(d, p02)
		// p0, p1, p2 coincident
		if (d <= AM_EPSILON_FLOAT) {
			if (includeLastPoint) {
				AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, curve->p0)
			}
			return;
		}
		else {
			// the curve is a line from p0 to p2
			AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, curve->p0)
			if (includeLastPoint) {
				AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, curve->p2)
			}
			return;
		}
	}

	l = amSqrtf(p01.x * p01.x + p01.y * p01.y);
	AM_ASSERT(l > 0.0f);
	vx = p01.x / l;
	vy = p01.y / l;
	x1 = p01.x * vx + p01.y * vy;
	x2 = p02.x * vx + p02.y * vy;

	if (x2 < x1) {

		AMBezier2f leftBez, rightBez;
		AMfloat t, t1;

		y2 = dmax * amAbsf(-p02.x * vy + p02.y * vx);
		if (params->sixtyfour_flatness > y2) {

			t = -x1 / (x2 - 2.0f * x1);
			t1 = 1.0f - t;

			rightBez.p0 = curve->p0;
			rightBez.p1 = curve->p1;
			rightBez.p2 = curve->p2;

			// cut right
			POINT_LERP(rightBez.p0, rightBez.p0, t1, rightBez.p1, t)
			POINT_LERP(rightBez.p1, rightBez.p1, t1, rightBez.p2, t)
			POINT_LERP(rightBez.p0, rightBez.p0, t1, rightBez.p1, t)
			// cut left
			leftBez.p0 = curve->p0;
			POINT_LERP(leftBez.p1, curve->p0, t1, curve->p1, t)
			leftBez.p2 = rightBez.p0;

			amBezier2fFlattenParabolic(points, &leftBez, params, AM_FALSE);
			amBezier2fFlattenParabolic(points, &rightBez, params, includeLastPoint);
		}
		else
			amBezier2fFlattenParabolic(points, curve, params, includeLastPoint);
	}
	else
		amBezier2fFlattenParabolic(points, curve, params, includeLastPoint);

	#undef POINT_LERP
	#undef MAX_ABS_DIST
}

/*!
	\brief Cubic Bezier constructor given 4 control points. It calculates first derivative coefficients too.
	\param dst output Bezier.
	\param p0 first control point.
	\param p1 second control point.
	\param p2 third control point.
	\param p3 fourth control point.
*/
void amBezier3fSet(AMBezier3f *dst,
				   const AMVect2f *p0,
				   const AMVect2f *p1,
				   const AMVect2f *p2,
				   const AMVect2f *p3) {

	dst->p0 = *p0;
	dst->p1 = *p1;
	dst->p2 = *p2;
	dst->p3 = *p3;

	AM_VECT2_SUB(&dst->d0, p1, p2)
	AM_VECT2_SELF_MUL(&dst->d0, 3)
	AM_VECT2_SELF_ADD(&dst->d0, p0)
	AM_VECT2_SELF_ADD(&dst->d0, p3)
	AM_VECT2_SELF_MUL(&dst->d0, 3.0f)

	AM_VECT2_MUL(&dst->d1, -2.0f, p1)
	AM_VECT2_SELF_ADD(&dst->d1, p2)
	AM_VECT2_SELF_MUL(&dst->d1, 6.0f)

	AM_VECT2_SUB(&dst->d2, p1, p0)
	AM_VECT2_SELF_MUL(&dst->d2, 3.0f)
}

/*!
	\brief Cubic Bezier evaluation. It evaluates a cubic Bezier position given the local parameter.
	\param dst output coordinates.
	\param curve input Bezier curve.
	\param u local parameter.
	\pre 0.0f <= u <= 1.0f
*/
void amBezier3fEval(AMVect2f *dst,
					const AMBezier3f *curve,
					const AMfloat u) {

	AMfloat uSqr = u * u;
	AMfloat uCub = uSqr * u;
	AMfloat u1 = 1.0f - u;
	AMfloat u1Sqr = u1 * u1;
	AMfloat u1Cub = u1Sqr * u1;
	AMfloat k1 = 3.0f * u * u1Sqr;
	AMfloat k2 = 3.0f * uSqr * u1;

	AM_ASSERT(dst);
	AM_ASSERT(curve);
	AM_ASSERT(u >= 0.0f && u <= 1.0f);
	
	dst->x = u1Cub * curve->p0.x + k1 * curve->p1.x + k2 * curve->p2.x + uCub * curve->p3.x;
	dst->y = u1Cub * curve->p0.y + k1 * curve->p1.y + k2 * curve->p2.y + uCub * curve->p3.y;
}

/*!
	\brief Cubic Bezier tangent evaluation. It evaluates a cubic Bezier tangent vector given the local parameter.
	\param dst output tangent vector.
	\param curve input Bezier curve.
	\param u local parameter.
	\pre 0.0f <= u <= 1.0f
*/
void amBezier3fTan(AMVect2f *dst,
				   const AMBezier3f *curve,
				   const AMfloat u) {

	AMfloat u2 = u * u;

	AM_ASSERT(dst);
	AM_ASSERT(curve);
	AM_ASSERT(u >= 0.0f && u <= 1.0f);

	dst->x = u2 * curve->d0.x + u * curve->d1.x + curve->d2.x;
	dst->y = u2 * curve->d0.y + u * curve->d1.y + curve->d2.y;
}

/*!
	\brief Cubic Bezier speed evaluation callback. It evaluates the length of the cubic Bezier tangent vector, given the local parameter.
	\param u local parameter.
	\param userData input Bezier curve, specified as a void pointer.
	\return the length of the quadratic Bezier tangent vector.
	\pre 0.0f <= u <= 1.0f
	\note curve is specified as a void pointer, in order to have a more flexible way to use it as a callback inside
	the Romberg integrator function.
*/
AMfloat amBezier3fSpeedCallback(const AMfloat u,
								void *userData) {

	const AMBezier3f *curve = (const AMBezier3f *)userData;
	AMVect2f tangent;

	amBezier3fTan(&tangent, curve, u);
	return amSqrtf(AM_VECT2_SQR_LENGTH(&tangent));
}

/*!
	\brief Cubic Bezier length evaluation. It evaluates a cubic Bezier length given a local parameter range.
	\param curve input Bezier curve.
	\param u0 min local parameter.
	\param u1 max local parameter.
	\return the length of the Bezier trait.
	\pre 0.0f <= u0 <= u1 <= 1.0f
*/
AMfloat amBezier3fLen(const AMBezier3f *curve,
					  const AMfloat u0,
					  const AMfloat u1) {

	AMfloat res;

	AM_ASSERT(curve);
	AM_ASSERT(u0 >= 0.0f && u0 <= 1.0f);
	AM_ASSERT(u1 >= 0.0f && u1 <= 1.0f);
	AM_ASSERT(u0 <= u1);

	amRombergf(&res, u0, u1, amBezier3fSpeedCallback, (void *)curve, AM_EPSILON_FLOAT);
	return res;
}

/*!
	\brief Given len = amBezier3fLen(curve, 0, u), this function calculates u.
	This function uses the Newton method to find the numerical root solution. This method has a quadratic convergence.
	\param result the output local parameter.
	\param curve input Bezier curve.
	\param len the curve length between 0 and the calculated local parameter.
	\return AM_TRUE if u was found in less than MAX_ITERATIONS steps (15), else AM_FALSE.
*/
AMbool amBezier3fParam(AMfloat *result,
					   const AMBezier3f *curve,
					   const AMfloat len) {

	#define MAX_ITERATIONS 15
	AMfloat totalLen, localLen, pivot, error, precision, l;
	AMVect2f tangent;
	AMuint32 i;

	AM_ASSERT(curve);
	AM_ASSERT(result);

	precision = 2.0f * AM_EPSILON_FLOAT;

	// check for out of range parameter
	if (len <= 0.0f) {
		*result = 0.0f;
		return AM_TRUE;
	}
	totalLen = amBezier3fLen(curve, 0.0f, 1.0f);
	if (len >= totalLen) {
		*result = 1.0f;
		return AM_TRUE;
	}

	// Newton's method for root searching; here we have to find root of function f = Length(u) - CurvePos
	// u is the value we are searching for
	localLen = len / totalLen;
	// here's a good starting point
	pivot = localLen;
	for (i = 0; i < MAX_ITERATIONS; ++i) {
		error = amBezier3fLen(curve, 0.0f, pivot) - len;
		// test relative error
		if (amAbsf(error / totalLen) <= precision) {
			*result = pivot;
			return AM_TRUE;
		}
		amBezier3fTan(&tangent, curve, pivot);
		l = AM_VECT2_SQR_LENGTH(&tangent);
		AM_ASSERT(l > AM_EPSILON_FLOAT);
		pivot -= error / amSqrtf(l);
		if (pivot < 0.0f)
			pivot = 0.001f;
		else
		if (pivot > 1.0f)
			pivot = 0.999f;
	}
	*result = pivot;
	return AM_FALSE;
	#undef MAX_ITERATIONS
}

/*!
	\brief Given a cubic Bezier curve and a local parameter, this function returns the left trait ([0; u] local parameter range).
	\param dst output (cut) Bezier curve.
	\param curve input Bezier curve.
	\param u local parameter.
	\pre 0.0f <= u <= 1.0f
*/
void amBezier3fCutLeft(AMBezier3f *dst,
					   const AMBezier3f *curve,
					   const AMfloat u) {

	AMfloat u1;
	AMVect2f q0, q1, q2;

	#define POINT_LERP(dest, pt0, w0, pt1, w1) \
		dest.x = pt0.x * w0 + pt1.x * w1; \
		dest.y = pt0.y * w0 + pt1.y * w1;

	AM_ASSERT(dst);
	AM_ASSERT(curve);
	AM_ASSERT(u >= 0.0f && u <= 1.0f);

	u1 = 1.0f - u;

	q0 = curve->p3;
	q1 = curve->p2;
	q2 = curve->p1;

	dst->p0 = curve->p0;

	POINT_LERP(q0, q0, u, q1, u1)
	POINT_LERP(q1, q1, u, q2, u1)
	POINT_LERP(q2, q2, u, curve->p0, u1)
	dst->p1 = q2;

	POINT_LERP(q0, q0, u, q1, u1)
	POINT_LERP(q1, q1, u, q2, u1)
	dst->p2 = q1;

	POINT_LERP(dst->p3, q0, u, q1, u1)
	// we can't use amBezier3Set, because it calculates also derivative coefficients amBezier3Set(dst, &q0, &q1, &q2, &q3)
	#undef POINT_LERP
}

/*!
	\brief Given a cubic Bezier curve and a local parameter, this function returns the right trait ([u; 1] local parameter range).
	\param dst output (cut) Bezier curve.
	\param curve input Bezier curve.
	\param u local parameter.
	\pre 0.0f <= u <= 1.0f
*/
void amBezier3fCutRight(AMBezier3f *dst,
						const AMBezier3f *curve,
						const AMfloat u) {

	AMfloat u1;
	AMVect2f q0, q1, q2;

	#define POINT_LERP(dest, pt0, w0, pt1, w1) \
		dest.x = pt0.x * w0 + pt1.x * w1; \
		dest.y = pt0.y * w0 + pt1.y * w1;

	AM_ASSERT(dst);
	AM_ASSERT(curve);
	AM_ASSERT(u >= 0.0f && u <= 1.0f);

	u1 = 1.0f - u;

	// right part
	q0 = curve->p0;
	q1 = curve->p1;
	q2 = curve->p2;

	dst->p3 = curve->p3;

	POINT_LERP(q0, q0, u1, q1, u)
	POINT_LERP(q1, q1, u1, q2, u)
	POINT_LERP(q2, q2, u1, curve->p3, u)
	dst->p2 = q2;

	POINT_LERP(q0, q0, u1, q1, u)
	POINT_LERP(q1, q1, u1, q2, u)
	dst->p1 = q1;

	POINT_LERP(dst->p0, q0, u1, q1, u)

	// we can't use amBezier3Set, because it calculates also derivative coefficients amBezier3Set(dst, &q0, &q1, &q2, &q3)
	#undef POINT_LERP
}

/*!
	\brief Given a monotone cubic Bezier curve, this function flattens it.
	\param points the output points array.
	\param curve input Bezier curve.
	\param params flattening thresholds.
	\param includeLastPoint if AM_TRUE the output array will contain the last control point too.
*/
void amBezier3fFlattenParabolic(AMVect2fDynArray *points,
								const AMBezier3f *curve,
								const AMFlattenParams *params,
								const AMbool includeLastPoint) {

	AMBezier3f bez, bez2;
	AMfloat v01x, v01y, t, y2, y3, vx, vy, w0, w1, w, d;

	AM_ASSERT(curve);
	AM_ASSERT(points);

	#define POINT_LERP(dest, pt0, w0, pt1, w1) \
		dest.x = pt0.x * w0 + pt1.x * w1; \
		dest.y = pt0.y * w0 + pt1.y * w1;

	bez.p0 = curve->p0;
	bez.p1 = curve->p1;
	bez.p2 = curve->p2;
	bez.p3 = curve->p3;
	
	AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, curve->p0)

	w = params->three_over_flatness;
	w0 = params->two_sqrt_flatness_over_three;
	w1 = params->two_cuberoot_flatness_over_three;

	while (1) {

		v01x = bez.p1.x - bez.p0.x;
		v01y = bez.p1.y - bez.p0.y;
		d = amSqrtf(v01x * v01x + v01y * v01y);
		if (d <= AM_EPSILON_FLOAT)
			break;

		v01x /= d;
		v01y /= d;

		// calculate x2, y2
		vx = bez.p2.x - bez.p0.x;
		vy = bez.p2.y - bez.p0.y;
		y2 = amAbsf(vy * v01x - vx * v01y);
		// calculate x3, y3
		vx = bez.p3.x - bez.p0.x;
		vy = bez.p3.y - bez.p0.y;
		y3 = (vy * v01x - vx * v01y);

		if (y3 * y3 > w * y2 * y2 * y2) {
			y3 = amAbsf(y3);
			if (y3 == 0.0f)
				break;

			t = w1 * amInvCubeRootf(y3);
		}
		else {
			if (y2 == 0.0f)
				break;
			t = w0 / amSqrtf(y2);
		}

		if (t + AM_EPSILON_FLOAT >= 1.0f)
			break;

		amBezier3fCutRight(&bez2, &bez, t);
		AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, bez2.p0)

		bez.p0 = bez2.p0;
		bez.p1 = bez2.p1;
		bez.p2 = bez2.p2;
		bez.p3 = bez2.p3;
	}
	if (includeLastPoint) {
		AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, curve->p3)
	}
}

/*!
	\brief Given a cubic Bezier curve, this function flattens it.
	\param points the output points array.
	\param curve input Bezier curve.
	\param params flattening thresholds.
	\param includeLastPoint if AM_TRUE the output array will contain the last control point too.
	\note the output points array will always include points of maximum, flex, cuspid.
*/
void amBezier3fFlatten(AMVect2fDynArray *points,
					   const AMBezier3f *curve,
					   const AMFlattenParams *params,
					   const AMbool includeLastPoint) {

	AMAABox2f tmpBox;
	AMVect2f p01, p02, p03;
	AMfloat boxW, boxH, d, x, y, a, b, c, t1, t2, dmax;
	AMBezier3f tmpBez, tmpBez2;
	AMfloat l, vx, vy, y2, y3;
	AMint32 n;

#define MAX_ABS_DIST(_dst, _p) \
	x = amAbsf((_p).x); \
	y = amAbsf((_p).y); \
	if (x >= y ) \
		_dst = x; \
	else \
		_dst = y;

	AM_ASSERT(points);
	AM_ASSERT(curve);
	
	AM_AABOX2_SET(&tmpBox, &curve->p0, &curve->p1)
	AM_AABOX2_EXTEND_TO_INCLUDE(&tmpBox, &curve->p2)
	AM_AABOX2_EXTEND_TO_INCLUDE(&tmpBox, &curve->p3)
	boxW = AM_AABOX2_WIDTH(&tmpBox);
	boxH = AM_AABOX2_HEIGHT(&tmpBox);
	dmax = AM_MAX(boxW, boxH);
	// p0, p1, p2, p3 coincident
	if (dmax <= AM_EPSILON_FLOAT) {
		if (includeLastPoint) {
			AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, curve->p0)
		}
		return;
	}
	else
	// take care of degenerated curves
	if (dmax < AM_BEZIER_DEGENERATION_THRESHOLD) {

		n = AM_MIN(params->degenerative_curve_segments, AM_MAX_POINTS_TO_FLATTEN_DEGENERATIONS);
		d = 1.0f / n;
		t1 = 0.0f;
		for (; n!= 0; --n) {
			amBezier3fEval(&p01, curve, t1);
			AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, p01)
			t1 += d;
		}

		if (includeLastPoint) {
			AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, curve->p3)
		}
		return;
	}

	AM_VECT2_SUB(&p01, &curve->p1, &curve->p0)
	AM_VECT2_SUB(&p02, &curve->p2, &curve->p0)
	AM_VECT2_SUB(&p03, &curve->p3, &curve->p0)

	p01.x /= dmax;
	p01.y /= dmax;
	p02.x /= dmax;
	p02.y /= dmax;
	p03.x /= dmax;
	p03.y /= dmax;

	MAX_ABS_DIST(d, p01)
	// p0, p1 coincident
	if (d <= AM_EPSILON_FLOAT) {

		MAX_ABS_DIST(d, p02)
		// p0, p1, p2 coincident
		if (d <= AM_EPSILON_FLOAT) {

			MAX_ABS_DIST(d, p03)
			// p0, p1, p2, p3 coincident
			if (d <= AM_EPSILON_FLOAT) {
				if (includeLastPoint) {
					AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, curve->p0)
				}
				return;
			}
			// just a line from p0 to p3
			else {
				AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, curve->p0)
				if (includeLastPoint) {
					AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, curve->p3)
				}
				return;
			}
		}
		// p0 and p1 coincident (p2 and p3 not coincident with p0/p1)
		else {
			AMfloat p02AbsX, p02AbsY;

			p02AbsX = amAbsf(p02.x);
			p02AbsY = amAbsf(p02.y);

			// move p1 along p0-p2 direction
			if (p02AbsX < p02AbsY) {

				if (p02AbsX <= AM_EPSILON_FLOAT)
					p01.y = (p02.y > 0.0f) ? (4.0f * AM_EPSILON_FLOAT) : (-4.0f * AM_EPSILON_FLOAT);
				else {
					p01.x = (p02.x > 0.0f) ? (4.0f * AM_EPSILON_FLOAT) : (-4.0f * AM_EPSILON_FLOAT);
					p01.y = (p01.x * p02.y) / p02.x;
					AM_ASSERT(amAbsf(p01.y) >= amAbsf(p01.x));
				}
			}
			else {
				if (p02AbsY <= AM_EPSILON_FLOAT)
					p01.x = (p02.x > 0.0f) ? (4.0f * AM_EPSILON_FLOAT) : (-4.0f * AM_EPSILON_FLOAT);
				else {
					p01.y = (p02.y > 0.0f) ? (4.0f * AM_EPSILON_FLOAT) : (-4.0f * AM_EPSILON_FLOAT);
					p01.x = (p01.y * p02.x) / p02.y;
					AM_ASSERT(amAbsf(p01.x) >= amAbsf(p01.y));
				}
			}

			tmpBez.p0 = curve->p0;
			tmpBez.p1.x = (p01.x * dmax) + curve->p0.x;
			tmpBez.p1.y = (p01.y * dmax) + curve->p0.y;
			tmpBez.p2 = curve->p2;
			tmpBez.p3 = curve->p3;
			l = amSqrtf(p02.x * p02.x + p02.y * p02.y);
			AM_ASSERT(l > 0.0f);
			vx = p02.x / l;
			vy = p02.y / l;
			y2 = -p02.x * vy + p02.y * vx;
			y3 = -p03.x * vy + p03.y * vx;
		}
	}
	else {
		tmpBez = *curve;
		l = amSqrtf(p01.x * p01.x + p01.y * p01.y);
		AM_ASSERT(l > 0.0f);
		vx = p01.x / l;
		vy = p01.y / l;
		y2 = -p02.x * vy + p02.y * vx;
		y3 = -p03.x * vy + p03.y * vx;
	}

	y2 = amAbsf(y2);
	y3 = amAbsf(y3);
	d = AM_MAX(y2, y3);
	if (params->sixtyfour_flatness > d * dmax) {

		a = 3.0f * (p01.x - p02.x) + p03.x;
		b = 2.0f * (p02.x - 2.0f * p01.x);
		c = p01.x;
		t1 = t2 = -1.0f;
		n = amQuadraticFormulaf(&t1, &t2, a, b, c);
		if (n == 0) {
			amBezier3fFlattenParabolic(points, &tmpBez, params, includeLastPoint);
			return;
		}
		else
		if (n == 1) {
			if (t1 > AM_EPSILON_FLOAT && (t1 + AM_EPSILON_FLOAT) < 1.0f) {
				amBezier3fCutLeft(&tmpBez2, &tmpBez, t1);
				amBezier3fFlattenParabolic(points, &tmpBez2, params, AM_FALSE);
				amBezier3fCutRight(&tmpBez2, &tmpBez, t1);
				amBezier3fFlattenParabolic(points, &tmpBez2, params, includeLastPoint);
				return;
			}
			else {
				amBezier3fFlattenParabolic(points, &tmpBez, params, includeLastPoint);
				return;
			}
		}
		else {
			AM_ASSERT(n == 2);

			if (t1 > t2) {
				d = t1;
				t1 = t2;
				t2 = d;
			}
			// if both solutions are not valid, do the classic parabolic schema
			if ((t1 <= AM_EPSILON_FLOAT || (t1 + AM_EPSILON_FLOAT) >= 1.0f) &&
				(t2 <= AM_EPSILON_FLOAT || (t2 + AM_EPSILON_FLOAT) >= 1.0f))
				amBezier3fFlattenParabolic(points, &tmpBez, params, includeLastPoint);
			else {
				// t1 valid
				if (t1 > AM_EPSILON_FLOAT && (t1 + AM_EPSILON_FLOAT) < 1.0f) {

					amBezier3fCutLeft(&tmpBez2, &tmpBez, t1);
					amBezier3fFlattenParabolic(points, &tmpBez2, params, AM_FALSE);
					amBezier3fCutRight(&tmpBez2, &tmpBez, t1);

					// t1 and t1 are valid
					if (t2 > AM_EPSILON_FLOAT && (t2 + AM_EPSILON_FLOAT) < 1.0f) {

						tmpBez = tmpBez2;
						t2 = (t2 - t1) / (1.0f - t1);
						AM_ASSERT(t2 > AM_EPSILON_FLOAT && (t2 + AM_EPSILON_FLOAT) < 1.0f);
						amBezier3fCutLeft(&tmpBez2, &tmpBez, t2);
						amBezier3fFlattenParabolic(points, &tmpBez2, params, AM_FALSE);
						amBezier3fCutRight(&tmpBez2, &tmpBez, t2);
						amBezier3fFlattenParabolic(points, &tmpBez2, params, includeLastPoint);
					}
					// t1 valid, t2 invalid
					else {
						amBezier3fFlattenParabolic(points, &tmpBez2, params, includeLastPoint);
					}
				}
				// t1 invalid, t2 valid
				else {
					AM_ASSERT(t2 > AM_EPSILON_FLOAT && (t2 + AM_EPSILON_FLOAT) < 1.0f);

					amBezier3fCutLeft(&tmpBez2, &tmpBez, t2);
					amBezier3fFlattenParabolic(points, &tmpBez2, params, AM_FALSE);
					amBezier3fCutRight(&tmpBez2, &tmpBez, t2);
					amBezier3fFlattenParabolic(points, &tmpBez2, params, includeLastPoint);
				}
			}
		}
	}
	else
		amBezier3fFlattenParabolic(points, &tmpBez, params, includeLastPoint);
	#undef MAX_ABS_DIST
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif


