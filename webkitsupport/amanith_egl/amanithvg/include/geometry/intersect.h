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

#ifndef _INTERSECT_H
#define _INTERSECT_H

/*!
	\file intersect.h
	\brief Ray and circle intersection routines, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "vector.h"

#define NO_SOLUTIONS		0
#define MULTIPLE_SOLUTIONS	1
#define SINGLE_SOLUTION		2
#define INFINITE_SOLUTIONS	4
#define COINCIDENT_SHAPES	8
#define INCLUDED_SHAPE		16
#define TANGENT_SHAPES		32

/*!
	\brief 2D ray.
*/
typedef struct _AMRay2f {
	//! Ray origin.
	AMVect2f origin;
	//! Ray direction.
	AMVect2f direction;
} AMRay2f;

/*!
	\brief 2D circle.
*/
typedef struct _AMCircle2f {
	//! Circle center.
	AMVect2f center;
	//! Circle radius.
	AMfloat radius;
} AMCircle2f;

// ray - ray intersection
AMbool amRayRayIntersect(AMfloat localParameters[2],
						 AMuint32 *flags,
						 const AMRay2f *ray1,
						 const AMRay2f *ray2);
// ray - circle intersection
AMbool amRayCircleIntersect(AMfloat localParameters[2],
							AMuint32 *flags,
							const AMRay2f *ray,
							const AMCircle2f *circle);

#endif
