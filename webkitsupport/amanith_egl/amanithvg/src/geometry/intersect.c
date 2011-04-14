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
	\file intersect.c
	\brief Ray and circle intersection routines, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "intersect.h"

/*!
	\brief Intersection between 2 rays.
	\param localParameters output intersection parameters, relative to both rays.
	\param flags output intersection flags; bitfield made of one or more the following values: NO_SOLUTIONS, MULTIPLE_SOLUTIONS, SINGLE_SOLUTION, INFINITE_SOLUTIONS, COINCIDENT_SHAPES, INCLUDED_SHAPE TANGENT_SHAPES.
	\param ray1 first input ray.
	\param ray2 second input ray.
	\return AM_TRUE if intersection exists, else AM_FALSE.
	\note intersection point, when it exists, is given by ray1.origin + localParameters[0] * ray1.direction or (equivalently) by ray2.origin + localParameters[1] * ray2.direction.
*/
AMbool amRayRayIntersect(AMfloat localParameters[2],
						 AMuint32 *flags,
						 const AMRay2f *ray1,
						 const AMRay2f *ray2) {

	AMVect2f diffOrg;
	AMfloat det;

	AM_ASSERT(ray1);
	AM_ASSERT(ray2);
	AM_ASSERT(flags);
	AM_ASSERT(localParameters);

	det = ray2->direction.x * ray1->direction.y - ray2->direction.y * ray1->direction.x;
	AM_VECT2_SUB(&diffOrg, &ray2->origin, &ray1->origin)

	if (amAbsf(det) > AM_EPSILON_FLOAT) {
		// lines intersect in a single point. Return both s and t values for use by calling functions
		AMfloat invDet = 1.0f / det;

		*flags = SINGLE_SOLUTION;
		localParameters[0] = (ray2->direction.x * diffOrg.y - ray2->direction.y * diffOrg.x) * invDet;
		localParameters[1] = (ray1->direction.x * diffOrg.y - ray1->direction.y * diffOrg.x) * invDet;
		return AM_TRUE;
	}
	else {
		// lines are parallel
		det = ray1->direction.x * diffOrg.y - ray1->direction.y * diffOrg.x;
		if (amAbsf(det) > AM_EPSILON_FLOAT) {
			// lines are disjoint
			*flags = NO_SOLUTIONS;
			return AM_FALSE;
		}
		else {
			// lines are the same (collinear)
			*flags = INFINITE_SOLUTIONS | COINCIDENT_SHAPES;
			return AM_TRUE;
		}
	}
}

/*!
	\brief Intersection between a ray and a circle.
	\param localParameters output intersections parameters, relative to the ray.
	\param flags output intersections flags; bitfield made of one or more the following values: NO_SOLUTIONS, MULTIPLE_SOLUTIONS, SINGLE_SOLUTION, INFINITE_SOLUTIONS, COINCIDENT_SHAPES, INCLUDED_SHAPE TANGENT_SHAPES.
	\param ray input ray.
	\param circle input circle.
	\return AM_TRUE if intersections exists, else AM_FALSE.
	\pre circle ray must be positive.
	\note intersection points, when they exists, are given by ray.origin + localParameters[0] * ray.direction and by ray.origin + localParameters[1] * ray.direction.
*/
AMbool amRayCircleIntersect(AMfloat localParameters[2],
							AMuint32 *flags,
							const AMRay2f *ray,
							const AMCircle2f *circle) {

	AMfloat locParams[2];
	AMfloat a, b, c, swap;
	AMVect2f diffOrg;
	AMint32 rootsCount;

	AM_ASSERT(ray);
	AM_ASSERT(circle);
	AM_ASSERT(circle->radius > 0.0f);
	AM_ASSERT(flags);
	AM_ASSERT(localParameters);

	AM_VECT2_SUB(&diffOrg, &ray->origin, &circle->center)
	a = AM_VECT2_SQR_LENGTH(&ray->direction);
	b = AM_VECT2_DOT(&diffOrg, &ray->direction);
	c = AM_VECT2_SQR_LENGTH(&diffOrg) - circle->radius * circle->radius;
	rootsCount = amQuadraticFormulaf(&locParams[0], &locParams[1], a, 2.0f * b, c);

	if (rootsCount == 0) {
		*flags = NO_SOLUTIONS;
		return AM_FALSE;
	}
	else
	if (rootsCount == 1) {
		if (locParams[0] < -AM_EPSILON_FLOAT) {
			*flags = NO_SOLUTIONS;
			return AM_FALSE;
		}
		else {
			*flags = SINGLE_SOLUTION | TANGENT_SHAPES;
			localParameters[0] = locParams[0];
			return AM_TRUE;
		}
	}
	// 2 solutions found, lets first sort them (ascending order)
	if (locParams[0] > locParams[1]) {
		swap = locParams[0];
		locParams[0] = locParams[1];
		locParams[1] = swap;
	}
	if (locParams[0] >= -AM_EPSILON_FLOAT) {
		localParameters[0] = locParams[0];
		localParameters[1] = locParams[1];
		*flags = MULTIPLE_SOLUTIONS;
		return AM_TRUE;
	}
	else
	if (locParams[1] >= -AM_EPSILON_FLOAT) {
		localParameters[0] = locParams[1];
		*flags = SINGLE_SOLUTION;
		return AM_TRUE;
	}
	else {
		*flags = NO_SOLUTIONS;
		return AM_FALSE;
	}
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

