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
	\file int64_math.c
	\brief 64 bit integer mathematics, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "int64_math.h"

#if defined(AM_NO_64BIT_SUPPORT)

typedef union _AMUnsigned32 {
	struct {
	#if defined(AM_BIG_ENDIAN)
		// Motorola, Power PC
		//! Higher 16bit part.
		AMuint16 hi;
		//! Lower 16bit part.
		AMuint16 lo;
	#elif defined(AM_LITTLE_ENDIAN)
		// Intel
		//! Lower 16bit part.
		AMuint16 lo;
		//! Higher 16bit part.
		AMuint16 hi;
	#else
		#error Unreachable point, please define target machine endianess (check config.h inclusion).
	#endif
	} c;
	//! Alias to refer the whole 32bit value.
	AMuint32 n;
} AMUnsigned32;

// negate a 64bit value
AMint64 amInt64Neg(const AMint64 v64) {

	AMint64 res;

	// two's complement
	res.c.lo = (v64.c.lo ^ 0xFFFFFFFF) + 1;
	res.c.hi = v64.c.hi ^ 0xFFFFFFFF;
	if (res.c.lo < 1)
		res.c.hi++;
	return res;
}

// add a 32bit value to a 64 bit value
AMuint64 amUint64Uint32Add(const AMuint64 a64,
						   const AMuint32 b32) {

	AMuint64 res;
	
	res.c.lo = a64.c.lo + b32;
	res.c.hi = a64.c.hi;
	if (res.c.lo < b32)
		res.c.hi++;
	return res;
}

// add two 64bit values
AMuint64 amUint64Add(const AMuint64 a64,
					 const AMuint64 b64) {

	AMuint64 res;

	res.c.lo = a64.c.lo + b64.c.lo;
	res.c.hi = a64.c.hi + b64.c.hi;
	if (res.c.lo < b64.c.lo)
		res.c.hi++;
	return res;
}

// left shift
AMuint64 amUint64LeftShift(const AMuint64 v64,
						   const AMuint8 n) {

	AMuint64 res;

	if (n > 32) {
		res.c.hi = v64.c.lo << (n - 32);
		res.c.lo = 0;
	}
	else {
		res.c.hi = (v64.c.hi << n) | (v64.c.lo >> (32 - n));
		res.c.lo = v64.c.lo << n;
	}
	return res;
}

// right shift
AMuint64 amUint64RightShift(const AMuint64 v64,
							const AMuint8 n) {

	AMuint64 res;

	if (n > 32) {
		res.c.lo = v64.c.hi >> (n - 32);
		res.c.hi = 0;
	}
	else {
		res.c.lo = (v64.c.lo >> n) | (v64.c.hi << (32 - n));
		res.c.hi = v64.c.hi >> n;
	}
	return res;
}

AMint64 amInt64RightShift(const AMint64 v64,
						  const AMuint8 n) {

	AMint64 res;
	AMuint32 bitSign;

	bitSign = (v64.c.hi & 0x80000000);

	if (n > 32) {
		if (bitSign) {
			AMuint32 mask = 0xFFFFFFFF << (n - 32);
			res.c.hi = 0xFFFFFFFF;
			res.c.lo = (v64.c.hi >> (n - 32)) | mask;
		}
		else {
			res.c.hi = 0;
			res.c.lo = (v64.c.hi >> (n - 32));
		}
	}
	else {
		AMuint32 mask = (bitSign) ? 0xFFFFFFFF << (32 - n) : 0;
		res.c.lo = (v64.c.lo >> n) | (v64.c.hi << (32 - n));
		res.c.hi = (v64.c.hi >> n) | mask;
	}
	return res;
}

AMuint64 amUint32Uint32Mul(const AMuint32 a32,
						   const AMuint32 b32) {

	AMUnsigned32 w0, w1, resLo, resHi, t0, t1, t2, t3;
	AMuint32 c0, c1;
	AMuint64 res;

	w0.n = a32;
	w1.n = b32;
	t0.n = (AMuint32)w0.c.lo * w1.c.lo;
	t1.n = (AMuint32)w0.c.lo * w1.c.hi;
	t2.n = (AMuint32)w0.c.hi * w1.c.lo;
	t3.n = (AMuint32)w0.c.hi * w1.c.hi;
	// compute low 32bit part
	c0 = 0;
	resLo.n = t0.n;
	resLo.c.hi += t1.c.lo;
	if (resLo.c.hi < t1.c.lo)
		c0++;
	resLo.c.hi += t2.c.lo;
	if (resLo.c.hi < t2.c.lo)
		c0++;
	// compute high 32bit part
	c1 = 0;
	resHi.n = t3.n;
	resHi.c.lo += t1.c.hi;
	if (resHi.c.lo < t1.c.hi)
		c1++;
	resHi.c.lo += t2.c.hi;
	if (resHi.c.lo < t2.c.hi)
		c1++;
	resHi.c.lo += c0;
	if (resHi.c.lo < c0)
		c1++;
	resHi.c.hi += c1;
	// output the result
	res.c.lo = resLo.n;
	res.c.hi = resHi.n;
	return res;
}

AMint64 amInt32Int32Mul(const AMint32 a32,
						const AMint32 b32) {

	AMuint32 sgn0, sgn1, w0, w1;
	AMint64 res;

	// extract v0 sign and module
	sgn0 = a32 & 0x80000000;
	w0 = (sgn0) ? -a32 : a32;
	// extract v1 sign and module
	sgn1 = b32 & 0x80000000;
	w1 = (sgn1) ? -b32 : b32;

	res = amUint32Uint32Mul(w0, w1);
	// apply two's complement
	if (sgn0 != sgn1)
		res = amInt64Neg(res);
	// debug stuff, do not remove it
#if defined(_DEBUG) && defined(AM_CC_MSVC)
	{
		__int64 r = (__int64)a32 * b32;
		AM_ASSERT(r == res.i64);
	}
#endif
	return res;
}

// 64bit * 32bit = 64bit
AMuint64 amUint64Uint32Mul(const AMuint64 a64,
						   const AMuint32 b32) {

	AMuint64 t0 = amUint32Uint32Mul(a64.c.lo, b32);
	AMuint64 t1 = amUint32Uint32Mul(a64.c.hi, b32);

	t0.c.hi += t1.c.lo;
	// debug stuff, do not remove it
#if defined(_DEBUG) && defined(AM_CC_MSVC)
	{
		unsigned __int64 r = a64.u64 * b32;
		AM_ASSERT(r == t0.u64);
	}
#endif
	return t0;
}

AMint64 amInt64Int32Mul(const AMint64 a64,
						const AMint32 b32) {

	AMuint32 sgnA64 = a64.c.hi & 0x80000000;
	AMuint32 sgnB32 = b32 & 0x80000000;
	AMuint64 t0, t1;
	AMint64 v64;
	AMint32 w32;

	// extract absolute values
	v64 = (sgnA64) ? amInt64Neg(a64) : a64;
	w32 = (sgnB32) ? -b32 : b32;
	// perform the unsigned multiplication
	t0 = amUint32Uint32Mul(v64.c.lo, w32);
	t1 = amUint32Uint32Mul(v64.c.hi, w32);
	t0.c.hi += t1.c.lo;
	// apply the sign rule
	if (sgnA64 != sgnB32)
		t0 = amInt64Neg(t0);
	// debug stuff, do not remove it
#if defined(_DEBUG) && defined(AM_CC_MSVC)
	{
		__int64 r = a64.i64 * b32;
		AM_ASSERT(r == t0.i64);
	}
#endif
	return t0;
}

// 64bit * 64bit = 64bit
AMint64 amInt64Int64Mul(const AMint64 a64,
						const AMint64 b64) {

	AMuint32 sgnA64 = a64.c.hi & 0x80000000;
	AMuint32 sgnB64 = b64.c.hi & 0x80000000;
	AMuint64 t0, t1, t2;
	AMint64 v64, w64;

	// extract absolute values
	v64 = (sgnA64) ? amInt64Neg(a64) : a64;
	w64 = (sgnB64) ? amInt64Neg(b64) : b64;
	// perform the unsigned multiplication
	t0 = amUint32Uint32Mul(v64.c.lo, w64.c.lo);
	t1 = amUint32Uint32Mul(v64.c.lo, w64.c.hi);
	t2 = amUint32Uint32Mul(v64.c.hi, w64.c.lo);
	t0.c.hi += t1.c.lo;
	t0.c.hi += t2.c.lo;
	// apply the sign rule
	if (sgnA64 != sgnB64)
		t0 = amInt64Neg(t0);
	// debug stuff, do not remove it
#if defined(_DEBUG) && defined(AM_CC_MSVC)
	{
		__int64 r = a64.i64 * b64.i64;
		AM_ASSERT(r == t0.i64);
	}
#endif
	return t0;
}

// 64bit / 64bit = 64bit
AMuint64 amUint64Uint64DivFull(AMuint64 dividend64,
							   AMuint64 divisor64) {

    AMint32 i, j, n, x1ne0, y1ne0, xgey;
    AMuint32 x32, y32;
    AMuint64 res;

    // division by 0 ?
    if (!(divisor64.c.lo | divisor64.c.hi)) {
		if (!(dividend64.c.lo | dividend64.c.hi)) {
			res.c.lo = 0;
			res.c.hi = 0;
        } else {
			res.c.lo = 0;
			res.c.hi = 0x80000000;
        }
		return res;
    }

    // extract non-zero components (they are always granted)
	x1ne0 = (dividend64.c.hi != 0);
	y1ne0 = (divisor64.c.hi != 0);
	x32 = (dividend64.c.hi) ? dividend64.c.hi : dividend64.c.lo;
	y32 = (divisor64.c.hi) ? divisor64.c.hi : divisor64.c.lo;

	i = (x32 >> 16) ? 24 : 8;
	j = (y32 >> 16) ? 24 : 8;
	i = (x32 >> i) ? i + 4 : i - 4;
	j = (y32 >> j) ? j + 4 : j - 4;
	i = (x32 >> i) ? i + 2 : i - 2;
	j = (y32 >> j) ? j + 2 : j - 2;
	i = (x32 >> i) ? i + 1 : i - 1;
	j = (y32 >> j) ? j + 1 : j - 1;
    i -= !(x32 >> i);
    j -= !(y32 >> j);

    // calculate offset and perform the y alignement
    n = i - j;
    divisor64.c.hi = (divisor64.c.hi << (n & 31)) |	((i != j) ? (divisor64.c.lo >> ((32 - n) & 31)) : 0);
	divisor64.c.lo <<= (n & 31);
    // number of result bits
    n += (x1ne0 - y1ne0) << 5;
	if (n > 31) {
		divisor64.c.hi = divisor64.c.lo;
		divisor64.c.lo = 0;
	}

	res.c.lo = 0;
	res.c.hi = 0;
    // at each iteration, we calculate a single bit of the quotient
    while (n-- >= 0) {
		xgey = (dividend64.c.hi != divisor64.c.hi) ? (dividend64.c.hi > divisor64.c.hi) : (dividend64.c.lo >= divisor64.c.lo);
        res.c.hi += res.c.hi + (((AMuint32)(res.c.lo) + (AMuint32)(res.c.lo)) < ((AMuint32)(res.c.lo)));
        res.c.lo += res.c.lo + xgey;
		if (xgey) {
			dividend64.c.hi -= (divisor64.c.hi + (dividend64.c.lo < divisor64.c.lo));
			dividend64.c.lo -= divisor64.c.lo;
		}
		divisor64.c.lo = (divisor64.c.lo >> 1) | (divisor64.c.hi << 31);
        divisor64.c.hi >>= 1;
    }

	return res;
}

AMint64 amInt64Int64DivFull(AMint64 dividend64,
							AMint64 divisor64) {

	AMuint32 sgnDividend, sgnDivisor;
	AMuint64 absDividend, absDivisor;
	AMint64 res;

	// extract the dividend sign and absolute value 
	sgnDividend = dividend64.c.hi & 0x80000000;
	absDividend = (sgnDividend) ? amInt64Neg(dividend64) : dividend64;
	// extract the dividend sign and absolute value 
	sgnDivisor = divisor64.c.hi & 0x80000000;
	absDivisor = (sgnDivisor) ? amInt64Neg(divisor64) : divisor64;
	// perform unsigned division
	res = amUint64Uint64DivFull(absDividend, absDivisor);
	// apply two's complement
	if (sgnDividend != sgnDivisor)
		res = amInt64Neg(res);
	return res;
}

// 64bit / 32bit = 32bit
AMuint32 amUint64Uint32Div(AMuint64 dividend64,
						   AMuint32 divisor32) {

	AMuint64 divisor64, res;

	divisor64.c.hi = 0;
	divisor64.c.lo = divisor32;
	res = amUint64Uint64DivFull(dividend64, divisor64);
	return res.c.lo;
}

AMint32 amInt64Int32Div(AMint64 dividend64,
						AMint32 divisor32) {

	AMuint64 divisor64, res;

	divisor64.c.hi = (divisor32 < 0) ? 0xFFFFFFFF : 0;
	divisor64.c.lo = divisor32;
	res = amInt64Int64DivFull(dividend64, divisor64);
	return res.c.lo;
}

// 64bit / 64bit = 32bit
AMuint32 amUint64Uint64Div(AMuint64 dividend64,
						   AMuint64 divisor64) {

	AMuint64 res;

	res = amUint64Uint64DivFull(dividend64, divisor64);
	// debug stuff, do not remove it
#if defined(_DEBUG) && defined(AM_CC_MSVC)
	{
		AMuint32 r = (AMuint32)(dividend64.u64 / divisor64.u64);
		AM_ASSERT(r == res.c.lo);
	}
#endif
	return res.c.lo;
}

AMint32 amInt64Int64Div(AMint64 dividend64,
						AMint64 divisor64) {

	AMuint64 res;

	res = amInt64Int64DivFull(dividend64, divisor64);
	// debug stuff, do not remove it
#if defined(_DEBUG) && defined(AM_CC_MSVC)
	{
		AMint32 r = (AMint32)(dividend64.i64 / divisor64.i64);
		AM_ASSERT(r == res.c.lo);
	}
#endif
	return res.c.lo;
}

// signed 64bit integer from float
AMint64 amInt64FromFloat(AMfloat f) {

	AMuint64 res;
    AMuint32 data = *(AMuint32*)&f;
    AMint8 exp = (AMint8)((data >> 23) & 0xff) - 127 - 23;

	res.c.lo = (data & 0x007fffff) | 0x00800000;
	res.c.hi = 0;
	if (exp > 0)
		res = amUint64LeftShift(res, exp);
	else {
		if (exp < -23)
			res.c.lo = 0;
		else
			res = amUint64RightShift(res, -exp);
	}
	// check the sign bit
	if (data & 0x80000000)
		res = amInt64Neg(res);

	return res;
}

// clamp a signed 64bit value between 0 and a positive 8bit number; the result is an unsigned 32bit
AMuint32 amInt64ZeroUint8Clamp(const AMint64 v64,
							   const AMuint8 unsignedInt8) {

	if (v64.c.hi & 0x80000000)
		return 0;

	if ((v64.c.hi) || (v64.c.lo > (AMuint32)unsignedInt8))
		return (AMuint32)unsignedInt8;

	return v64.c.lo;
}


// "greater than" comparison
AMbool amUint64Greater(const AMuint64 a64,
					   const AMuint64 b64) {

	if (a64.c.hi > b64.c.hi)
		return AM_TRUE;
	else
	if (a64.c.hi < b64.c.hi)
		return AM_FALSE;
	else {
		if (a64.c.lo > b64.c.lo)
			return AM_TRUE;
		return AM_FALSE;
	}
}

AMbool amInt64Greater(const AMint64 a64,
					  const AMint64 b64) {

	AMuint32 bitSign0 = a64.c.hi & 0x80000000;
	AMuint32 bitSign1 = b64.c.hi & 0x80000000;

	// a64 positive, b64 negative
	if (!bitSign0 && bitSign1)
		return AM_TRUE;
	// a64 negative, b64 positive
	else
	if (bitSign0 && !bitSign1)
		return AM_FALSE;

	// a64 and b64 have the same sign (both positive or both negative)
	return amUint64Greater(a64, b64);
}

// "greater or equal than" comparison
AMbool amUint64GreaterEqual(const AMuint64 a64,
							const AMuint64 b64) {

	if (a64.c.hi > b64.c.hi)
		return AM_TRUE;
	else
	if (a64.c.hi < b64.c.hi)
		return AM_FALSE;
	else {
		if (a64.c.lo >= b64.c.lo)
			return AM_TRUE;
		return AM_FALSE;
	}
}

// "lesser than" comparison
AMbool amUint64Lesser(const AMuint64 a64,
					  const AMuint64 b64) {

	if (a64.c.hi < b64.c.hi)
		return AM_TRUE;
	else
	if (a64.c.hi > b64.c.hi)
		return AM_FALSE;
	else {
		if (a64.c.lo < b64.c.lo)
			return AM_TRUE;
		return AM_FALSE;
	}
}

AMbool amInt64Lesser(const AMint64 a64,
					 const AMint64 b64) {

	AMuint32 bitSign0 = a64.c.hi & 0x80000000;
	AMuint32 bitSign1 = b64.c.hi & 0x80000000;

	// a64 positive, b64 negative
	if (!bitSign0 && bitSign1)
		return AM_FALSE;
	// a64 negative, b64 positive
	else
	if (bitSign0 && !bitSign1)
		return AM_TRUE;

	// a64 and b64 have the same sign (both positive or both negative)
	return amUint64Lesser(a64, b64);
}

// "lesser or equal than" comparison
AMbool amUint64LesserEqual(const AMuint64 a64,
						   const AMuint64 b64) {

	if (a64.c.hi < b64.c.hi)
		return AM_TRUE;
	else
	if (a64.c.hi > b64.c.hi)
		return AM_FALSE;
	else {
		if (a64.c.lo <= b64.c.lo)
			return AM_TRUE;
		return AM_FALSE;
	}
}

// "greater than zero" comparison
AMbool amInt64GreaterZero(const AMint64 v64) {

	if (v64.c.hi & 0x80000000)
		return AM_FALSE;
	else {
		// value is > 0
		if (v64.c.hi != 0 || v64.c.lo != 0)
			return AM_TRUE;
		// value is 0
		return AM_FALSE;
	}
}

// "greater or equal zero" comparison
AMbool amInt64GreaterEqualZero(const AMint64 v64) {

	if (v64.c.hi & 0x80000000)
		return AM_FALSE;
	return AM_TRUE;
}

// "lesser than zero" comparison
AMbool amInt64LesserZero(const AMint64 v64) {

	if (v64.c.hi & 0x80000000)
		return AM_TRUE;
	return AM_FALSE;
}

// "lesser or equal zero" comparison
AMbool amInt64LesserEqualZero(const AMint64 v64) {

	if (v64.c.hi & 0x80000000)
		return AM_TRUE;
	else {
		if (v64.c.hi != 0 || v64.c.lo != 0)
			return AM_FALSE;
		// value is 0
		return AM_TRUE;
	}
}

#else
#endif
