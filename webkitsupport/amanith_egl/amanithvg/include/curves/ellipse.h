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

#ifndef _ELLIPSE_H
#define _ELLIPSE_H

/*!
	\file ellipse.h
	\brief Ellipse curves, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "aabox.h"
#include "dynarray.h"

/*!
	\brief Ellipse curve.
*/
typedef struct _AMEllipsef {
	//! Center of the ellipse.
	AMVect2f center;
	//! Semi axis X (length).
	AMfloat xSemiAxisLength;
	//! Semi axis Y (length).
	AMfloat ySemiAxisLength;
	//! Rotation offset angle, it permits non axes-aligned ellipses.
	AMfloat offsetRotation;
	//! Pre-calculated cosine of offsetRotation angle.
	AMfloat cosOfsRot;
	//! Pre-calculated sine of offsetRotation angle.
	AMfloat sinOfsRot;
	//! Start angle corresponding to the domain lower bound.
	AMfloat startAngle;
	//! End angle corresponding to the domain upper bound.
	AMfloat endAngle;
	//! Direction ccw/cw.
	AMbool ccw;
} AMEllipsef;

// Given an ellipse curve, it checks if the specified angle falls inside the domain.
AMbool amEllipsefAngleIncluded(const AMEllipsef *curve,
							   const AMfloat angle);
// Ellipse constructor given center, axes, rotation, angle domain and direction (cw/ccw).
void amEllipsefSetByAxes(AMEllipsef *dst,
						 const AMVect2f *center,
						 const AMfloat xSemiAxisLength,
						 const AMfloat ySemiAxisLength,
						 const AMfloat offsetRotation,
						 const AMfloat startAngle,
						 const AMfloat endAngle,
						 const AMbool ccw);
// Ellipse constructor given 2 points, axes, rotation, small/large arc flag and direction (cw/ccw).
AMbool amEllipsefSetByPoints(AMEllipsef *dst,
							 const AMVect2f *q0,
							 const AMVect2f *q1,
							 const AMfloat xSemiAxisLength,
							 const AMfloat ySemiAxisLength,
							 const AMfloat offsetRotation,
							 const AMbool largeArc,
							 const AMbool ccw);
// Ellipse evaluation. It evaluates an ellipse position given an angle that falls inside the domain.
void amEllipsefEvalByAngle(AMVect2f *dst,
						   const AMEllipsef *curve,
						   const AMfloat angle);
// Ellipse evaluation. It evaluates an ellipse position given the local parameter.
void amEllipsefEval(AMVect2f *dst,
					const AMEllipsef *curve,
					const AMfloat u);
// Ellipse tangent evaluation. It evaluates an ellipse tangent vector given the local parameter.
void amEllipsefTan(AMVect2f *dst,
				   const AMEllipsef *curve,
				   const AMfloat u);
// Ellipse length evaluation. It evaluates an ellipse length given a local parameter range.
AMfloat amEllipsefLen(const AMEllipsef *curve,
					  const AMfloat u0,
					  const AMfloat u1);
// Given len = amEllipsefLen(curve, 0, u), this function calculates u.
AMbool amEllipsefParam(AMfloat *result,
					   const AMEllipsef *curve,
					   const AMfloat len);
// Given an ellipse curve, this function flattens it.
void amEllipsefFlatten(AMVect2fDynArray *points,
					   const AMEllipsef *curve,
					   const AMFlattenParams *params,
					   const AMbool includeLastPoint);
// Transform an ellipse applying a specified affine matrix.
void amEllipsefTransform(AMEllipsef *dst,
						const AMMatrix33f *matrix,
						const AMEllipsef *src);

#endif
