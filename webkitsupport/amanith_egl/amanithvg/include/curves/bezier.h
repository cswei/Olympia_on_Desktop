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

#ifndef _BEZIER_H
#define _BEZIER_H

/*!
	\file bezier.h
	\brief Quadratic and cubic Bezier curves, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "aabox.h"
#include "dynarray.h"


/*!
	\brief Quadratic Bezier.
*/
typedef struct _AMBezier2f {
	//! Control point 0
	AMVect2f p0;
	//! Control point 1
	AMVect2f p1;
	//! Control point 2
	AMVect2f p2;
	//! First derivative coefficient (<b>d0</b> * t + d1)
	AMVect2f d0;
	//! First derivative coefficient (d0 * t + <b>d1</b>)
	AMVect2f d1;
} AMBezier2f;

/*!
	\brief Cubic Bezier.
*/
typedef struct _AMBezier3f {
	//! Control point 0
	AMVect2f p0;
	//! Control point 1
	AMVect2f p1;
	//! Control point 2
	AMVect2f p2;
	//! Control point 3
	AMVect2f p3;
	//! First derivative coefficient (<b>d0</b> * t * t + d1 * t + d2)
	AMVect2f d0;
	//! First derivative coefficient (d0 * t * t + <b>d1</b> * t + d2)
	AMVect2f d1;
	//! First derivative coefficient (d0 * t * t + d1 * t + <b>d2</b>)
	AMVect2f d2;
} AMBezier3f;

// Quadratic Bezier constructor given 3 control points. It calculates first derivative coefficients too.
void amBezier2fSet(AMBezier2f *dst,
				   const AMVect2f *p0,
				   const AMVect2f *p1,
				   const AMVect2f *p2);
// Quadratic Bezier evaluation. It evaluates a quadratic Bezier position given the local parameter.
void amBezier2fEval(AMVect2f *dst,
					const AMBezier2f *curve,
					const AMfloat u);
// Quadratic Bezier tangent evaluation. It evaluates a quadratic Bezier tangent vector given the local parameter.
void amBezier2fTan(AMVect2f *dst,
				   const AMBezier2f *curve,
				   const AMfloat u);
// Quadratic Bezier length evaluation. It evaluates a quadratic Bezier length given a local parameter range.
AMfloat amBezier2fLen(const AMBezier2f *curve,
					  const AMfloat u0,
					  const AMfloat u1);
// Given len = amBezier2fLen(curve, 0, u), this function calculates u.
AMbool amBezier2fParam(AMfloat *result,
					   const AMBezier2f *curve,
					   const AMfloat len);
// Given a quadratic Bezier curve, this function flattens it.
void amBezier2fFlatten(AMVect2fDynArray *points,
					   const AMBezier2f *curve,
					   const AMFlattenParams *params,
					   const AMbool includeLastPoint);

// Cubic Bezier constructor given 4 control points. It calculates first derivative coefficients too.
void amBezier3fSet(AMBezier3f *dst,
				   const AMVect2f *p0,
				   const AMVect2f *p1,
				   const AMVect2f *p2,
				   const AMVect2f *p3);
// Cubic Bezier evaluation. It evaluates a cubic Bezier position given the local parameter.
void amBezier3fEval(AMVect2f *dst,
					const AMBezier3f *curve,
					const AMfloat u);
// Cubic Bezier tangent evaluation. It evaluates a cubic Bezier tangent vector given the local parameter.
void amBezier3fTan(AMVect2f *dst,
				   const AMBezier3f *curve,
				   const AMfloat u);
// Cubic Bezier length evaluation. It evaluates a cubic Bezier length given a local parameter range.
AMfloat amBezier3fLen(const AMBezier3f *curve,
					  const AMfloat u0,
					  const AMfloat u1);
// Given len = amBezier3fLen(curve, 0, u), this function calculates u.
AMbool amBezier3fParam(AMfloat *result,
					   const AMBezier3f *curve,
					   const AMfloat len);
// Given a Cubic Bezier curve, this function flattens it.
void amBezier3fFlatten(AMVect2fDynArray *points,
					   const AMBezier3f *curve,
					   const AMFlattenParams *params,
					   const AMbool includeLastPoint);

#endif
