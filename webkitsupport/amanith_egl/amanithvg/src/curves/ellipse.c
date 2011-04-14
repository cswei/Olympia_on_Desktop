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
	\file ellipse.c
	\brief Ellipse curves, implememtation.
	\author Matteo Muratori
	\author Michele Fabbri
*/
#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif


#include "ellipse.h"
#include "integration.h"

/*!
	\brief Wrap the specified angle in the range [0; 2pi].
	\param angle input angle in radians.
	\return output wrapped angle.
*/
AMfloat amAngleWrap(const AMfloat angle) {

	AMfloat n;

	if (angle < 0.0f) {
		n = amCeilf(-angle / AM_2PI);
		return (angle + n * AM_2PI);
	}
	if (angle > AM_2PI) {
		n = amFloorf(angle / AM_2PI);
		return (angle - n * AM_2PI);
	}
	else
		return angle;
}

/*!
	\brief Given an ellipse curve, it checks if the specified angle falls inside the domain.
	\param curve input ellipse curve.
	\param angle input angle in radians.
	\return AM_TRUE if the specified angle falls inside the domain, else AM_FALSE.
*/
AMbool amEllipsefAngleIncluded(const AMEllipsef *curve,
							   const AMfloat angle) {

	AMfloat t = amAngleWrap(angle);

	AM_ASSERT(curve);

	if (curve->ccw) {
		if (curve->startAngle < curve->endAngle)
			return (t >= curve->startAngle && t <= curve->endAngle) ? AM_TRUE : AM_FALSE;
		else
			return ((t >= curve->startAngle) || (t <= curve->endAngle)) ? AM_TRUE : AM_FALSE;
	}
	// cw
	else {
		if (curve->startAngle < curve->endAngle)
			return ((t <= curve->startAngle) || (t >= curve->endAngle)) ? AM_TRUE : AM_FALSE;
		else
			return (t >= curve->endAngle && t <= curve->startAngle) ? AM_TRUE : AM_FALSE;
	}
}

/*!
	\brief Ellipse constructor given center, axes, rotation, angle domain and direction (cw/ccw).
	\param dst output ellipse.
	\param center center of the ellipse.
	\param xSemiAxisLength length of horizontal semi-axis.
	\param ySemiAxisLength length of vertical semi-axis.
	\param offsetRotation rotation (in radians) relative to the horizontal axis.
	\param startAngle start angle corresponding to the domain lower bound.
	\param endAngle end angle corresponding to the domain upper bound.
	\param ccw AM_TRUE for ccw direction, AM_FALSE for cw direction.
	\pre xSemiAxisLength >= 0.0f, ySemiAxisLength >= 0.0f
*/
void amEllipsefSetByAxes(AMEllipsef *dst,
						 const AMVect2f *center,
						 const AMfloat xSemiAxisLength,
						 const AMfloat ySemiAxisLength,
						 const AMfloat offsetRotation,
						 const AMfloat startAngle,
						 const AMfloat endAngle,
						 const AMbool ccw) {

	AM_ASSERT(dst);
	AM_ASSERT(center);
	AM_ASSERT(xSemiAxisLength >= 0.0f);
	AM_ASSERT(ySemiAxisLength >= 0.0f);

	dst->center = *center;
	dst->xSemiAxisLength = xSemiAxisLength;
	dst->ySemiAxisLength = ySemiAxisLength;
	dst->offsetRotation = offsetRotation;
	amSinCosf(&dst->sinOfsRot, &dst->cosOfsRot, offsetRotation);
	dst->ccw = ccw;
	// set angle domain
	dst->startAngle = amAngleWrap(startAngle);
	dst->endAngle = amAngleWrap(endAngle);
}

/*!
	\brief Given two points p0 and p1, return AM_TRUE if two unit circles can be found so that they interpolate
	p0 and p1. In this case centers of these circles are returned in c0 and c1, else an AM_FALSE value
	is returned and growingFactor will contain the smallest factor to scale p0 and p1 (along p0-p1 direction)
	so that unit circles can be found.
	\param c0 first output center, if p0 and p1 can be interpolated
	\param c1 second output center, if p0 and p1 can be interpolated
	\param growingFactor the smallest factor to scale p0 and p1 so that unit circles can be found.
	\param p0 first point to interpolate.
	\param p1 second point to interpolate.
	\return AM_TRUE if two unit circles have been found so that they interpolate p0 and p1, else AM_FALSE.
*/
AMbool amUnitCirclesFind(AMVect2f *c0,
						 AMVect2f *c1,
						 AMfloat *growingFactor,
						 const AMVect2f *p0,
						 const AMVect2f *p1) {

	AMVect2f d, pm;
	AMfloat dsq, disc, s, sdx, sdy;

	AM_ASSERT(p0);
	AM_ASSERT(p1);
	AM_ASSERT(c0);
	AM_ASSERT(c1);
	AM_ASSERT(growingFactor);

	AM_VECT2_SUB(&d, p1, p0);
	AM_VECT2_SET(&pm, (p1->x + p0->x) * 0.5f, (p1->y + p0->y) * 0.5f)

	// solve for intersecting unit circles
	dsq = AM_VECT2_SQR_LENGTH(&d);
	if (dsq <= AM_EPSILON_FLOAT) {
		// points are coincident
		*growingFactor = 0.0f;
		return AM_FALSE;
	}
	disc = (1.0f / dsq) - 0.25f;
	if (disc < 0.0f) {
		// points are too far apart, we must calculate the smallest factor that permits a solution
		*growingFactor = amSqrtf(dsq) * 0.5f;
		return AM_FALSE;
	}
	// two distinct centers
	s = amSqrtf(disc);
	sdx = s * d.x;
	sdy = s * d.y;
	AM_VECT2_SET(c0, pm.x + sdy, pm.y - sdx)
	AM_VECT2_SET(c1, pm.x - sdy, pm.y + sdx)
	return AM_TRUE;
}

/*!
	\brief Ellipse constructor given 2 points, axes, rotation, small/large arc flag and direction (cw/ccw).
	\param dst output ellipse.
	\param q0 first point to interpolate.
	\param q1 second point to interpolate.
	\param xSemiAxisLength length of horizontal semi-axis.
	\param ySemiAxisLength length of vertical semi-axis.
	\param offsetRotation rotation (in radians) relative to the horizontal axis.
	\param largeArc AM_TRUE to choose the largest arc, AM_FALSE to choose the smallest one.
	\param ccw AM_TRUE for ccw direction, AM_FALSE for cw direction.
	\pre xSemiAxisLength >= 0.0f, ySemiAxisLength >= 0.0f
	\note REFERENCE MISMATCH: "If exactly one of rh and rv is 0, and the arc endpoints are not coincident, the arc is drawn
	as if it were projected onto the line containing the endpoints". Reference implementation draws the line between endpoints.
*/
AMbool amEllipsefSetByPoints(AMEllipsef *dst,
							 const AMVect2f *q0,
							 const AMVect2f *q1,
							 const AMfloat xSemiAxisLength,
							 const AMfloat ySemiAxisLength,
							 const AMfloat offsetRotation,
							 const AMbool largeArc,
							 const AMbool ccw) {

	// pre-compute rotation matrix entries
	AMfloat c, s, dx, dy, d;
	AMfloat theta0, theta1, theta2, theta3, gf = 1.0f, oneOverGf, cross0;
	AMfloat xSemiAxis = xSemiAxisLength;
	AMfloat ySemiAxis = ySemiAxisLength;
	AMVect2f p0, p1, c0, c1, cc0, cc1, tmp0, tmp1, v0, v1;

	AM_ASSERT(dst);
	AM_ASSERT(q0);
	AM_ASSERT(q1);
	AM_ASSERT(xSemiAxisLength >= 0.0f);
	AM_ASSERT(ySemiAxisLength >= 0.0f);

	dx = amAbsf(q1->x - q0->x);
	dy = amAbsf(q1->y - q0->y);
	d = AM_MAX(dx, dy);
	// coincident points or both semi axes length are 0 (OpenVG 1.0.1: "If both rh and rv are 0, or if the arc
	// endpoints are coincident, the arc is drawn as a line segment between its endpoints").
	if (d <= AM_EPSILON_FLOAT || (xSemiAxis <= AM_EPSILON_FLOAT && ySemiAxis <= AM_EPSILON_FLOAT)) {
		amEllipsefSetByAxes(dst, q0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, AM_TRUE);
		return AM_TRUE;
	}
	// N.B.: REFERENCE MISMATCH
	// "If exactly one of rh and rv is 0, and the arc endpoints are not coincident, the arc is drawn
	// as if it were projected onto the line containing the endpoints".
	// Reference implementation draws the line between endpoints.
#if 0
	if (xSemiAxis <= AM_EPSILON_FLOAT || ySemiAxis <= AM_EPSILON_FLOAT) {
		c0.x = (q0->x + q1->x) * 0.5f;
		c0.y = (q0->y + q1->y) * 0.5f;
		theta0 = amAtan2f(q1->y - q0->y, q1->x - q0->x);
		xSemiAxis = amSqrtf((c0.x - q0->x) * (c0.x - q0->x) + (c0.y - q0->y) * (c0.y - q0->y));
		AM_ASSERT(xSemiAxis > AM_EPSILON_FLOAT);
		amEllipsefSetByAxes(dst, &c0, xSemiAxis, 0.0f, theta0, AM_PI, 0.0f, ccw);
		return AM_TRUE;
	}
#endif
	if (xSemiAxis <= AM_EPSILON_FLOAT)
		xSemiAxis = 2.0f * AM_EPSILON_FLOAT;
	else
	if (ySemiAxis <= AM_EPSILON_FLOAT)
		ySemiAxis = 2.0f * AM_EPSILON_FLOAT;

	amSinCosf(&s, &c, offsetRotation);
	// transform (x0, y0) and (x1, y1) into unit space using (inverse) rotate, followed by (inverse) scale
	AM_VECT2_SET(&p0, (q0->x * c + q0->y * s) / xSemiAxis, (-q0->x * s + q0->y * c) / ySemiAxis)
	AM_VECT2_SET(&p1, (q1->x * c + q1->y * s) / xSemiAxis, (-q1->x * s + q1->y * c) / ySemiAxis)

	if (!amUnitCirclesFind(&c0, &c1, &gf, &p0, &p1)) {
		if (gf == 0.0f)
			return AM_FALSE;

		oneOverGf = 1.0f / gf;

		AM_VECT2_MUL(&tmp0, oneOverGf, &p0);
		AM_VECT2_MUL(&tmp1, oneOverGf, &p1);

		AM_VECT2_SET(&c0, (tmp0.x + tmp1.x) * 0.5f, (tmp0.y + tmp1.y) * 0.5f)
		AM_VECT2_SET(&c1, (tmp0.x + tmp1.x) * 0.5f, (tmp0.y + tmp1.y) * 0.5f)
		theta0 = theta2 = amAtan2f(oneOverGf * p0.y - c0.y, oneOverGf * p0.x - c0.x);
		theta1 = theta3 = amAtan2f(oneOverGf * p1.y - c0.y, oneOverGf * p1.x - c0.x);
	}
	else {
		theta0 = amAtan2f(p0.y - c0.y, p0.x - c0.x);
		theta1 = amAtan2f(p1.y - c0.y, p1.x - c0.x);
		theta2 = amAtan2f(p0.y - c1.y, p0.x - c1.x);
		theta3 = amAtan2f(p1.y - c1.y, p1.x - c1.x);
	}

	// transform back to original coordinate space using (forward) scale followed by (forward) rotate
	c0.x *= (xSemiAxis * gf);
	c0.y *= (ySemiAxis * gf);
	c1.x *= (xSemiAxis * gf);
	c1.y *= (ySemiAxis * gf);

	// cc0 and cc1 now are the final centers
	AM_VECT2_SET(&cc0, c0.x * c - c0.y * s, c0.x * s + c0.y * c)
	AM_VECT2_SET(&cc1, c1.x * c - c1.y * s, c1.x * s + c1.y * c)
	AM_VECT2_SUB(&v0, q0, &cc0);
	AM_VECT2_SUB(&v1, q1, &cc0);

	// cross0 < 0 means that v0 span the greater angle in ccw to overlap v1
	cross0 = AM_VECT2_CROSS(&v0, &v1);

	if (largeArc) {
		if (cross0 < 0.0f) {
			if (ccw)
				// ellipse cc0
				amEllipsefSetByAxes(dst, &cc0, xSemiAxis * gf, ySemiAxis * gf, offsetRotation, theta0, theta1, AM_TRUE);
			else
				// ellipse cc1
				amEllipsefSetByAxes(dst, &cc1, xSemiAxis * gf, ySemiAxis * gf, offsetRotation, theta2, theta3, AM_FALSE);
		}
		else {
			if (ccw)
				// ellipse cc1
				amEllipsefSetByAxes(dst, &cc1, xSemiAxis * gf, ySemiAxis * gf, offsetRotation, theta2, theta3, AM_TRUE);
			else
				// ellipse cc0
				amEllipsefSetByAxes(dst, &cc0, xSemiAxis * gf, ySemiAxis * gf, offsetRotation, theta0, theta1, AM_FALSE);
		}
	}
	else {
		if (cross0 > 0.0f) {
			if (ccw)
				// ellipse cc0
				amEllipsefSetByAxes(dst, &cc0, xSemiAxis * gf, ySemiAxis * gf, offsetRotation, theta0, theta1, AM_TRUE);
			else
				// ellipse cc1
				amEllipsefSetByAxes(dst, &cc1, xSemiAxis * gf, ySemiAxis * gf, offsetRotation, theta2, theta3, AM_FALSE);
		}
		else {
			if (ccw)
				// ellipse cc1
				amEllipsefSetByAxes(dst, &cc1, xSemiAxis * gf, ySemiAxis * gf, offsetRotation, theta2, theta3, AM_TRUE);
			else
				// ellipse cc0
				amEllipsefSetByAxes(dst, &cc0, xSemiAxis * gf, ySemiAxis * gf, offsetRotation, theta0, theta1, AM_FALSE);
		}
	}
	return AM_TRUE;
}

/*!
	\brief Given a local parameter between 0 and 1, it returns the corresponding domain angle value.
	\param curve input ellipse curve.
	\param u local parameter.
	\return output mapped angle (in radians).
	\pre 0.0f <= u <= 1.0f
*/
AMfloat amEllipsefParamToAngle(const AMEllipsef *curve,
							   const AMfloat u) {

	AMfloat res;

	AM_ASSERT(curve);
	AM_ASSERT(u >= 0.0f && u <= 1.0f);

	if (curve->ccw) {
		if (curve->startAngle < curve->endAngle)
			return amLerpf(u, curve->startAngle, curve->endAngle);
		else {
			res = curve->startAngle + u * (AM_2PI - curve->startAngle + curve->endAngle);
			if (res > AM_2PI)
				res -= AM_2PI;
			return res;
		}
	}
	// cw
	else {
		if (curve->startAngle < curve->endAngle) {
			res = curve->startAngle - u * (AM_2PI - curve->endAngle + curve->startAngle);
			if (res < 0.0f)
				res += AM_2PI;
			return res;
		}
		else
			return (curve->startAngle - u * (curve->startAngle - curve->endAngle));
	}
}

/*!
	\brief Ellipse evaluation. It evaluates an ellipse position given an angle that falls inside the domain.
	\param dst output coordinates.
	\param curve input ellipse curve.
	\param angle angle at which evaluate the ellipse.
*/
void amEllipsefEvalByAngle(AMVect2f *dst,
						   const AMEllipsef *curve,
						   const AMfloat angle) {

	AMVect2f p;

	AM_ASSERT(dst);
	AM_ASSERT(curve);
	AM_ASSERT(curve->xSemiAxisLength >= 0.0f);
	AM_ASSERT(curve->ySemiAxisLength >= 0.0f);

	AM_VECT2_SET(&p, curve->xSemiAxisLength * amCosf(angle), curve->ySemiAxisLength * amSinf(angle))

	AM_VECT2_SET(dst, (p.x * curve->cosOfsRot - p.y * curve->sinOfsRot) + curve->center.x,
					  (p.x * curve->sinOfsRot + p.y * curve->cosOfsRot) + curve->center.y)
}

/*!
	\brief Ellipse evaluation. It evaluates an ellipse position given the local parameter.
	\param dst output coordinates.
	\param curve input ellipse curve.
	\param u local parameter.
	\pre 0.0f <= u <= 1.0f
*/
void amEllipsefEval(AMVect2f *dst,
					const AMEllipsef *curve,
					const AMfloat u) {

	AM_ASSERT(dst);
	AM_ASSERT(curve);
	AM_ASSERT(u >= 0.0f && u <= 1.0f);
	AM_ASSERT(curve->xSemiAxisLength >= 0.0f);
	AM_ASSERT(curve->ySemiAxisLength >= 0.0f);

	if (curve->xSemiAxisLength <= AM_EPSILON_FLOAT) {
		// both axis are 0-length, the ellipse is collapsed into one point
		if (curve->ySemiAxisLength <= AM_EPSILON_FLOAT) {
			*dst = curve->center;
			return;
		}
		// only y axis is non 0-length, the ellipse is collapsed into a line segment
		else {
			AMVect2f startP, endP;

			amEllipsefEvalByAngle(&startP, curve, curve->startAngle);
			amEllipsefEvalByAngle(&endP, curve, curve->endAngle);
			AM_VECT2_SET(dst, amLerpf(u, startP.x, endP.x), amLerpf(u, startP.y, endP.y))
			return;
		}
	}
	else
	// only x axis is non 0-length, the ellipse is collapsed into a line segment
	if (curve->ySemiAxisLength <= AM_EPSILON_FLOAT) {

		AMVect2f startP, endP;

		amEllipsefEvalByAngle(&startP, curve, curve->startAngle);
		amEllipsefEvalByAngle(&endP, curve, curve->endAngle);
		AM_VECT2_SET(dst, amLerpf(u, startP.x, endP.x), amLerpf(u, startP.y, endP.y))
		return;
	}
	// non degenerated ellipse
	else
		amEllipsefEvalByAngle(dst, curve, amEllipsefParamToAngle(curve, u));
}

/*!
	\brief Ellipse tangent evaluation. It evaluates an ellipse tangent vector given the local parameter.
	\param dst output tangent vector.
	\param curve input ellipse curve.
	\param u local parameter.
	\pre 0.0f <= u <= 1.0f
*/
void amEllipsefTan(AMVect2f *dst,
				   const AMEllipsef *curve,
				   const AMfloat u) {

	AMfloat l, angle = amEllipsefParamToAngle(curve, u);
	AMVect2f v;

	AM_ASSERT(dst);
	AM_ASSERT(curve);
	AM_ASSERT(u >= 0.0f && u <= 1.0f);
	AM_ASSERT(curve->xSemiAxisLength >= 0.0f);
	AM_ASSERT(curve->ySemiAxisLength >= 0.0f);

	if (curve->xSemiAxisLength <= AM_EPSILON_FLOAT) {
		// both axis are 0-length, the ellipse is collapsed into one point
		if (curve->ySemiAxisLength <= AM_EPSILON_FLOAT) {
			AM_VECT2_SET(dst, 0.0f, 0.0f)
			return;
		}
		// only y axis is non 0-length, the ellipse is collapsed into a line segment
		else {
			AMVect2f startP, endP;

			amEllipsefEvalByAngle(&startP, curve, curve->startAngle);
			amEllipsefEvalByAngle(&endP, curve, curve->endAngle);
			AM_VECT2_SUB(dst, &endP, &startP)
			return;
		}
	}
	else
	// only x axis is non 0-length, the ellipse is collapsed into a line segment
	if (curve->ySemiAxisLength <= AM_EPSILON_FLOAT) {

		AMVect2f startP, endP;

		amEllipsefEvalByAngle(&startP, curve, curve->startAngle);
		amEllipsefEvalByAngle(&endP, curve, curve->endAngle);
		AM_VECT2_SUB(dst, &endP, &startP)
		return;
	}
	// non degenerated ellipse
	else {
		if (curve->ccw) {
			if (curve->startAngle < curve->endAngle)
				l = (curve->endAngle - curve->startAngle);
			else
				l = (AM_2PI - curve->startAngle + curve->endAngle);
		}
		// cw
		else {
			if (curve->startAngle < curve->endAngle)
				l = -(AM_2PI - curve->endAngle + curve->startAngle);
			else
				l = -(curve->startAngle - curve->endAngle);
		}

		AM_VECT2_SET(&v, (-curve->xSemiAxisLength * amSinf(angle)) * l, (curve->ySemiAxisLength * amCosf(angle)) * l)
		AM_VECT2_SET(dst, v.x * curve->cosOfsRot - curve->sinOfsRot * v.y, v.x * curve->sinOfsRot + v.y * curve->cosOfsRot)
	}
}

/*!
	\brief Ellipse speed evaluation callback. It evaluates the length of the ellipse tangent vector, given the local parameter.
	\param u local parameter.
	\param userData input ellipse curve, specified as a void pointer.
	\return the length of the ellipse tangent vector.
	\pre 0.0f <= u <= 1.0f
	\note curve is specified as a void pointer, in order to have a more flexible way to use it as a callback inside
	the Romberg integrator function.
*/
AMfloat amEllipsefSpeedCallback(const AMfloat u,
								void *userData) {

	const AMEllipsef *curve = (const AMEllipsef *)userData;
	AMVect2f tangent;

	amEllipsefTan(&tangent, curve, u);
	return amSqrtf(AM_VECT2_SQR_LENGTH(&tangent));
}

/*!
	\brief Ellipse length evaluation. It evaluates an ellipse length given a local parameter range.
	\param curve input ellipse curve.
	\param u0 min local parameter.
	\param u1 max local parameter.
	\return the length of the ellipse trait.
	\pre 0.0f <= u0 <= u1 <= 1.0f
*/
AMfloat amEllipsefLen(const AMEllipsef *curve,
					  const AMfloat u0,
					  const AMfloat u1) {

	AM_ASSERT(curve);
	AM_ASSERT(u0 >= 0.0f && u0 <= 1.0f);
	AM_ASSERT(u1 >= 0.0f && u1 <= 1.0f);
	AM_ASSERT(u0 <= u1);
	AM_ASSERT(curve->xSemiAxisLength >= 0.0f);
	AM_ASSERT(curve->ySemiAxisLength >= 0.0f);

	if (curve->xSemiAxisLength <= AM_EPSILON_FLOAT) {
		// both axis are 0-length, the ellipse is collapsed into one point
		if (curve->ySemiAxisLength <= AM_EPSILON_FLOAT) {
			return 0.0f;
		}
		// only y axis is non 0-length, the ellipse is collapsed into a line segment
		else {
			AMVect2f startP, endP, p0, p1;

			amEllipsefEvalByAngle(&startP, curve, curve->startAngle);
			amEllipsefEvalByAngle(&endP, curve, curve->endAngle);
			AM_VECT2_SET(&p0, amLerpf(u0, startP.x, endP.x), amLerpf(u0, startP.y, endP.y))
			AM_VECT2_SET(&p1, amLerpf(u1, startP.x, endP.x), amLerpf(u1, startP.y, endP.y))
			return amSqrtf(AM_VECT2_SQR_DISTANCE(&p1, &p0));
		}
	}
	else
	// only x axis is non 0-length, the ellipse is collapsed into a line segment
	if (curve->ySemiAxisLength <= AM_EPSILON_FLOAT) {

		AMVect2f startP, endP, p0, p1;

		amEllipsefEvalByAngle(&startP, curve, curve->startAngle);
		amEllipsefEvalByAngle(&endP, curve, curve->endAngle);
		AM_VECT2_SET(&p0, amLerpf(u0, startP.x, endP.x), amLerpf(u0, startP.y, endP.y))
		AM_VECT2_SET(&p1, amLerpf(u1, startP.x, endP.x), amLerpf(u1, startP.y, endP.y))
		return amSqrtf(AM_VECT2_SQR_DISTANCE(&p1, &p0));
	}
	// non degenerated ellipse
	else {
		AMfloat res;

		amRombergf(&res, u0, u1, amEllipsefSpeedCallback, (void *)curve, AM_EPSILON_FLOAT);
		return res;
	}
}

/*!
	\brief Given len = amEllipsefLen(curve, 0, u), this function calculates u.
	This function uses the Newton method to find the numerical root solution. This method has a quadratic convergence.
	\param result the output local parameter.
	\param curve input ellipse curve.
	\param len the curve length between 0 and the calculated local parameter.
	\return AM_TRUE if u was found in less than MAX_ITERATIONS steps (15), else AM_FALSE.
*/
AMbool amEllipsefParam(AMfloat *result,
					   const AMEllipsef *curve,
					   const AMfloat len) {

	#define MAX_ITERATIONS 15
	AMfloat totalLen, localLen, pivot, error, precision, l;
	AMVect2f tangent;
	AMuint32 i;

	AM_ASSERT(curve);
	AM_ASSERT(result);
	AM_ASSERT(curve->xSemiAxisLength >= 0.0f);
	AM_ASSERT(curve->ySemiAxisLength >= 0.0f);

	precision = 2.0f * AM_EPSILON_FLOAT;

	// check for out of range parameter
	if (len <= 0.0f) {
		*result = 0.0f;
		return AM_TRUE;
	}
	totalLen = amEllipsefLen(curve, 0.0f, 1.0f);
	if (len >= totalLen) {
		*result = 1.0f;
		return AM_TRUE;
	}

	if (curve->xSemiAxisLength <= AM_EPSILON_FLOAT) {
		// both axis are 0-length, the ellipse is collapsed into one point
		if (curve->ySemiAxisLength <= AM_EPSILON_FLOAT) {
			*result = 0.0;
			return AM_TRUE;
		}
		// only y axis is non 0-length, the ellipse is collapsed into a line segment
		else {
			AM_ASSERT(totalLen > AM_EPSILON_FLOAT);
			*result = len / totalLen;
			return AM_TRUE;
		}
	}
	else
	// only x axis is non 0-length, the ellipse is collapsed into a line segment
	if (curve->ySemiAxisLength <= AM_EPSILON_FLOAT) {
		AM_ASSERT(totalLen > AM_EPSILON_FLOAT);
		*result = len / totalLen;
		return AM_TRUE;
	}
	// non degenerated ellipse
	else {
		// Newton's method for root searching; here we have to find root of function f = Length(u) - CurvePos
		// u is the value we are searching for
		localLen = len / totalLen;
		// here's a good starting point
		pivot = localLen;
		for (i = 0; i < MAX_ITERATIONS; ++i) {
			error = amEllipsefLen(curve, 0.0f, pivot) - len;
			// test relative error
			if (amAbsf(error / totalLen) <= precision) {
				*result = pivot;
				return AM_TRUE;
			}
			amEllipsefTan(&tangent, curve, pivot);
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
	}
	#undef MAX_ITERATIONS
}

/*!
	\brief Given an ellipse curve, this function flattens it.
	\param points the output points array.
	\param curve input ellipse curve.
	\param params flattening thresholds.
	\param includeLastPoint if AM_TRUE the output array will contain the last control point too.
*/
void amEllipsefFlatten(AMVect2fDynArray *points,
					   const AMEllipsef *curve,
					   const AMFlattenParams *params,
					   const AMbool includeLastPoint) {

	AMuint32 n = 3;
	AMfloat r, beta, dev, n1, aOverb, bOvera, devOverRadius;
	AMfloat deltaAngle, cosDelta, sinDelta;
	AMVect2f p, q, w, tmp;
	AMuint32 i;

	#define TANGENT_PRECISION 0.001f

	AM_ASSERT(points);
	AM_ASSERT(curve);

	if (curve->xSemiAxisLength <= AM_EPSILON_FLOAT) {
		// both axis are 0-length, the ellipse is collapsed into one point
		if (curve->ySemiAxisLength <= AM_EPSILON_FLOAT) {
			AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, curve->center)
			return;
		}
		// only y axis is non 0-length, the ellipse is collapsed into a line segment
		else {
			AMVect2f startP, endP;

			amEllipsefEvalByAngle(&startP, curve, curve->startAngle);
			amEllipsefEvalByAngle(&endP, curve, curve->endAngle);
			AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, startP)
			AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, endP)
			return;
		}
	}
	else
	// only x axis is non 0-length, the ellipse is collapsed into a line segment
	if (curve->ySemiAxisLength <= AM_EPSILON_FLOAT) {

		AMVect2f startP, endP;

		amEllipsefEvalByAngle(&startP, curve, curve->startAngle);
		amEllipsefEvalByAngle(&endP, curve, curve->endAngle);
		AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, startP)
		AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, endP)
		return;
	}
	// non degenerated ellipse
	else {
		AMfloat tmpAngle;

		// first find major semi-axis length
		r = AM_MAX(curve->xSemiAxisLength, curve->ySemiAxisLength);

		// now calculate the number of segments to produce (number of times we have to subdivide angle) that
		// permit to have a squared chordal distance less than MaxDeviation
		dev = AM_CLAMP(params->flatness, AM_EPSILON_FLOAT, r - (AM_EPSILON_FLOAT * r));

		if (curve->ccw)
			beta = (curve->startAngle < curve->endAngle) ? curve->endAngle - curve->startAngle : AM_2PI - curve->startAngle + curve->endAngle;
		// cw
		else
			beta = (curve->startAngle < curve->endAngle) ? AM_2PI - curve->endAngle + curve->startAngle : curve->startAngle - curve->endAngle;

		devOverRadius = dev / r;
		n1 = (amAbsf(devOverRadius) <= AM_EPSILON_FLOAT) ? beta : beta / (2.0f * amAcosf(1.0f - devOverRadius));
		if (n1 > (AMfloat)n)
			n = (AMuint32)amCeilf(n1);

		aOverb = curve->xSemiAxisLength / curve->ySemiAxisLength;
		bOvera = curve->ySemiAxisLength / curve->xSemiAxisLength;
		deltaAngle = beta / (AMfloat)n;

		tmpAngle = (curve->ccw) ? deltaAngle : -deltaAngle;
		amSinCosf(&sinDelta, &cosDelta, tmpAngle);

		// push first point
		AM_VECT2_SET(&p, curve->xSemiAxisLength * amCosf(curve->startAngle),
						 curve->ySemiAxisLength * amSinf(curve->startAngle))
		AM_VECT2_SET(&q, curve->cosOfsRot * p.x - curve->sinOfsRot * p.y + curve->center.x,
						 curve->sinOfsRot * p.x + curve->cosOfsRot * p.y + curve->center.y)
		AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, q)

		// this point grants (almost) correct start tangent
		AM_VECT2_SET(&tmp, curve->xSemiAxisLength * amCosf(curve->startAngle + tmpAngle * TANGENT_PRECISION),
						   curve->ySemiAxisLength * amSinf(curve->startAngle + tmpAngle * TANGENT_PRECISION))
		AM_VECT2_SET(&q, curve->cosOfsRot * tmp.x - curve->sinOfsRot * tmp.y + curve->center.x,
						 curve->sinOfsRot * tmp.x + curve->cosOfsRot * tmp.y + curve->center.y)
		AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, q)

		// generate the other points
		for (i = 0; i < n - 1; ++i) {
			AM_VECT2_SET(&w, p.x * cosDelta - aOverb * p.y * sinDelta, bOvera * p.x * sinDelta + p.y * cosDelta)
			AM_VECT2_SET(&q, curve->cosOfsRot * w.x - curve->sinOfsRot * w.y + curve->center.x,
							 curve->sinOfsRot * w.x + curve->cosOfsRot * w.y + curve->center.y)
			AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, q)
			p = w;
		}

		// this point grants (almost) correct end tangent
		AM_VECT2_SET(&tmp, curve->xSemiAxisLength * amCosf(curve->endAngle - tmpAngle * TANGENT_PRECISION),
						   curve->ySemiAxisLength * amSinf(curve->endAngle - tmpAngle * TANGENT_PRECISION))
		AM_VECT2_SET(&q, curve->cosOfsRot * tmp.x - curve->sinOfsRot * tmp.y + curve->center.x,
						 curve->sinOfsRot * tmp.x + curve->cosOfsRot * tmp.y + curve->center.y)
		AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, q)

		// check for last point
		if (includeLastPoint) {
			// analitycal calculus, just to avoid accumulated error during the incremental schema
			AM_VECT2_SET(&p, curve->xSemiAxisLength * amCosf(curve->endAngle),
							 curve->ySemiAxisLength * amSinf(curve->endAngle))
			AM_VECT2_SET(&q, curve->cosOfsRot * p.x - curve->sinOfsRot * p.y + curve->center.x,
							 curve->sinOfsRot * p.x + curve->cosOfsRot * p.y + curve->center.y)
			AM_DYNARRAY_PUSH_BACK((*points), AMVect2f, q)
		}
	}

	#undef TANGENT_PRECISION
}

/*!
	\brief Given an angle domain and a direction, it returns AM_TRUE if they identify the largest arc, else AM_FALSE.
	\param startAngle start angle corresponding to the domain lower bound.
	\param endAngle end angle corresponding to the domain upper bound.
	\param ccw AM_TRUE for ccw direction, AM_FALSE for cw direction.
	\return AM_TRUE if specified parameters identify the largest arc, else AM_FALSE.
*/
AMbool amAngleDomainLarge(const AMfloat startAngle,
						  const AMfloat endAngle,
						  const AMbool ccw) {

	AMfloat l;

	if (ccw)
		l = (startAngle < endAngle) ? (endAngle - startAngle) : (AM_2PI - startAngle + endAngle);
	// cw
	else
		l = (startAngle < endAngle) ? AM_2PI - endAngle + startAngle : (startAngle - endAngle);

	return (l <= AM_PI) ? AM_FALSE : AM_TRUE;
}

/*!
	\brief Given an ellipse, it returns AM_TRUE if its internal parameters identify the largest arc, else AM_FALSE.
	\param ellipse input ellipse curve.
	\return AM_TRUE if ellipse's parameters identify the largest arc, else AM_FALSE.
*/
AM_INLINE AMbool amEllipsefLargeArc(const AMEllipsef *ellipse) {

	return amAngleDomainLarge(ellipse->startAngle, ellipse->endAngle, ellipse->ccw);
}

/*!
	\brief Transform an ellipse applying a specified affine matrix.
	\param dst output (transformed) ellipse curve.
	\param matrix transformation matrix to apply.
	\param src input ellipse curve.
	\note Applying the specified matrix, the input ellipse must remain an ellipse.
	First of all we use conic equation in the form ax^2 + 2bxy + dy^2 + 2cx + 2ey + 1 = 0, and we
	impose the passage through 5 points that are granted to be on transformed ellipse (chosen in a way
	that ensures system solving stability).  So we solve the system and get a, b, d, c, e parameters.\n
	Now we have to port conic form into used one (semi axes lengths, center, offset rotation, start angle,
	end angle and cw/ccw direction). To do this, we calculate Grahm matrix associated to conic (ellipse is a
	quadratic form), then we diagonalize it using spectral decomposition.\n
	Eigenvalues give us new semi axes lengths, and eigenvectors give us offset rotation. Start angle and end
	angle are calculated from transformed endpoints and new semi axes lengths.\n
	Cw/ccw direction is calculated imposing that spanned angle (from start angle to end angle) will remain the
	same in the sense of large/small arc.\n
	\note REFERENCE MISMATCH: "Any *ARC_TO segments are transformed, but the endpoint parametrization of the resulting arc segments
	are implementation-dependent". Applying negative scale to elliptical arcs, AmanithVG, in order to preserve the original shapes,
	inverts the arcs orientation (cw --> ccw, ccw --> cw).
*/
void amEllipsefTransform(AMEllipsef *dst,
						 const AMMatrix33f *matrix,
						 const AMEllipsef *src) {

	AMVect2f p0, p1, p2, p3, p4, oldTransfCenter, startPoint, endPoint;
	AMMatrix55f M, invM;
	AMVect5f rhs, solution;
	AMbool fullRank, newCCW, srcLargeArc;
	AMfloat angMin, angMax, step1, step2, ang1, ang2, detM, ofsRot;

	AM_ASSERT(dst);
	AM_ASSERT(matrix);
	AM_ASSERT(src);

	AM_MATRIX33_VECT2_MUL(&oldTransfCenter, matrix, &src->center)

	srcLargeArc = amEllipsefLargeArc(src);
	angMin = src->startAngle;
	angMax = src->endAngle;
	
	amEllipsefEvalByAngle(&startPoint, src, src->startAngle);
	amEllipsefEvalByAngle(&endPoint, src, src->endAngle);

	// calculate end points
	AM_MATRIX33_VECT2_MUL(&p0, matrix, &startPoint)
	AM_MATRIX33_VECT2_MUL(&p1, matrix, &endPoint)

	// calculate other 3 points
	if (src->startAngle > src->endAngle) {
		AMfloat swap = angMin;
		angMin = angMax;
		angMax = swap;
	}
	
	if (srcLargeArc) {
		step1 = (angMax - angMin) / 3.0f;
		step2 = (AM_2PI - angMax + angMin) / 2.0f;
		amEllipsefEvalByAngle(&p2, src, angMin + step1);
		AM_MATRIX33_VECT2_SELF_MUL(matrix, &p2, AMVect2f)
		amEllipsefEvalByAngle(&p3, src, angMin + 2.0f * step1);
		AM_MATRIX33_VECT2_SELF_MUL(matrix, &p3, AMVect2f)
		amEllipsefEvalByAngle(&p4, src, angMax + step2);
		AM_MATRIX33_VECT2_SELF_MUL(matrix, &p4, AMVect2f)
	}
	else {
		step1 = (angMax - angMin) / 2.0f;
		step2 = (AM_2PI - angMax + angMin) / 3.0f;
		amEllipsefEvalByAngle(&p2, src, angMin + step1);
		AM_MATRIX33_VECT2_SELF_MUL(matrix, &p2, AMVect2f)
		amEllipsefEvalByAngle(&p3, src, angMin + step2);
		AM_MATRIX33_VECT2_SELF_MUL(matrix, &p3, AMVect2f)
		amEllipsefEvalByAngle(&p4, src, angMax + 2 * step2);
		AM_MATRIX33_VECT2_SELF_MUL(matrix, &p4, AMVect2f)
	}

	AM_VECT2_SELF_SUB(&p0, &oldTransfCenter)
	AM_VECT2_SELF_SUB(&p1, &oldTransfCenter)
	AM_VECT2_SELF_SUB(&p2, &oldTransfCenter)
	AM_VECT2_SELF_SUB(&p3, &oldTransfCenter)
	AM_VECT2_SELF_SUB(&p4, &oldTransfCenter)

	// build system matrix
	M.a[0][0] = p0.x * p0.x;
	M.a[0][1] = 2.0f * (p0.x * p0.y);
	M.a[0][2] = p0.y * p0.y;
	M.a[0][3] = 2.0f * p0.x;
	M.a[0][4] = 2.0f * p0.y;
	M.a[1][0] = p1.x * p1.x;
	M.a[1][1] = 2.0f * (p1.x * p1.y);
	M.a[1][2] = p1.y * p1.y;
	M.a[1][3] = 2.0f * p1.x;
	M.a[1][4] = 2.0f * p1.y;
	M.a[2][0] = p2.x * p2.x;
	M.a[2][1] = 2.0f * (p2.x * p2.y);
	M.a[2][2] = p2.y * p2.y;
	M.a[2][3] = 2.0f * p2.x;
	M.a[2][4] = 2.0f * p2.y;
	M.a[3][0] = p3.x * p3.x;
	M.a[3][1] = 2.0f * (p3.x * p3.y);
	M.a[3][2] = p3.y * p3.y;
	M.a[3][3] = 2.0f * p3.x;
	M.a[3][4] = 2.0f * p3.y;
	M.a[4][0] = p4.x * p4.x;
	M.a[4][1] = 2.0f * (p4.x * p4.y);
	M.a[4][2] = p4.y * p4.y;
	M.a[4][3] = 2.0f * p4.x;
	M.a[4][4] = 2.0f * p4.y;
	// build rhs vector
	rhs.x = rhs.y = rhs.z = rhs.v = rhs.w = -1.0f;
	// invert system matrix
	fullRank = amMatrix55fInvert(&invM, &detM, &M);
	if (fullRank) {

		AMMatrix33f grahm;
		AMVect2f eValue1, eValue2, eValue3, q0, q1;
		AMVect3f eVector1, eVector2, eVector3, xAxis;
		AMfloat newXSemiLen, newYSemiLen, cosOfsRot, sinOfsRot, xAxisLen, yAxisLen, detRot;
		AMMatrix33f rot, scl;

		// solve the system and get conic coefficients
		solution.x = invM.a[0][0] * rhs.x + invM.a[0][1] * rhs.y + invM.a[0][2] * rhs.z + invM.a[0][3] * rhs.v + invM.a[0][4] * rhs.w;
		solution.y = invM.a[1][0] * rhs.x + invM.a[1][1] * rhs.y + invM.a[1][2] * rhs.z + invM.a[1][3] * rhs.v + invM.a[1][4] * rhs.w;
		solution.z = invM.a[2][0] * rhs.x + invM.a[2][1] * rhs.y + invM.a[2][2] * rhs.z + invM.a[2][3] * rhs.v + invM.a[2][4] * rhs.w;
		solution.v = invM.a[3][0] * rhs.x + invM.a[3][1] * rhs.y + invM.a[3][2] * rhs.z + invM.a[3][3] * rhs.v + invM.a[3][4] * rhs.w;
		solution.w = invM.a[4][0] * rhs.x + invM.a[4][1] * rhs.y + invM.a[4][2] * rhs.z + invM.a[4][3] * rhs.v + invM.a[4][4] * rhs.w;

		// build associated Grahm matrix
		grahm.a[0][0] = solution.x; grahm.a[0][1] = solution.y; grahm.a[0][2] = solution.v;
		grahm.a[1][0] = solution.y; grahm.a[1][1] = solution.z; grahm.a[1][2] = solution.w;
		grahm.a[2][0] = solution.v; grahm.a[2][1] = solution.w; grahm.a[2][2] = 1.0f;

		// diagonalize Grahm matrix using spectral decomposition
		amMatrix33fEigenSolver(&eValue1, &eValue2, &eValue3, &eVector1, &eVector2, &eVector3, &grahm, AM_FALSE);
		// eigenvalues are real, so y-component can be discarded
		xAxis = eVector1;
		xAxisLen = eValue1.x;
		yAxisLen = eValue2.x;
		if (amAbsf(eVector1.z) > 0.5f) {
			xAxis = eVector2;
			xAxisLen = eValue2.x;
			yAxisLen = eValue3.x;
		}
		else
		if (amAbsf(eVector2.z) > 0.5f)
			yAxisLen = eValue3.x;

		// calculate new semi axes lengths
		newXSemiLen = amSqrtf(1.0f / amAbsf(xAxisLen));
		newYSemiLen = amSqrtf(1.0f / amAbsf(yAxisLen));
		// calculate offset rotation (relative to x-axis)
		ofsRot = amAtan2f(xAxis.y, xAxis.x);
		// port transformed end points to new ellipse coordinate system (so the new ellipse is centered
		// at (0, 0) and axes aligned
		cosOfsRot = amCosf(ofsRot);
		sinOfsRot = amSinf(ofsRot);

		AM_VECT2_SET(&q0, cosOfsRot * p0.x + sinOfsRot * p0.y, -sinOfsRot * p0.x + cosOfsRot * p0.y)
		AM_VECT2_SET(&q1, cosOfsRot * p1.x + sinOfsRot * p1.y, -sinOfsRot * p1.x + cosOfsRot * p1.y)

		// calculate new start/end angles
		ang1 = amAtan2f(q0.y / newYSemiLen, q0.x / newXSemiLen);
		ang2 = amAtan2f(q1.y / newYSemiLen, q1.x / newXSemiLen);

		// calculate direction
		amMatrix33fPolarDecompose(&rot, &detRot, &scl, matrix);
		newCCW = (detRot > 0.0f) ? src->ccw : !src->ccw;

		// set the brand new ellipse
		amEllipsefSetByAxes(dst, &oldTransfCenter, newXSemiLen, newYSemiLen, ofsRot, ang1, ang2, newCCW);
	}
	else
		// in this case we can only ensure first and last point positions
		amEllipsefSetByPoints(dst, &p0, &p1, src->xSemiAxisLength, src->ySemiAxisLength, src->offsetRotation, amEllipsefLargeArc(src), src->ccw);
}
#if defined (RIM_VG_SRC)
#pragma pop
#endif


