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

#ifndef _VECTOR_H
#define _VECTOR_H

/*!
	\file vector.h
	\brief Vector related definitions and routines, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "mathematics.h"

/*!
	\brief 2D vector, integers components.
*/
typedef struct _AMVect2i {
	//! x component.
	AMint32 x;
	//! y component.
	AMint32 y;
} AMVect2i;

/*!
	\brief 2D vector, fixed point components.
*/
typedef struct _AMVect2x {
	//! x component.
	AMfixed x;
	//! y component.
	AMfixed y;
} AMVect2x;

/*!
	\brief 2D vector, float components.
*/
typedef struct _AMVect2f {
	//! x component.
	AMfloat x;
	//! y component.
	AMfloat y;
} AMVect2f;

/*!
	\brief 3D vector, float components.
*/
typedef struct _AMVect3f {
	//! x component.
	AMfloat x;
	//! y component.
	AMfloat y;
	//! z component.
	AMfloat z;
} AMVect3f;

/*!
	\brief 5D vector, float components.
*/
typedef struct _AMVect5f {
	//! x component.
	AMfloat x;
	//! y component.
	AMfloat y;
	//! z component.
	AMfloat z;
	//! v component.
	AMfloat v;
	//! w component.
	AMfloat w;
} AMVect5f;


//! 2D vector addition operator.
#define AM_VECT2_ADD(_dst, _v0, _v1) \
	(_dst)->x = (_v0)->x + (_v1)->x; \
	(_dst)->y = (_v0)->y + (_v1)->y;

//! 2D vector self addition operator.
#define AM_VECT2_SELF_ADD(_dst, _v) \
	(_dst)->x += (_v)->x; \
	(_dst)->y += (_v)->y;

//! 2D vector subtraction operator.
#define AM_VECT2_SUB(_dst, _v0, _v1) \
	(_dst)->x = (_v0)->x - (_v1)->x; \
	(_dst)->y = (_v0)->y - (_v1)->y;

//! 2D vector self subtraction operator.
#define AM_VECT2_SELF_SUB(_dst, _v) \
	(_dst)->x -= (_v)->x; \
	(_dst)->y -= (_v)->y;

//! 2D vector negation operator.
#define AM_VECT2_NEG(_dst, _v) \
	(_dst)->x = -(_v)->x; \
	(_dst)->y = -(_v)->y;

//! 2D vector self negation operator.
#define AM_VECT2_SELF_NEG(_dst) \
	(_dst)->x = -(_dst)->x; \
	(_dst)->y = -(_dst)->y;

//! 2D vector dot operator.
#define AM_VECT2_DOT(_v0, _v1) ((_v0)->x * (_v1)->x + (_v0)->y * (_v1)->y)

//! 2D vector cross operator.
#define AM_VECT2_CROSS(_v0, _v1) ((_v0)->x * (_v1)->y - (_v1)->x * (_v0)->y)

//! 2D vector constructor, given components.
#define AM_VECT2_SET(_dst, _x, _y) \
	(_dst)->x = (_x); \
	(_dst)->y = (_y);

//! 2D vector scalar multiplication.
#define AM_VECT2_MUL(_dst, _factor, _v) \
	(_dst)->x = (_v)->x * (_factor); \
	(_dst)->y = (_v)->y * (_factor);

//! 2D vector scalar self multiplication.
#define AM_VECT2_SELF_MUL(_dst, _factor) \
	(_dst)->x *= (_factor); \
	(_dst)->y *= (_factor);

//! 2D vector multiply-add operator.
#define AM_VECT2_MADD(_dst, _v0, _factor, _v1) \
	(_dst)->x = (_v0)->x + (_factor) * (_v1)->x; \
	(_dst)->y = (_v0)->y + (_factor) * (_v1)->y;

//! 2D vector self multiply-add operator.
#define AM_VECT2_SELF_MADD(_dst, _factor, _v) \
	(_dst)->x += (_factor) * (_v)->x; \
	(_dst)->y += (_factor) * (_v)->y;

//! 2D vector reflection (around a center point) operator.
#define AM_VECT2_REFLECT(_dst, _v, _center) \
	(_dst)->x = 2 * (_center)->x - (_v)->x; \
	(_dst)->y = 2 * (_center)->y - (_v)->y;

//! It calculates squared length of a 2D vector.
#define AM_VECT2_SQR_LENGTH(_v) ((_v)->x * (_v)->x + (_v)->y * (_v)->y)

//! It calculates the squared distance between two 2D points.
#define AM_VECT2_SQR_DISTANCE(_v0, _v1) (((_v0)->x - (_v1)->x) * ((_v0)->x - (_v1)->x) + ((_v0)->y - (_v1)->y) * ((_v0)->y - (_v1)->y))

//! 3D vector constructor, given components.
#define AM_VECT3_SET(_dst, _x, _y, _z) \
	(_dst)->x = (_x); \
	(_dst)->y = (_y); \
	(_dst)->z = (_z);

//! 3D vector cross operator.
#define AM_VECT3_CROSS(_dst, _v0, _v1) \
	(_dst)->x = (_v0)->y * (_v1)->z - (_v0)->z * (_v1)->y; \
	(_dst)->y = (_v0)->z * (_v1)->x - (_v0)->x * (_v1)->z; \
	(_dst)->z = (_v0)->x * (_v1)->y - (_v0)->y * (_v1)->x;

/*!
	\brief It calculates the normalized direction from p0 to p1.
	\param normDir output normalized direction.
	\param p0 input start point.
	\param p1 input end point.
	\return the distance between p0 and p1.
*/
AM_INLINE AMfloat amVect2fNormDirection(AMVect2f *normDir,
										const AMVect2f *p0,
										const AMVect2f *p1) {

	AMfloat l;

	AM_VECT2_SUB(normDir, p0, p1);
	l = amSqrtf(AM_VECT2_SQR_LENGTH(normDir));

	if (l <= AM_EPSILON_FLOAT) {
		normDir->x = 0.0f;
		normDir->y = 0.0f;
		return 0.0f;
	}
	else {
		normDir->x /= l;
		normDir->y /= l;
		return l;
	}
}

/*!
	\brief It normalizes the given 2D vector.
	\param dst output normalized vector.
	\param v input vector.
	\return the length of input vector.
*/
AM_INLINE AMfloat amVect2fNormalize(AMVect2f *dst,
									const AMVect2f *v) {

	AMfloat l = amSqrtf(AM_VECT2_SQR_LENGTH(v));

	if (l <= AM_EPSILON_FLOAT) {
		dst->x = 0.0f;
		dst->y = 0.0f;
		return 0.0f;
	}
	else {
		dst->x = v->x / l;
		dst->y = v->y / l;
		return l;
	}
}

/*!
	\brief It normalizes the given 2D vector.
	\param v input/output vector.
	\return the length of input vector.
*/
AM_INLINE AMfloat amVect2fSelfNormalize(AMVect2f *v) {

	AMfloat l = amSqrtf(AM_VECT2_SQR_LENGTH(v));

	if (l <= AM_EPSILON_FLOAT) {
		v->x = 0.0f;
		v->y = 0.0f;
		return 0.0f;
	}
	else {
		AMfloat invl = 1.0f / l;
		v->x *= invl;
		v->y *= invl;
		return l;
	}
}

#endif
