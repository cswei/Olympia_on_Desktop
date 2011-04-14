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
	\file matrix.c
	\brief Matrix related definitions and routines, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "matrix.h"

#if defined RIM_VG_SRC
#include "bugdispc.h"
#endif

/*!
	\brief It inverts a 3x3 affine matrix (float elements).
	\param dst output (inverted) matrix.
	\param src input matrix.
	\return AM_TRUE if input matrix is invertible, else AM_FALSE.
	\pre \a src input matrix must be affine, and it must not refers to \a dst matrix (they must be different pointers).
*/
AMbool amMatrix33fInvertAffine(AMMatrix33f *dst,
							   const AMMatrix33f *src) {

	AMfloat a = src->a[0][0];
	AMfloat b = src->a[0][1];
	AMfloat c = src->a[1][0];
	AMfloat d = src->a[1][1];
	AMdouble det = (AMdouble)a * d - (AMdouble)b * c;
	AMdouble invDet, a00, a01, a10, a11;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(dst != src);
	AM_ASSERT(amMatrix33fIsAffine(src));

	// check for singularity
	if (det == 0.0) {
		AM_MATRIX33_IDENTITY(dst)
		return AM_FALSE;
	}
	invDet = 1.0 / det;

	a00 = d * invDet;
	a01 = -b * invDet;
	a10 = -c * invDet;
	a11 = a * invDet;

	dst->a[0][0] = (AMfloat)a00;
	dst->a[0][1] = (AMfloat)a01;
	dst->a[0][2] = (AMfloat)(-src->a[0][2] * a00 - src->a[1][2] * a01);
	dst->a[1][0] = (AMfloat)a10;
	dst->a[1][1] = (AMfloat)a11;
	dst->a[1][2] = (AMfloat)(-src->a[0][2] * a10 - src->a[1][2] * a11);
	dst->a[2][0] = 0.0f;
	dst->a[2][1] = 0.0f;
	dst->a[2][2] = 1.0f;
	return AM_TRUE;
}

/*!
	\brief It inverts a 3x3 matrix (float elements), using Gauss-Jordan elimination with maximum pivot strategy.
	\param dst output (inverted) matrix.
	\param determinant output determinant, if the matrix is invertible.
	\param m input matrix.
	\return AM_TRUE if input matrix is invertible, else AM_FALSE.
*/
AMbool amMatrix33fInvert(AMMatrix33f *dst,
						 AMfloat *determinant,
						 const AMMatrix33f *m) {

	AMfloat cs[3], pv, pav, det = 1.0f;
	AMint32 i, ik, j, jk, k, pc[3], pl[3];
	AMMatrix33f _result;

	#define SWAP_ELEMENT(_r0, _c0, _r1, _c1) \
		AMfloat swapElem = _result.a[_r0][_c0]; \
		_result.a[_r0][_c0] = _result.a[_r1][_c1]; \
		_result.a[_r1][_c1] = swapElem;

	AM_ASSERT(dst);
	AM_ASSERT(m);

	_result = *m;

	// initializations
	pc[0] = pl[0] = pc[1] = pl[1] = pc[2] = pl[2] = 0;
	cs[0] = cs[1] = cs[2] = 0.0f;
	for (k = 0; k < 3; ++k) {
		// find the greatest pivot
		pv = _result.a[k][k];
		ik = jk = k;
		pav = amAbsf(pv);
		for (i = k; i < 3; ++i)
			for (j = k; j < 3; ++j) {
				if (amAbsf(_result.a[i][j]) > pav) {
					pv = _result.a[i][j];
					pav = amAbsf(pv);
					ik = i;
					jk = j;
				}
			}
			// pivot is in (ik, jk)
			pc[k] = jk;
			pl[k] = ik;
			// track swapping
			if (ik != k) det = -det;
			if (jk != k) det = -det;
			// is the matrix singular?
			if (pv == 0.0f) {
				if (determinant) *determinant = 0.0f;
				// matrix rearrangement, lines
				for (i = 2; i >= 0; --i) {
					ik = pc[i];
					if (ik == i) continue;
					// swap i, pc[i] lines
					for (j = 0; j < 3; ++j) {
						SWAP_ELEMENT(i, j, ik, j)
					}
				}
				// matrix rearrangement, columns
				for (j = 2; j >= 0; --j) {
					jk = pl[j];
					if (jk == j) continue;
					// swap j, pl[j] columns
					for (i = 0; i < 3; ++i) {
						SWAP_ELEMENT(i, j, i, jk)
					}
				}
				// partial result
				*dst = _result;
				return AM_FALSE;
			}
			// update the determinant accumulating the pivot
			det *= pv;
			// pivot in (k, k)
			if (ik != k)
				for (i = 0; i < 3; ++i) {
					// swap ik, k lines
					SWAP_ELEMENT(ik, i, k, i)
				}
				// pivot located at the right line
				if (jk != k)
					for (i = 0; i < 3; ++i) {
						// swap jk, k columns
						SWAP_ELEMENT(i, jk, i, k)
					}
					// pivot located at the right column
					// cs vector stores the k column, k column now is set to 0
					cs[0] = _result.a[0][k];
					_result.a[0][k] = 0.0f;
					cs[1] = _result.a[1][k];
					_result.a[1][k] = 0.0f;
					cs[2] = _result.a[2][k];
					_result.a[2][k] = 0.0f;

					cs[k] = 0.0f;
					_result.a[k][k] = 1.0f;
					// line k modified
					_result.a[k][0] = _result.a[k][0] / pv;
					_result.a[k][1] = _result.a[k][1] / pv;
					_result.a[k][2] = _result.a[k][2] / pv;
					// other lines modified
					for (j = 0; j < 3; ++j) {
						if (j == k)	continue;
						// line j modified
						_result.a[j][0] = _result.a[j][0] - cs[j] * _result.a[k][0];
						_result.a[j][1] = _result.a[j][1] - cs[j] * _result.a[k][1];
						_result.a[j][2] = _result.a[j][2] - cs[j] * _result.a[k][2];
					}
	}
	// matrix rearrangement, lines
	for (i = 2; i >= 0; --i) {
		ik = pc[i];
		if (ik == i) continue;
		// swap i, pc[i] lines
		for (j = 0; j < 3; ++j) {
			SWAP_ELEMENT(i, j, ik, j)
		}
	}
	// matrix rearrangement, columns
	for (j = 2; j >= 0; --j) {
		jk = pl[j];
		if (jk == j) continue;
		// swap j, pl[j] columns
		for (i = 0; i < 3; ++i) {
			SWAP_ELEMENT(i, j, i, jk)
		}
	}
	*dst = _result;
	if (determinant) *determinant = det;
	return AM_TRUE;
	#undef SWAP_ELEMENT
}

/*!
	\brief It inverts a 5x5 matrix (float elements), using Gauss-Jordan elimination with maximum pivot strategy.
	\param dst output (inverted) matrix.
	\param determinant output determinant, if the matrix is invertible.
	\param m input matrix.
	\return AM_TRUE if input matrix is invertible, else AM_FALSE.
*/
AMbool amMatrix55fInvert(AMMatrix55f *dst,
						 AMfloat *determinant,
						 const AMMatrix55f *m) {

	AMfloat cs[5], pv, pav, det = 1.0f;
	AMint32 i, ik, j, jk, k, pc[5], pl[5];
	AMMatrix55f _result;

	#define SWAP_ELEMENT(_r0, _c0, _r1, _c1) \
		AMfloat swapElem = _result.a[_r0][_c0]; \
		_result.a[_r0][_c0] = _result.a[_r1][_c1]; \
		_result.a[_r1][_c1] = swapElem;

	AM_ASSERT(dst);
	AM_ASSERT(m);

	_result = *m;

	// initializations
	pc[0] = pl[0] = pc[1] = pl[1] = pc[2] = pl[2] = pc[3] = pl[3] = pc[4] = pl[4] = 0;
	cs[0] = cs[1] = cs[2] = cs[3] = cs[4] = 0.0f;
	for (k = 0; k < 5; ++k) {
		// find the greatest pivot
		pv = _result.a[k][k];
		ik = jk = k;
		pav = amAbsf(pv);
		for (i = k; i < 5; ++i)
			for (j = k; j < 5; ++j) {
				if (amAbsf(_result.a[i][j]) > pav) {
					pv = _result.a[i][j];
					pav = amAbsf(pv);
					ik = i;
					jk = j;
				}
			}
			// pivot is in (ik, jk)
			pc[k] = jk;
			pl[k] = ik;
			// track swapping
			if (ik != k) det = -det;
			if (jk != k) det = -det;
			// is the matrix singular?
			if (pv == 0.0f) {
				if (determinant) *determinant = 0.0f;
				// matrix rearrangement, lines
				for (i = 4; i >= 0; --i) {
					ik = pc[i];
					if (ik == i) continue;
					// swap i, pc[i] lines
					for (j = 0; j < 5; ++j) {
						SWAP_ELEMENT(i, j, ik, j)
					}
				}
				// matrix rearrangement, columns
				for (j = 4; j >= 0; --j) {
					jk = pl[j];
					if (jk == j) continue;
					// swap j, pl[j] columns
					for (i = 0; i < 5; ++i) {
						SWAP_ELEMENT(i, j, i, jk)
					}
				}
				// partial result
				*dst = _result;
				return AM_FALSE;
			}
			// update the determinant accumulating the pivot
			det *= pv;
			// pivot in (k, k)
			if (ik != k)
				for (i = 0; i < 5; ++i) {
					// swap ik, k lines
					SWAP_ELEMENT(ik, i, k, i)
				}
				// pivot located at the right line
				if (jk != k)
					for (i = 0; i < 5; ++i) {
						// swap jk, k columns
						SWAP_ELEMENT(i, jk, i, k)
					}
					// pivot located at the right column
					// cs vector stores the k column, k column now is set to 0
					cs[0] = _result.a[0][k];
					_result.a[0][k] = 0.0f;
					cs[1] = _result.a[1][k];
					_result.a[1][k] = 0.0f;
					cs[2] = _result.a[2][k];
					_result.a[2][k] = 0.0f;
					cs[3] = _result.a[3][k];
					_result.a[3][k] = 0.0f;
					cs[4] = _result.a[4][k];
					_result.a[4][k] = 0.0f;

					cs[k] = 0.0f;
					_result.a[k][k] = 1.0f;
					// line k modified
					_result.a[k][0] = _result.a[k][0] / pv;
					_result.a[k][1] = _result.a[k][1] / pv;
					_result.a[k][2] = _result.a[k][2] / pv;
					_result.a[k][3] = _result.a[k][3] / pv;
					_result.a[k][4] = _result.a[k][4] / pv;
					// other lines modified
					for (j = 0; j < 5; ++j) {
						if (j == k)	continue;
						// line j modified
						_result.a[j][0] = _result.a[j][0] - cs[j] * _result.a[k][0];
						_result.a[j][1] = _result.a[j][1] - cs[j] * _result.a[k][1];
						_result.a[j][2] = _result.a[j][2] - cs[j] * _result.a[k][2];
						_result.a[j][3] = _result.a[j][3] - cs[j] * _result.a[k][3];
						_result.a[j][4] = _result.a[j][4] - cs[j] * _result.a[k][4];
					}
	}
	// matrix rearrangement, lines
	for (i = 4; i >= 0; --i) {
		ik = pc[i];
		if (ik == i) continue;
		// swap i, pc[i] lines
		for (j = 0; j < 5; ++j) {
			SWAP_ELEMENT(i, j, ik, j)
		}
	}
	// matrix rearrangement, columns
	for (j = 4; j >= 0; --j) {
		jk = pl[j];
		if (jk == j) continue;
		// swap j, pl[j] columns
		for (i = 0; i < 5; ++i) {
			SWAP_ELEMENT(i, j, i, jk)
		}
	}
	*dst = _result;
	if (determinant) *determinant = det;
	return AM_TRUE;
	#undef SWAP_ELEMENT
}

/*
	\brief Polar decomposition, it produces unique factors Q (rotation) and S (scale).
	\param rot output rotation matrix.
	\param rotDeterminant output determinant of the rotation matrix.
	\param scl output scale matrix.
	\param src input matrix.
	\return AM_TRUE if input matrix can be decomposed, else AM_FALSE.
*/
AMbool amMatrix33fPolarDecompose(AMMatrix33f *rot,
								 AMfloat *rotDeterminant,
								 AMMatrix33f *scl,
								 const AMMatrix33f *src) {

	AMMatrix33f w, a, tw, tmp0, tmp1;
	AMfloat limit, g, f, n1, n2, pf, det;
	AMbool invOk;
	AMint32 i, j;

	AM_ASSERT(src);
	AM_ASSERT(rot);
	AM_ASSERT(rotDeterminant);
	AM_ASSERT(scl);

	a = *src;
	w = a;
	// stopping criterion
	limit = (1.0f + AM_EPSILON_FLOAT) * amSqrtf(3.0f);

	// initialization
	AM_MATRIX33_TRANSPOSE(&tw, &w)
	invOk = amMatrix33fInvert(&a, &det, &tw);
	if (!invOk) {
		*rotDeterminant = 0.0f;
		return AM_FALSE;
	}

	n1 = AM_MATRIX33_NORM_FROBENIUS(&a);
	n2 = AM_MATRIX33_NORM_FROBENIUS(&w);
	g = amSqrtf(n1 / n2);
	
	// w = 0.5f * (g * w + (1.0f / g) * a);
	AM_MATRIX33_SCL_MUL(&tmp0, 0.5f * g, &w)
	AM_MATRIX33_SCL_MUL(&tmp1, 0.5f / g, &a)
	AM_MATRIX33_ADD(&w, &tmp0, &tmp1)

	f = AM_MATRIX33_NORM_FROBENIUS(&w);
	pf = AM_MAX_FLOAT;
	while ((f > limit) && (f < pf)) {
		pf = f;
		AM_MATRIX33_TRANSPOSE(&tw, &w)
		invOk = amMatrix33fInvert(&a, &det, &tw);
		AM_ASSERT(invOk);

		n1 = AM_MATRIX33_NORM_FROBENIUS(&a);
		g = amSqrtf(n1 / f);
		// w = 0.5f * (g * w + (1.0f / g) * a);
		AM_MATRIX33_SCL_MUL(&tmp0, 0.5f * g, &w)
		AM_MATRIX33_SCL_MUL(&tmp1, 0.5f / g, &a)
		AM_MATRIX33_ADD(&w, &tmp0, &tmp1)
		f = AM_MATRIX33_NORM_FROBENIUS(&w);
	}

	// finally build Q factor and its determinant
	*rot = w;
	*rotDeterminant = det;

	// now build S factor
	AM_MATRIX33_TRANSPOSE(&tw, &w)
	AM_MATRIX33_MUL(scl, &tw, src, AMMatrix33f)

	for (i = 0; i < 3; ++i)
		for (j = i; j < 3; ++j)
			scl->a[i][j] = scl->a[j][i] = ((scl->a[i][j] + scl->a[j][i]) * 0.5f);
	return AM_TRUE;
}

/*!
	\brief It reduces a symmetric 3x3 matrix to a tridiagonal form, using Householder transformations.
	\param matrix input (symmetric) matrix.
	\param diag output diagonal.
	\param subDiag output sub-diagonal.
*/
void amMatrix33fTridiagonalReduce(AMMatrix33f *matrix,
								  AMVect3f *diag,
								  AMVect3f *subDiag) {

	AMfloat m00 = matrix->a[0][0];
	AMfloat m01 = matrix->a[0][1];
	AMfloat m02 = matrix->a[0][2];
	AMfloat m11 = matrix->a[1][1];
	AMfloat m12 = matrix->a[1][2];
	AMfloat m22 = matrix->a[2][2];

	AM_ASSERT(matrix);
	AM_ASSERT(diag);
	AM_ASSERT(subDiag);

	diag->x = m00;
	subDiag->z = 0.0f;
	if (m02 != 0.0f) {

		AMfloat length = amSqrtf(m01 * m01 + m02 * m02);
		AMfloat invLength = 1.0f / length;
		AMfloat q;

		m01 *= invLength;
		m02 *= invLength;
		q = 2.0f * m01 * m12 + m02 * (m22 - m11);
		diag->y = m11 + m02 * q;
		diag->z = m22 - m02 * q;
		subDiag->x = length;
		subDiag->y = m12 - m01 * q;
		matrix->a[0][0] = 1.0f;
		matrix->a[0][1] = 0.0f;
		matrix->a[0][2] = 0.0f;
		matrix->a[1][0] = 0.0f;
		matrix->a[1][1] = m01;
		matrix->a[1][2] = m02;
		matrix->a[2][0] = 0.0f;
		matrix->a[2][1] = m02;
		matrix->a[2][2] = -m01;
	}
	else {
		diag->y = m11;
		diag->z = m22;
		subDiag->x = m01;
		subDiag->y = m12;
		AM_MATRIX33_IDENTITY(matrix)
	}
}

/*!
	\brief It calculates eigenvalues and eigenvector of a tridiagonal matrix, using QL algorithm.
	\param matrix output matrix containing eigenvectors.
	\param diag input matrix diagonal; when finished it will contain real parts of eigenvalues.
	\param subDiag input matrix sub-diagonal; when finished it will contain imaginary parts of eigenvalues.
	\param sort if AM_TRUE, eigenvalues (and relative eigenvectors) are sorted (descending order).
	\note This function has been derived from Jama (Java matrix package).\n
	Original software Copyright Notice:\n
	"This software is a cooperative product of The MathWorks and the National Institute of
	Standards and Technology (NIST) which has been released to the public domain."
*/
void amMatrix33fTridQL(AMMatrix33f *matrix,
					   AMVect3f *diag,
					   AMVect3f *subDiag,
					   const AMbool sort) {
	
	AMint32 l, m, i, j, k;
	AMfloat f = 0.0f;
	AMfloat tst1 = 0.0f;
	AMfloat *d = (AMfloat *)diag, *e = (AMfloat *)subDiag;
	
	for (l = 0; l < 3; l++) {
		// find small sub-diagonal element
		tst1 = AM_MAX(tst1, amAbsf(d[l]) + amAbsf(e[l]));
		m = l;

		while (m < 3) {
			if (amAbsf(e[m]) <= AM_EPSILON_FLOAT * tst1)
				break;
			m++;
		}

    #if defined RIM_VG_SRC
        // fixes coverity static overrun of 3x3 matrix->a
        if( m >= 3 ) { 
            kdCatfailRim("Out of bounds indexing 3x3 matrix");
        }
    #endif

		// if m == l, d[l] is an eigenvalue, otherwise, iterate.
		if (m > l) {
			AMint32 iter = 0;
			AMfloat g, p, r, dl1, h, c, c2, c3, el1, s, s2;
			do {
				iter++;  // (could check iteration count here)
				// compute implicit shift
				g = d[l];
				p = (d[l + 1] - g) / (2.0f * e[l]);
				r = amHypotf(p, 1.0f);
				if (p < 0.0f)
					r = -r;
				d[l] = e[l] / (p + r);
				d[l + 1] = e[l] * (p + r);
				dl1 = d[l + 1];
				h = g - d[l];
				for (i = l + 2; i < 3; ++i)
					d[i] -= h;
				f = f + h;
				// implicit QL transformation
				p = d[m];
				c = 1.0f;
				c2 = c;
				c3 = c;
				el1 = e[l + 1];
				s = 0.0f;
				s2 = 0.0f;
				for (i = m - 1; i >= l; --i) {
					c3 = c2;
					c2 = c;
					s2 = s;
					g = c * e[i];
					h = c * p;
					r = amHypotf(p, e[i]);
					e[i + 1] = s * r;
					s = e[i] / r;
					c = p / r;
					p = c * d[i] - s * g;
					d[i + 1] = h + s * (c * g + s * d[i]);
					// accumulate transformation
					for (k = 0; k < 3; ++k) {
						h = matrix->a[k][i + 1];
						matrix->a[k][i + 1] = s * matrix->a[k][i] + c * h;
						matrix->a[k][i] = c * matrix->a[k][i] - s * h;
					}
				}
				p = -s * s2 * c3 * el1 * e[l] / dl1;
				e[l] = s * p;
				d[l] = c * p;
				// check for convergence
			} while (amAbsf(e[l]) > AM_EPSILON_FLOAT * tst1);
		}
		d[l] = d[l] + f;
		e[l] = 0.0f;
	}
	// check for sort
	if (!sort)
		return;
	// sort eigenvalues and corresponding vectors
	for (i = 0; i < 2; ++i) {

		AMfloat p = d[i];

		k = i;
		for (j = i + 1; j < 3; ++j) {
			if (d[j] < p) {
				k = j;
				p = d[j];
			}
		}
		if (k != i) {
			d[k] = d[i];
			d[i] = p;
			for (j = 0; j < 3; ++j) {
				p = matrix->a[j][i];
				matrix->a[j][i] = matrix->a[j][k];
				matrix->a[j][k] = p;
			}
		}
	}
}

/*!
	\brief Solve eigenvalues/eigenvectors, induced by the specified symmetric 3x3 matrix.
	\param eigenValue1 first eigenvalue. At index 0 there'll be the real part, at index 1 the imaginary part.
	\param eigenValue2 second eigenvalue. At index 0 there'll be the real part, at index 1 the imaginary part.
	\param eigenValue3 third eigenvalue. At index 0 there'll be the real part, at index 1 the imaginary part.
	\param eigenVector1 the vector corresponding to eigenValue1.
	\param eigenVector2 the vector corresponding to eigenValue2.
	\param eigenVector3 the vector corresponding to eigenValue3.
	\param matrix input symmetric matrix.
	\param sort if AM_TRUE eigenvalues (and their corresponding eigenvectors) are returned in descending order, so
	that eigenValue1 is the eigenvalue with the greater module.
	\pre input matrix must be symmetric.
*/
void amMatrix33fEigenSolver(AMVect2f *eigenValue1,
							AMVect2f *eigenValue2,
							AMVect2f *eigenValue3,
							AMVect3f *eigenVector1,
							AMVect3f *eigenVector2,
							AMVect3f *eigenVector3,
							const AMMatrix33f *matrix,
							const AMbool sort) {

	AMMatrix33f a;
	AMVect3f d, e;

	AM_ASSERT(matrix);

	a = *matrix;
	amMatrix33fTridiagonalReduce(&a, &d, &e);
	amMatrix33fTridQL(&a, &d, &e, sort);

	if (eigenValue1) {
		eigenValue1->x = d.x;
		eigenValue1->y = e.x;
	}
	if (eigenValue2) {
		eigenValue2->x = d.y;
		eigenValue2->y = e.y;
	}
	if (eigenValue3) {
		eigenValue3->x = d.z;
		eigenValue3->y = e.z;
	}

	if (eigenVector1) {
		eigenVector1->x = a.a[0][0];
		eigenVector1->y = a.a[1][0];
		eigenVector1->z = a.a[2][0];
	}
	if (eigenVector2) {
		eigenVector2->x = a.a[0][1];
		eigenVector2->y = a.a[1][1];
		eigenVector2->z = a.a[2][1];
	}
	if (eigenVector3) {
		eigenVector3->x = a.a[0][2];
		eigenVector3->y = a.a[1][2];
		eigenVector3->z = a.a[2][2];
	}
}

/*!
	\brief Extract scale factors from a given matrix.
	\param scaleFactors output scale factors.
	\param matrix input matrix.
*/
void amMatrix33fScaleFactors(AMVect3f *scaleFactors,
							 const AMMatrix33f *matrix) {

	AMMatrix33f rot, scl, srcCopy;
	AMfloat detRot;
	AMVect2f eigenValue1, eigenValue2, eigenValue3;
	AMVect3f eigenVector1, eigenVector2, eigenVector3;

	AM_ASSERT(scaleFactors);
	AM_ASSERT(matrix);

	// copy the source matrix forcing 0-translation and affinity
	AM_MATRIX33_SET(&srcCopy, matrix->a[0][0], matrix->a[0][1], 0.0f,
							  matrix->a[1][0], matrix->a[1][1], 0.0f,
							  0.0f, 0.0f, 1.0f)

	// faster pipeline for pure scale matrix
	if (amAbsf(srcCopy.a[0][1]) <= AM_EPSILON_FLOAT && amAbsf(srcCopy.a[1][0]) <= AM_EPSILON_FLOAT) {
		scaleFactors->x = srcCopy.a[0][0];
		scaleFactors->y = srcCopy.a[1][1];
		scaleFactors->z = 1.0f;
		return;
	}

	// first polar decompose given affine matrix
	if (amMatrix33fPolarDecompose(&rot, &detRot, &scl, &srcCopy)) {
		// spectral decomposition
		amMatrix33fEigenSolver(&eigenValue1, &eigenValue2, &eigenValue3, &eigenVector1, &eigenVector2, &eigenVector3, &scl, AM_FALSE);
		// extract stretch factors (eigenvalues are real numbers because S is symmetric with real values)
		scaleFactors->x = eigenValue1.x;
		scaleFactors->y = eigenValue2.x;
		scaleFactors->z = eigenValue3.x;
	}
	else {
		scaleFactors->x = 0.0f;
		scaleFactors->y = 0.0f;
		scaleFactors->z = 0.0f;
	}
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

