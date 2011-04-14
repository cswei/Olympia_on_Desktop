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

#ifndef _MATRIX_H_
#define _MATRIX_H_

/*!
	\file matrix.h
	\brief Matrix related definitions and routines, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "vector.h"

/*!
	\brief A 2x2 matrix, float elements.
*/
typedef struct _AMMatrix22f {
	//! Matrix elements, in the a[row][column] form.
	AMfloat a[2][2];
} AMMatrix22f;

/*!
	\brief A 3x3 matrix, float elements.
*/
typedef struct _AMMatrix33f {
	//! Matrix elements, in the a[row][column] form.
	AMfloat a[3][3];
} AMMatrix33f;

/*!
	\brief A 3x3 matrix, fixed point elements.
*/
typedef struct _AMMatrix33x {
	//! Matrix elements, in the a[row][column] form.
	AMfixed a[3][3];
} AMMatrix33x;

/*!
	\brief A 4x4 matrix, float elements.
*/
typedef struct _AMMatrix44f {
	//! Matrix elements, in the a[row][column] form.
	AMfloat a[4][4];
} AMMatrix44f;

/*!
	\brief A 5x5 matrix, float elements.
*/
typedef struct _AMMatrix55f {
	//! Matrix elements, in the a[row][column] form.
	AMfloat a[5][5];
} AMMatrix55f;

//! It self transposes a 2x2 matrix. \a _dataType must be the same type of matrix elements.
#define AM_MATRIX22_SELF_TRANSPOSE(_m, _dataType) { \
	_dataType _tmp; \
	_tmp = (_m)->a[1][0]; \
	(_m)->a[1][0] = (_m)->a[0][1]; \
	(_m)->a[0][1] = _tmp; \
}

//! 2x2 matrix - 2D vector multiplication. \a _vectType must be the same type of the vector.
#define AM_MATRIX22_VECT2_MUL(_dst, _m, _v, _vectType) { \
	_vectType _tmp; \
	_tmp.x = (_m)->a[0][0] * (_v)->x + (_m)->a[0][1] * (_v)->y; \
	_tmp.y = (_m)->a[1][0] * (_v)->x + (_m)->a[1][1] * (_v)->y; \
	(*(_dst)) = _tmp; \
}

//! 3x3 matrix constructor, given elements.
#define AM_MATRIX33_SET(_dst, _a00, _a01, _a02, _a10, _a11, _a12, _a20, _a21, _a22) \
	(_dst)->a[0][0] = (_a00); \
	(_dst)->a[0][1] = (_a01); \
	(_dst)->a[0][2] = (_a02); \
	(_dst)->a[1][0] = (_a10); \
	(_dst)->a[1][1] = (_a11); \
	(_dst)->a[1][2] = (_a12); \
	(_dst)->a[2][0] = (_a20); \
	(_dst)->a[2][1] = (_a21); \
	(_dst)->a[2][2] = (_a22);

//! 3x3 matrix copy operator.
#define AM_MATRIX33_COPY(_dst, _src) \
	(_dst)->a[0][0] = (_src)->a[0][0]; \
	(_dst)->a[0][1] = (_src)->a[0][1]; \
	(_dst)->a[0][2] = (_src)->a[0][2]; \
	(_dst)->a[1][0] = (_src)->a[1][0]; \
	(_dst)->a[1][1] = (_src)->a[1][1]; \
	(_dst)->a[1][2] = (_src)->a[1][2]; \
	(_dst)->a[2][0] = (_src)->a[2][0]; \
	(_dst)->a[2][1] = (_src)->a[2][1]; \
	(_dst)->a[2][2] = (_src)->a[2][2];

//! 3x3 matrix negation operator.
#define AM_MATRIX33_NEG(_dst, _m) \
	(_dst)->a[0][0] = -(_m)->a[0][0]; \
	(_dst)->a[0][1] = -(_m)->a[0][1]; \
	(_dst)->a[0][2] = -(_m)->a[0][2]; \
	(_dst)->a[1][0] = -(_m)->a[1][0]; \
	(_dst)->a[1][1] = -(_m)->a[1][1]; \
	(_dst)->a[1][2] = -(_m)->a[1][2]; \
	(_dst)->a[2][0] = -(_m)->a[2][0]; \
	(_dst)->a[2][1] = -(_m)->a[2][1]; \
	(_dst)->a[2][2] = -(_m)->a[2][2];

//! 3x3 matrix self negation operator.
#define AM_MATRIX33_SELF_NEG(_m) \
	(_m)->a[0][0] = -(_m)->a[0][0]; \
	(_m)->a[0][1] = -(_m)->a[0][1]; \
	(_m)->a[0][2] = -(_m)->a[0][2]; \
	(_m)->a[1][0] = -(_m)->a[1][0]; \
	(_m)->a[1][1] = -(_m)->a[1][1]; \
	(_m)->a[1][2] = -(_m)->a[1][2]; \
	(_m)->a[2][0] = -(_m)->a[2][0]; \
	(_m)->a[2][1] = -(_m)->a[2][1]; \
	(_m)->a[2][2] = -(_m)->a[2][2];

//! 3x3 matrix addition operator.
#define AM_MATRIX33_ADD(_dst, _m0, _m1) \
	(_dst)->a[0][0] = (_m0)->a[0][0] + (_m1)->a[0][0]; \
	(_dst)->a[0][1] = (_m0)->a[0][1] + (_m1)->a[0][1]; \
	(_dst)->a[0][2] = (_m0)->a[0][2] + (_m1)->a[0][2]; \
	(_dst)->a[1][0] = (_m0)->a[1][0] + (_m1)->a[1][0]; \
	(_dst)->a[1][1] = (_m0)->a[1][1] + (_m1)->a[1][1]; \
	(_dst)->a[1][2] = (_m0)->a[1][2] + (_m1)->a[1][2]; \
	(_dst)->a[2][0] = (_m0)->a[2][0] + (_m1)->a[2][0]; \
	(_dst)->a[2][1] = (_m0)->a[2][1] + (_m1)->a[2][1]; \
	(_dst)->a[2][2] = (_m0)->a[2][2] + (_m1)->a[2][2];

//! 3x3 matrix self addition operator.
#define AM_MATRIX33_SELF_ADD(_dst, _m) \
	(_dst)->a[0][0] += (_m)->a[0][0]; \
	(_dst)->a[0][1] += (_m)->a[0][1]; \
	(_dst)->a[0][2] += (_m)->a[0][2]; \
	(_dst)->a[1][0] += (_m)->a[1][0]; \
	(_dst)->a[1][1] += (_m)->a[1][1]; \
	(_dst)->a[1][2] += (_m)->a[1][2]; \
	(_dst)->a[2][0] += (_m)->a[2][0]; \
	(_dst)->a[2][1] += (_m)->a[2][1]; \
	(_dst)->a[2][2] += (_m)->a[2][2];

//! 3x3 matrix subtraction operator.
#define AM_MATRIX33_SUB(_dst, _m0, _m1) \
	(_dst)->a[0][0] = (_m0)->a[0][0] - (_m1)->a[0][0]; \
	(_dst)->a[0][1] = (_m0)->a[0][1] - (_m1)->a[0][1]; \
	(_dst)->a[0][2] = (_m0)->a[0][2] - (_m1)->a[0][2]; \
	(_dst)->a[1][0] = (_m0)->a[1][0] - (_m1)->a[1][0]; \
	(_dst)->a[1][1] = (_m0)->a[1][1] - (_m1)->a[1][1]; \
	(_dst)->a[1][2] = (_m0)->a[1][2] - (_m1)->a[1][2]; \
	(_dst)->a[2][0] = (_m0)->a[2][0] - (_m1)->a[2][0]; \
	(_dst)->a[2][1] = (_m0)->a[2][1] - (_m1)->a[2][1]; \
	(_dst)->a[2][2] = (_m0)->a[2][2] - (_m1)->a[2][2];

//! 3x3 matrix self subtraction operator.
#define AM_MATRIX33_SELF_SUB(_dst, _m) \
	(_dst)->a[0][0] -= (_m)->a[0][0]; \
	(_dst)->a[0][1] -= (_m)->a[0][1]; \
	(_dst)->a[0][2] -= (_m)->a[0][2]; \
	(_dst)->a[1][0] -= (_m)->a[1][0]; \
	(_dst)->a[1][1] -= (_m)->a[1][1]; \
	(_dst)->a[1][2] -= (_m)->a[1][2]; \
	(_dst)->a[2][0] -= (_m)->a[2][0]; \
	(_dst)->a[2][1] -= (_m)->a[2][1]; \
	(_dst)->a[2][2] -= (_m)->a[2][2];

//! 3x3 matrix scalar multiplication.
#define AM_MATRIX33_SCL_MUL(_dst, _factor, _m) \
	(_dst)->a[0][0] = (_factor) * (_m)->a[0][0]; \
	(_dst)->a[0][1] = (_factor) * (_m)->a[0][1]; \
	(_dst)->a[0][2] = (_factor) * (_m)->a[0][2]; \
	(_dst)->a[1][0] = (_factor) * (_m)->a[1][0]; \
	(_dst)->a[1][1] = (_factor) * (_m)->a[1][1]; \
	(_dst)->a[1][2] = (_factor) * (_m)->a[1][2]; \
	(_dst)->a[2][0] = (_factor) * (_m)->a[2][0]; \
	(_dst)->a[2][1] = (_factor) * (_m)->a[2][1]; \
	(_dst)->a[2][2] = (_factor) * (_m)->a[2][2];

//! 3x3 matrix self scalar multiplication.
#define AM_MATRIX33_SELF_SCL_MUL(_dst, _factor) \
	(_dst)->a[0][0] *= (_factor); \
	(_dst)->a[0][1] *= (_factor); \
	(_dst)->a[0][2] *= (_factor); \
	(_dst)->a[1][0] *= (_factor); \
	(_dst)->a[1][1] *= (_factor); \
	(_dst)->a[1][2] *= (_factor); \
	(_dst)->a[2][0] *= (_factor); \
	(_dst)->a[2][1] *= (_factor); \
	(_dst)->a[2][2] *= (_factor);

//! 3x3 matrix, Frobenius norm.
#define AM_MATRIX33_NORM_FROBENIUS(_dst) \
	(amSqrtf((_dst)->a[0][0] * (_dst)->a[0][0] + \
		     (_dst)->a[0][1] * (_dst)->a[0][1] + \
			 (_dst)->a[0][2] * (_dst)->a[0][2] + \
			 (_dst)->a[1][0] * (_dst)->a[1][0] + \
			 (_dst)->a[1][1] * (_dst)->a[1][1] + \
			 (_dst)->a[1][2] * (_dst)->a[1][2] + \
			 (_dst)->a[2][0] * (_dst)->a[2][0] + \
			 (_dst)->a[2][1] * (_dst)->a[2][1] + \
			 (_dst)->a[2][2] * (_dst)->a[2][2]))

//! It constructs a 3x3 scale matrix, given scale factors.
#define AM_SCALE_TO_MATRIX33(_dst, _valueX, _valueY) \
	(_dst)->a[0][0] = (_valueX); \
	(_dst)->a[0][1] = 0.0f; \
	(_dst)->a[0][2] = 0.0f; \
	(_dst)->a[1][0] = 0.0f; \
	(_dst)->a[1][1] = (_valueY); \
	(_dst)->a[1][2] = 0.0f; \
	(_dst)->a[2][0] = 0.0f; \
	(_dst)->a[2][1] = 0.0f; \
	(_dst)->a[2][2] = 1.0f;

//! It constructs a 3x3 translation matrix, given translation values.
#define AM_TRANSLATION_TO_MATRIX33(_dst, _valueX, _valueY) \
	(_dst)->a[0][0] = 1.0f; \
	(_dst)->a[0][1] = 0.0f; \
	(_dst)->a[0][2] = (_valueX); \
	(_dst)->a[1][0] = 0.0f; \
	(_dst)->a[1][1] = 1.0f; \
	(_dst)->a[1][2] = (_valueY); \
	(_dst)->a[2][0] = 0.0f; \
	(_dst)->a[2][1] = 0.0f; \
	(_dst)->a[2][2] = 1.0f;

//! It constructs a 3x3 identity matrix.
#define AM_MATRIX33_IDENTITY(_dst) \
	(_dst)->a[0][0] = 1; \
	(_dst)->a[0][1] = 0; \
	(_dst)->a[0][2] = 0; \
	(_dst)->a[1][0] = 0; \
	(_dst)->a[1][1] = 1; \
	(_dst)->a[1][2] = 0; \
	(_dst)->a[2][0] = 0; \
	(_dst)->a[2][1] = 0; \
	(_dst)->a[2][2] = 1;

//! It transposes a 3x3 matrix.
#define AM_MATRIX33_TRANSPOSE(_dst, _m) \
	(_dst)->a[0][0] = (_m)->a[0][0]; \
	(_dst)->a[0][1] = (_m)->a[1][0]; \
	(_dst)->a[0][2] = (_m)->a[2][0]; \
	(_dst)->a[1][0] = (_m)->a[0][1]; \
	(_dst)->a[1][1] = (_m)->a[1][1]; \
	(_dst)->a[1][2] = (_m)->a[2][1]; \
	(_dst)->a[2][0] = (_m)->a[0][2]; \
	(_dst)->a[2][1] = (_m)->a[1][2]; \
	(_dst)->a[2][2] = (_m)->a[2][2];

//! It self transposes a 3x3 matrix. \a _dataType must be the same type of matrix elements.
#define AM_MATRIX33_SELF_TRANSPOSE(_m, _dataType) { \
	_dataType _tmp; \
	_tmp = (_m)->a[1][0]; \
	(_m)->a[1][0] = (_m)->a[0][1]; \
	(_m)->a[0][1] = _tmp; \
	_tmp = (_m)->a[2][0]; \
	(_m)->a[2][0] = (_m)->a[0][2]; \
	(_m)->a[0][2] = _tmp; \
	_tmp = (_m)->a[2][1]; \
	(_m)->a[2][1] = (_m)->a[1][2]; \
	(_m)->a[1][2] = _tmp; \
}

//! Full 3x3 matrix multiplication. \a _matrixType must be the same type of input matrices.
#define AM_MATRIX33_MUL(_dst, _m0, _m1, _matrixType) { \
		_matrixType _tmp; \
		_tmp.a[0][0] = (_m0)->a[0][0] * (_m1)->a[0][0] + (_m0)->a[0][1] * (_m1)->a[1][0] + (_m0)->a[0][2] * (_m1)->a[2][0]; \
		_tmp.a[0][1] = (_m0)->a[0][0] * (_m1)->a[0][1] + (_m0)->a[0][1] * (_m1)->a[1][1] + (_m0)->a[0][2] * (_m1)->a[2][1]; \
		_tmp.a[0][2] = (_m0)->a[0][0] * (_m1)->a[0][2] + (_m0)->a[0][1] * (_m1)->a[1][2] + (_m0)->a[0][2] * (_m1)->a[2][2]; \
		_tmp.a[1][0] = (_m0)->a[1][0] * (_m1)->a[0][0] + (_m0)->a[1][1] * (_m1)->a[1][0] + (_m0)->a[1][2] * (_m1)->a[2][0]; \
		_tmp.a[1][1] = (_m0)->a[1][0] * (_m1)->a[0][1] + (_m0)->a[1][1] * (_m1)->a[1][1] + (_m0)->a[1][2] * (_m1)->a[2][1]; \
		_tmp.a[1][2] = (_m0)->a[1][0] * (_m1)->a[0][2] + (_m0)->a[1][1] * (_m1)->a[1][2] + (_m0)->a[1][2] * (_m1)->a[2][2]; \
		_tmp.a[2][0] = (_m0)->a[2][0] * (_m1)->a[0][0] + (_m0)->a[2][1] * (_m1)->a[1][0] + (_m0)->a[2][2] * (_m1)->a[2][0]; \
		_tmp.a[2][1] = (_m0)->a[2][0] * (_m1)->a[0][1] + (_m0)->a[2][1] * (_m1)->a[1][1] + (_m0)->a[2][2] * (_m1)->a[2][1]; \
		_tmp.a[2][2] = (_m0)->a[2][0] * (_m1)->a[0][2] + (_m0)->a[2][1] * (_m1)->a[1][2] + (_m0)->a[2][2] * (_m1)->a[2][2]; \
		(*(_dst)) = _tmp; \
	}

//! 3x3 matrix - 3D vector multiplication. \a _vectType must be the same type of the vector.
#define AM_MATRIX33_VECT3_MUL(_dst, _m, _v, _vectType) { \
		_vectType _tmp; \
		_tmp.x = (_m)->a[0][0] * (_v)->x + (_m)->a[0][1] * (_v)->y + (_m)->a[0][2] * (_v)->z; \
		_tmp.y = (_m)->a[1][0] * (_v)->x + (_m)->a[1][1] * (_v)->y + (_m)->a[1][2] * (_v)->z; \
		_tmp.z = (_m)->a[2][0] * (_v)->x + (_m)->a[2][1] * (_v)->y + (_m)->a[2][2] * (_v)->z; \
		(*(_dst)) = _tmp; \
	}

//! 3x3 matrix - 2D vector self multiplication. \a _vectType must be the same type of the vector.
#define AM_MATRIX33_VECT2_SELF_MUL(_m, _v, _vectType) { \
		_vectType _tmp; \
		_tmp.x = (_m)->a[0][0] * (_v)->x + (_m)->a[0][1] * (_v)->y + (_m)->a[0][2]; \
		_tmp.y = (_m)->a[1][0] * (_v)->x + (_m)->a[1][1] * (_v)->y + (_m)->a[1][2]; \
		(*(_v)) = _tmp; \
	}

//! 3x3 matrix - 2D vector multiplication.
#define AM_MATRIX33_VECT2_MUL(_dst, _m, _v) \
	(_dst)->x = (_m)->a[0][0] * (_v)->x + (_m)->a[0][1] * (_v)->y + (_m)->a[0][2]; \
	(_dst)->y = (_m)->a[1][0] * (_v)->x + (_m)->a[1][1] * (_v)->y + (_m)->a[1][2];

/*!
	It performs a 3x3 matrix - 2D vector multiplication, then a perspective projection. \a _vectType must be the type
	of the vector, \a _vectDataType must be the same type of vector components.
*/
#define AM_MATRIX33_VECT2_MUL_PERSPECTIVE(_dst, _m, _v, _vectType, _vectDataType) { \
		_vectType _tmp; \
		_vectDataType _z; \
		_tmp.x = (_m)->a[0][0] * (_v)->x + (_m)->a[0][1] * (_v)->y + (_m)->a[0][2]; \
		_tmp.y = (_m)->a[1][0] * (_v)->x + (_m)->a[1][1] * (_v)->y + (_m)->a[1][2]; \
		_z = (_m)->a[2][0] * (_v)->x + (_m)->a[2][1] * (_v)->y + (_m)->a[2][2]; \
		if (_z != 0) { \
			tmp.x /= _z; \
			tmp.y /= _z; \
		} \
		(*(_dst)) = _tmp; \
	}

//! It constructs a 4x4 identity matrix.
#define AM_MATRIX44_IDENTITY(_dst) \
	(_dst)->a[0][0] = 1; \
	(_dst)->a[0][1] = 0; \
	(_dst)->a[0][2] = 0; \
	(_dst)->a[0][3] = 0; \
	(_dst)->a[1][0] = 0; \
	(_dst)->a[1][1] = 1; \
	(_dst)->a[1][2] = 0; \
	(_dst)->a[1][3] = 0; \
	(_dst)->a[2][0] = 0; \
	(_dst)->a[2][1] = 0; \
	(_dst)->a[2][2] = 1; \
	(_dst)->a[2][3] = 0; \
	(_dst)->a[3][0] = 0; \
	(_dst)->a[3][1] = 0; \
	(_dst)->a[3][2] = 0; \
	(_dst)->a[3][3] = 1;

#if defined(AM_FIXED_POINT_PIPELINE)
/*!
	\brief It converts a floating point affine matrix into a 17.15 fixed point equivalent.
	\param dst output (fixed point) matrix.
	\param src input (floating point) matrix.
	\note the conversion could silently overflow.
*/
AM_INLINE void amRasMatrixFToX(AMMatrix33x *dst,
							   const AMMatrix33f *src) {

	AM_ASSERT(dst);
	AM_ASSERT(src);

	dst->a[0][0] = amFloatToFixed1715(src->a[0][0]);
	dst->a[0][1] = amFloatToFixed1715(src->a[0][1]);
	dst->a[0][2] = amFloatToFixed1715(src->a[0][2]);
	dst->a[1][0] = amFloatToFixed1715(src->a[1][0]);
	dst->a[1][1] = amFloatToFixed1715(src->a[1][1]);
	dst->a[1][2] = amFloatToFixed1715(src->a[1][2]);
	dst->a[2][0] = 0;
	dst->a[2][1] = 0;
	dst->a[2][2] = 1 << 15;
}
#endif

/*!
	\brief Check if a 3x3 matrix is affine, under the float precision.
	\param m input matrix.
	\return AM_TRUE if matrix is affine, else AM_FALSE.
*/
AM_INLINE AMbool amMatrix33fIsAffine(const AMMatrix33f *m) {

	if (amAbsf(m->a[2][0]) > AM_EPSILON_FLOAT ||
		amAbsf(m->a[2][1]) > AM_EPSILON_FLOAT ||
		amAbsf(m->a[2][0]) > 1.0f + AM_EPSILON_FLOAT)
		return AM_FALSE;
	else
		return AM_TRUE;
}

// Affine 3x3 matrix inversion.
AMbool amMatrix33fInvertAffine(AMMatrix33f *dst,
							   const AMMatrix33f *src);
//	Full 3x3 matrix inversion using Gauss-Jordan elimination using maximum pivot strategy.
AMbool amMatrix33fInvert(AMMatrix33f *dst,
						 AMfloat *determinant,
						 const AMMatrix33f *m);
//	Full 5x5 matrix inversion using Gauss-Jordan elimination using maximum pivot strategy.
AMbool amMatrix55fInvert(AMMatrix55f *dst,
						 AMfloat *determinant,
						 const AMMatrix55f *m);
// Polar decomposition, it produces unique factors Q (rotation) and S (scale)
AMbool amMatrix33fPolarDecompose(AMMatrix33f *rot,
							  AMfloat *rotDeterminant,
							  AMMatrix33f *scl,
							  const AMMatrix33f *src);
// Solve eigenvalues/eigenvectors (for 3x3 matrix), induced by the specified matrix.
void amMatrix33fEigenSolver(AMVect2f *eigenValue1,
							AMVect2f *eigenValue2,
							AMVect2f *eigenValue3,
							AMVect3f *eigenVector1,
							AMVect3f *eigenVector2,
							AMVect3f *eigenVector3,
							const AMMatrix33f *matrix,
							const AMbool sort);
// Extract scale factors from a given matrix.
void amMatrix33fScaleFactors(AMVect3f *scaleFactors,
							 const AMMatrix33f *matrix);

#endif
