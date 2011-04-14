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

#ifndef _MATHEMATICS_H
#define _MATHEMATICS_H

/*!
	\file mathematics.h
	\brief Math routines, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "amanith_globals.h"
#if defined(AM_OS_BREW)
	#include <AEEStdLib.h>
#else
	#include <math.h>
	#include <stdlib.h>  // for abs(), integer version
#endif

//! PI constant.
AM_EXTERN_C const AMfloat AM_PI;
//! 2.0f * PI constant.
AM_EXTERN_C const AMfloat AM_2PI;
//! PI / 180.0f.
AM_EXTERN_C const AMfloat AM_DEG2RAD;
//! 180.0f / PI.
AM_EXTERN_C const AMfloat AM_RAD2DEG;

// it performs res = (a * b) / 255 just for unsigned integers
#define MULT_DIV_255(_res, _a, _b) \
	(_res) = (_a) * (_b) + 0x80; \
	(_res) = ((((_res) >> 8) + (_res)) >> 8);

//! Check if a float value is NaN (not a number).
AM_INLINE AM_NOALIAS AM_PURE AMbool amIsNan(const AMfloat value) {

#if defined(isnan)
	// C99 math.h
	return (isnan(value)) ? AM_TRUE : AM_FALSE;
#else
	// suppose IEEE 754 representation
	AMuint32 v = (*(AMuint32 *)&(value));
	return ((0x7F800000 - (v & 0x7FFFFFFF)) >> 31) ? AM_TRUE : AM_FALSE;
#endif
}

//! Check if a float value is -Inf/+Inf.
AM_INLINE AM_NOALIAS AM_PURE AMbool amIsInf(const AMfloat value) {

#if defined(isinf)
	// C99 math.h
	return (isinf(value)) ? AM_TRUE : AM_FALSE;
#else
	// suppose IEEE 754 representation
	AMuint32 v = (*(AMuint32 *)&(value));
	return ((v << 1) == 0xFF000000) ? AM_TRUE : AM_FALSE;
#endif
}

//! Force to 0 a NaN value, force minimum float for -Inf value, force maximum float for +Inf value, else return the passed value.
AM_INLINE AM_NOALIAS AM_PURE AMfloat amNanInfFix(const AMfloat value) {

	if (amIsNan(value))
		return 0.0f;
	else
	if (amIsInf(value))
		return (value < 0) ? AM_MIN_FLOAT : AM_MAX_FLOAT;
	else
		return value;
}

//! Absolute function (int specific version).
AM_INLINE AM_NOALIAS AM_PURE AMint32 amAbs(const AMint32 value) {

	#if defined(AM_OS_BREW)
		// simulate the absolute value
		if (value < 0)
			return(-value);
		else
			return value;
	#else
		return abs(value);
	#endif
}

//! Absolute function (float specific version).
AM_INLINE AM_NOALIAS AM_PURE AMfloat amAbsf(const AMfloat value) {

	#if defined(AM_OS_BREW)
		return (AMfloat)FABS(value);
	#else
		return (AMfloat)fabs(value);
	#endif
}

//! Ceil function (float specific version).
AM_INLINE AM_NOALIAS AM_PURE AMfloat amCeilf(const AMfloat value) {

	#if defined(AM_OS_BREW)
		return (AMfloat)FCEIL(value);
	#else
		return (AMfloat)ceil(value);
	#endif
}

//! Floor function (float specific version).
AM_INLINE AM_NOALIAS AM_PURE AMfloat amFloorf(const AMfloat value) {

	#if defined(AM_OS_BREW)
		return (AMfloat)FFLOOR(value);
	#else
		return (AMfloat)floor(value);
	#endif
}

//! Min macro.
#define AM_MIN(_a, _b) (((_a) < (_b)) ? (_a) : (_b))
//! Max macro.
#define AM_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
//! Clamp macro.
#define AM_CLAMP(_v, _lo, _hi) (((_v) > (_hi)) ? (_hi) : ((((_v) < (_lo)) ? (_lo) : (_v))))
//! Clamp4 macro (it clamps 4 values).
#define AM_CLAMP4(_dst, _src, _lo, _hi) \
	_dst[0] = AM_CLAMP(_src[0], _lo, _hi); \
	_dst[1] = AM_CLAMP(_src[1], _lo, _hi); \
	_dst[2] = AM_CLAMP(_src[2], _lo, _hi); \
	_dst[3] = AM_CLAMP(_src[3], _lo, _hi);

/*!
	Sign macro, it returns:\n
	- 0 if value == 0
	- 1 if value > 0
	- -1 if value < 0
*/
#define AM_SIGN(_v) (((_v) > (0)) ? (1) : ((((_v) < (0)) ? (-1) : (0))))

//! Cut off the digits after the decimal place.
AM_INLINE AM_NOALIAS AM_PURE AMfloat amTruncf(const AMfloat value) {

	return (value < 0.0f) ? amCeilf(value) : amFloorf(value);
}

//!	Calculates the cosine of the specified radians value (float version).
AM_INLINE AM_NOALIAS AM_PURE AMfloat amCosf(const AMfloat value) {

	#if defined(AM_OS_BREW)
		return (AMfloat)FCOS(value);
	#else
		return (AMfloat)cos(value);
	#endif
}


//!	Calculates the sine of the specified radians value (float version).
AM_INLINE AM_NOALIAS AM_PURE AMfloat amSinf(const AMfloat value) {

	#if defined(AM_OS_BREW)
		return (AMfloat)FSIN(value);
	#else
		return (AMfloat)sin(value);
	#endif
}
//!	Calculates sine and cosine of the specified radians value (float version).
AM_INLINE void amSinCosf(AMfloat *resSin,
						 AMfloat *resCos,
						 const AMfloat radAngle) {
#if defined(AM_OS_WIN32) && defined(AM_CC_MSVC)
	AMfloat c, s;
	__asm {
		fld dword ptr [radAngle]
		fsincos
		fstp dword ptr [c]
		fstp dword ptr [s]
	}
	*resSin = s;
	*resCos = c;
#else
	*resSin = amSinf(radAngle);
	*resCos = amCosf(radAngle);
#endif
}


#if defined(AM_OS_BREW)
AMfloat brewAcosf(const AMfloat value);
#endif

/*!
	\brief Calculates the arccosine of a given value (float version).
	\param value input value.
	\return the arccosine of passed value.
	\note if value is lesser than -1, this function returns AM_PI constant.\n
	If value is greater than 1, this function returns 0.
*/
AM_INLINE AM_NOALIAS AM_PURE AMfloat amAcosf(const AMfloat value) {

	if (-1.0f < value) {
		if (value < 1.0f)
		#if defined(AM_OS_BREW)
			return brewAcosf(value);
		#else
			return (AMfloat)acos(value);
		#endif
		return 0.0f;
	}
	return AM_PI;
}

#if defined(AM_OS_BREW)
AMfloat brewAtan2f(const AMfloat y,
				   const AMfloat x);
#endif
/*!
	Calculates the arctangent of y/x (float version).

	\param y first input value.
	\param x second input value.
	\return the arctangent of y/x, in the range [-PI; PI] radians.
	\note If both y and x are 0, this function returns 0.\n
	This function uses the signs of both parameters to determine the quadrant of the return value.
*/
AM_INLINE AM_NOALIAS AM_PURE AMfloat amAtan2f(const AMfloat y,
											  const AMfloat x) {

	#if defined(AM_OS_BREW)
		return brewAtan2f(y, x);
	#else
		return (AMfloat)atan2(y, x);
	#endif
}

/*!
	\brief Calculates the exponential of value (float version).
	\todo implement exp function on BREW.
*/
AM_INLINE AM_NOALIAS AM_PURE AMfloat amExpf(const AMfloat value) {

	#if defined(AM_OS_BREW)
		return 0.0f;
	#else
		return (AMfloat)exp(value);
	#endif
}

//!	Calculates base raised to the power of exponent (float version).
AM_INLINE AM_NOALIAS AM_PURE AMfloat amPowf(const AMfloat base,
											const AMfloat exponent) {

	#if defined(AM_OS_BREW)
		return (AMfloat)FPOW(base, exponent);
	#else
		return (AMfloat)pow(base, exponent);
	#endif
}

//! Calculates the inverse cube root of the given value.
AM_INLINE AM_NOALIAS AM_PURE AMfloat amInvCubeRootf(const AMfloat value) {

#if defined(AM_IEEE754_NOT_COMPLIANT)
	return amPowf(value, -0.33333333333f);
#else
	union {
		AMuint32 u;
		AMfloat f;
	} estimate;

	AMint32 exp;
	AMfloat e, e2, e3;

	estimate.f = value;
	// for IEEE-754 compliant machines like PPC and x86
	exp = (estimate.u & 0x7F800000UL) >> 23;
	// remove the bias
	exp -= 127;
	// multiply by -1/3 in such a way to put the bits in the right position
	exp *= -2796203;
	// add the bias back in
	exp += 127 << 23;
	// put the exponent and a garbage significand back in
	estimate.u = (estimate.u & 0x80000000UL) | (exp & 0x7FFFFFFFUL);
	// do some iterations using the full (non-approximated) refinement
	e = estimate.f;
	e2 = e * e;
	e3 = e2 * e;
	e += (1.0f - value * e3) / (3.0f * value * e2);
	e2 = e * e;
	e3 = e2 * e;
	e += (1.0f - value * e3) / (3.0f * value * e2);
	// do some iterations using the approximated refinement now that the estimate is good
	e += (1.0f - value * e * e * e) * e * 0.33333333333f;
	e += (1.0f - value * e * e * e) * e * 0.33333333333f;
	e += (1.0f - value * e * e * e) * e * 0.33333333333f;
	return e;
#endif
}

//! Calculate 32bit integer square root of the given value.
AM_INLINE AM_NOALIAS AM_PURE AMuint32 amIntSqrt32(AMuint32 value) {

	AMuint32 g = 0;
	AMuint32 bshift = 15;
	AMuint32 b = 1 << bshift;

	do {
		AMuint32 temp = (g + g + b) << bshift;
		if (value >= temp) {
			g += b;
			value -= temp;
		}
		b >>= 1;
	} while (bshift--);
	return g;
}

/*!
	\brief Calculates the square root of the given value (float version).
	\pre value >= 0.0f.
*/
AM_INLINE AM_NOALIAS AM_PURE AMfloat amSqrtf(const AMfloat value) {

	AM_ASSERT(value >= 0.0f);

	#if defined(AM_OS_BREW)
		return (AMfloat)FSQRT(value);
	#else
		return (AMfloat)sqrt(value);
	#endif
}

//! Calculates the square root of the given value (fast float version).
AM_INLINE AM_NOALIAS AM_PURE AMfloat amFastSqrtf(AMfloat value) {

#if defined(AM_OS_WIN32) && defined(AM_CC_MSVC)
	AMfloat res;

	__asm {
		fld value
		fsqrt
		fstp res
	}
	return res;
#else
	return amSqrtf(value);
#endif
}

/*!
	\brief Degree to radian conversion (float version).
	\param degValue value to convert, expressed in degrees.
	\return radian representation of degValue parameter.
*/
AM_INLINE AM_NOALIAS AM_PURE AMfloat amDeg2Radf(const AMfloat degValue) {

	return (degValue * AM_DEG2RAD);
}

/*!
	\brief Radians to degrees conversion (float version).
	\param radValue value to convert, expressed in radians.
	\return Degree representation of RadValue parameter.
*/
AM_INLINE AM_NOALIAS AM_PURE AMfloat amRad2Degf(const AMfloat radValue) {

	return (radValue * AM_RAD2DEG);
}

/*!
	\brief Linear interpolation between two values (float version).
	\param lerpAmount interpolation amount.
	\param a first value.
	\param b second value.
	\pre 0.0f <= lerpAmount <= 1.0f.
	\note The implementation is numerically stable.
*/
AM_INLINE AM_NOALIAS AM_PURE AMfloat amLerpf(const AMfloat lerpAmount,
											 const AMfloat a,
											 const AMfloat b) {

	AM_ASSERT(lerpAmount >= 0 && lerpAmount <= 1);

	return (lerpAmount <= 0.5f) ? (a + (b - a) * lerpAmount) : (b + (a - b) * (1.0f - lerpAmount));
}

/*!
	\brief Calculate hypotenuse of two scalar values (float version).
	\param a first input value.
	\param b second input value.
	\note This function is very stable by avoiding underflow/overflow. It uses (a * sqrt(1 + (b/a) * (b/a)))
	rather than	sqrt(a * a + b * b).
*/
AM_INLINE AM_NOALIAS AM_PURE AMfloat amHypotf(const AMfloat a,
											  const AMfloat b) {

	if (a != 0.0f) {
		AMfloat c = b / a;
		return amAbsf(a) * amSqrtf(1.0f + c * c);
	}
	return amAbsf(b);
}

// Quadratic formula used to compute the two roots of the given 2nd degree polynomial.
AMint32 amQuadraticFormulaf(AMfloat *r1,
							AMfloat *r2,
							const AMfloat a,
							const AMfloat b,
							const AMfloat c);

#endif
