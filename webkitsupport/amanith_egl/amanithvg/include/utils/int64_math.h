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

#ifndef _INT64_MATH_H
#define _INT64_MATH_H

/*!
	\file int64_math.h
	\brief 64 bit integer mathematics, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "amanith_globals.h"

#if defined(AM_NO_64BIT_SUPPORT)
	// negate a 64bit value
	AMint64 amInt64Neg(const AMint64 v64);
	// add a 32bit value to a 64 bit value
	AMuint64 amUint64Uint32Add(const AMuint64 a64,
							   const AMuint32 b32);
	// add two 64bit values
	AMuint64 amUint64Add(const AMuint64 a64,
						 const AMuint64 b64);
	// left shift a 64bit value
	AMuint64 amUint64LeftShift(const AMuint64 v64,
							   const AMuint8 n);
	// right shift an unsigned 64bit value
	AMuint64 amUint64RightShift(const AMuint64 v64,
								const AMuint8 n);
	// right shift a signed 64bit value
	AMint64 amInt64RightShift(const AMint64 v64,
							  const AMuint8 n);
	// 32bit * 32bit = 64bit
	AMuint64 amUint32Uint32Mul(const AMuint32 a32,
							   const AMuint32 b32);
	AMint64 amInt32Int32Mul(const AMint32 a32,
							const AMint32 b32);
	// 64bit * 32bit = 64bit
	AMuint64 amUint64Uint32Mul(const AMuint64 a64,
							   const AMuint32 b32);
	AMint64 amInt64Int32Mul(const AMint64 a64,
							const AMint32 b32);
	// 64bit * 64bit = 64bit
	AMint64 amInt64Int64Mul(const AMint64 a64,
							const AMint64 b64);
	// 64bit / 32bit = 32bit
	AMuint32 amUint64Uint32Div(AMuint64 dividend64,
							   AMuint32 divisor32);
	AMint32 amInt64Int32Div(AMint64 dividend64,
							AMint32 divisor32);
	// 64bit / 64bit = 32bit
	AMuint32 amUint64Uint64Div(AMuint64 dividend64,
							   AMuint64 divisor64);
	AMint32 amInt64Int64Div(AMint64 dividend64,
							AMint64 divisor64);
	// signed 64bit integer from float
	AMint64 amInt64FromFloat(AMfloat f);
	// clamp a signed 64bit value between 0 and a positive 8bit number; the result is an unsigned 32bit
	AMuint32 amInt64ZeroUint8Clamp(const AMint64 v64,
								   const AMuint8 unsignedInt8);
	// "greater than" comparison
	AMbool amUint64Greater(const AMuint64 a64,
						   const AMuint64 b64);
	AMbool amInt64Greater(const AMint64 a64,
						  const AMint64 b64);
	// "greater or equal than" comparison
	AMbool amUint64GreaterEqual(const AMuint64 a64,
								const AMuint64 b64);
	// "lesser than" comparison
	AMbool amUint64Lesser(const AMuint64 a64,
						  const AMuint64 b64);
	AMbool amInt64Lesser(const AMint64 a64,
						 const AMint64 b64);
	// "lesser or equal than" comparison
	AMbool amUint64LesserEqual(const AMuint64 a64,
							   const AMuint64 b64);
	// "greater than zero" comparison
	AMbool amInt64GreaterZero(const AMint64 v64);
	// "lesser than zero" comparison
	AMbool amInt64LesserZero(const AMint64 v64);
	// "greater or equal zero" comparison
	AMbool amInt64GreaterEqualZero(const AMint64 v64);
	// "lesser or equal zero" comparison
	AMbool amInt64LesserEqualZero(const AMint64 v64);

	// cast a 64bit value into a 32bit value
	#define UINT64_UINT32_CAST(_v64) ((_v64).c.lo)
	#define INT64_INT32_CAST(_v64) ((AMint32)((_v64).c.lo))
	// cast a signed 64bit value to an unsigned 64bit value
	#define INT64_UINT64_CAST(_v64) (_v64)
	// set a zero value
	#define UINT64_ZERO_SET(_v64) (_v64).c.lo = (_v64).c.hi = 0
	#define INT64_ZERO_SET(_v64) (UINT64_ZERO_SET(_v64))
	// negate a signed 64bit value
	#define INT64_NEG(_v64) (amInt64Neg(_v64))
	// add a 32bit value to a 64 bit value
	#define UINT64_UINT32_ADD(_a64, _b32) (amUint64Uint32Add(_a64, _b32))
	#define INT64_INT32_ADD(_a64, _b32) (UINT64_UINT32_ADD(_a64, _b32))
	// add two 64bit values
	#define UINT64_ADD(_a64, _b64) (amUint64Add(_a64, _b64))
	#define INT64_ADD(_a64, _b64) (UINT64_ADD(_a64, _b64))
	// subtract two 64bit values
	#define INT64_SUB(_a64, _b64) (INT64_ADD(_a64, INT64_NEG(_b64)))
	#define UINT64_SUB(_a64, _b64) (INT64_SUB(_a64, (_b64)))
	// left shift
	#define UINT64_LSHIFT(_v64, _n) (amUnt64LeftShift(_v64, _n))
	#define INT64_LSHIFT(_v64, _n) (UINT64_LSHIFT(_v64, _n))
	// right shift
	#define UINT64_RSHIFT(_v64, _n) (amUint64RightShift(_v64, _n))
	#define INT64_RSHIFT(_v64, _n) (amInt64RightShift(_v64, _n))
	// 32bit x 32bit = 64bit
	#define UINT32_UINT32_MUL(_a32, _b32)	(amUint32Uint32Mul(_a32, _b32))
	#define INT32_INT32_MUL(_a32, _b32)	(amInt32Int32Mul(_a32, _b32))
	// 64bit * 32bit = 64bit
	#define UINT64_UINT32_MUL(_a64, _b32) (amUint64Uint32Mul(_a64, _b32))
	#define INT64_INT32_MUL(_a64, _b32) (amInt64Int32Mul(_a64, _b32))
	// 64bit * 64bit = 64bit
	#define INT64_INT64_MUL(_a64, _b64) (amInt64Int64Mul(_a64, _b64))
	// 64bit / 32bit = 32bit
	#define UINT64_UINT32_DIV(_dividend64, _divisor32) (amUint64Uint32Div(_dividend64, _divisor32))
	#define INT64_INT32_DIV(_dividend64, _divisor32) (amInt64Int32Div(_dividend64, _divisor32))
	// 64bit / 64bit = 32bit
	#define UINT64_UINT64_DIV(_dividend64, _divisor64) (amUint64Uint64Div(_dividend64, _divisor64))
	#define INT64_INT64_DIV(_dividend64, _divisor64) (amInt64Int64Div(_dividend64, _divisor64))
	// signed 64bit integer from float
	#define INT64_FROM_FLOAT(_f) (amInt64FromFloat(_f))
	// clamp a signed 64bit value between 0 and a positive 8bit number; the result is an unsigned 32bit
	#define INT64_CLAMP_ZERO_UINT8(_v64, _unsignedInt8) (amInt64ZeroUint8Clamp(_v64, _unsignedInt8))
	// "equal to" comparison
	#define UINT64_EQUAL(_a64, _b64) (((_a64).c.hi == (_b64).c.hi) && ((_a64).c.lo == (_b64).c.lo))
	#define INT64_EQUAL(_a64, _b64) (UINT64_EQUAL(_a64, _b64))
	// "greater than" comparison
	#define UINT64_GREATER(_a64, _b64) (amUint64Greater(_a64, _b64))
	#define INT64_GREATER(_a64, _b64) (amInt64Greater(_a64, _b64))
	// "greater or equal than" comparison
	#define UINT64_GREATER_EQUAL(_a64, _b64) (amUint64GreaterEqual(_a64, _b64))
	// "lesser than" comparison
	#define UINT64_LESSER(_a64, _b64) (amUint64Lesser(_a64, _b64))
	#define INT64_LESSER(_a64, _b64) (amInt64Lesser(_a64, _b64))
	// "lesser or equal than" comparison
	#define UINT64_LESSER_EQUAL(_a64, _b64) (amUint64LesserEqual(_a64, _b64))
	// "equal zero" comparison
	#define UINT64_EQUAL_ZERO(_v64) (((_v64).c.hi == 0) && ((_v64).c.lo == 0))
	#define INT64_EQUAL_ZERO(_v64) (UINT64_EQUAL_ZERO(_v64))
	// "greater than zero" comparison
	#define INT64_GREATER_ZERO(_v64) (amInt64GreaterZero(_v64))
	// "lesser than zero" comparison
	#define INT64_LESSER_ZERO(_v64) (amInt64LesserZero(_v64))
	// "greater or equal zero" comparison
	#define INT64_GREATER_EQUAL_ZERO(_v64) (amInt64GreaterEqualZero(_v64))
	// "lesser or equal zero" comparison
	#define INT64_LESSER_EQUAL_ZERO(_v64) (amInt64LesserEqualZero(_v64))

#else
	#if defined(AM_ARCH_I386)
		#if defined(AM_CC_MSVC)
			#include <intrin.h>
			#pragma intrinsic(__emul)
			#pragma intrinsic(__emulu)
			#define INT32_INT32_MUL(_a32, _b32)	(__emul(_a32, _b32))
			#define UINT32_UINT32_MUL(_a32, _b32)	(__emulu(_a32, _b32))
		#elif defined(AM_CC_GCC)
			AM_INLINE __attribute__((always_inline)) AMint64 INT32_INT32_MUL(const int a, const int b) {

				AMint64 retval;
				__asm__("imull %[b]" : "=A" (retval) : [a] "a" (a), [b] "rm" (b));
				return retval;
			}
			AM_INLINE __attribute__((always_inline)) AMuint64 UINT32_UINT32_MUL(const unsigned int a, const unsigned int b) {
		
				AMuint64 retval;
				__asm__("mull %[b]" : "=A" (retval) : [a] "a" (a), [b] "rm" (b));
				return retval;
			}
		#else
			#define INT32_INT32_MUL(_a32, _b32)	((AMint64)(_a32) * (_b32))
			#define UINT32_UINT32_MUL(_a32, _b32)	((AMuint64)(_a32) * (_b32))
		#endif
	#else
		#define INT32_INT32_MUL(_a32, _b32)	((AMint64)(_a32) * (_b32))
		#define UINT32_UINT32_MUL(_a32, _b32)	((AMuint64)(_a32) * (_b32))
	#endif

	// cast a 64bit value into a 32bit value
	#define UINT64_UINT32_CAST(_v64) ((AMuint32)(_v64))
	#define INT64_INT32_CAST(_v64) ((AMint32)(_v64))
	// cast a signed 64bit value to an unsigned 64bit value
	#define INT64_UINT64_CAST(_v64) (AMuint64)(_v64)
	// set a zero value
	#define UINT64_ZERO_SET(_v64) (_v64) = 0
	#define INT64_ZERO_SET(_v64) (UINT64_ZERO_SET(_v64))
	// negate a signed 64bit value
	#define INT64_NEG(_v64) (-(_v64))
	// add a 32bit value to a 64 bit value
	#define UINT64_UINT32_ADD(_a64, _b32) ((_a64) + (_b32))
	#define INT64_INT32_ADD(_a64, _b32) ((_a64) + (_b32))
	// add two 64bit values
	#define UINT64_ADD(_a64, _b64) ((_a64) + (_b64))
	#define INT64_ADD(_a64, _b64) ((_a64) + (_b64))
	// subtract two 64bit values
	#define UINT64_SUB(_a64, _b64) ((_a64) - (_b64))
	#define INT64_SUB(_a64, _b64) ((_a64) - (_b64))
	// left shift
	#define UINT64_LSHIFT(_v64, _n) ((_v64) << (_n))
	#define INT64_LSHIFT(_v64, _n) ((_v64) << (_n))
	// right shift
	#define UINT64_RSHIFT(_v64, _n) ((_v64) >> (_n))
	#define INT64_RSHIFT(_v64, _n) ((_v64) >> (_n))
	// 64bit * 32bit = 64bit
	#define UINT64_UINT32_MUL(_a64, _b32) ((_a64) * (_b32))
	#define INT64_INT32_MUL(_a64, _b32) ((_a64) * (_b32))
	// 64bit * 64bit = 64bit
	#define INT64_INT64_MUL(_a64, _b64) ((_a64) * (_b64))
	// 64bit / 32bit = 32bit
	#define UINT64_UINT32_DIV(_dividend64, _divisor32) ((_dividend64) / (_divisor32))
	#define INT64_INT32_DIV(_dividend64, _divisor32) ((_dividend64) / (_divisor32))
	// 64bit / 64bit = 32bit
	#define UINT64_UINT64_DIV(_dividend64, _divisor64) ((_dividend64) / (_divisor64))
	#define INT64_INT64_DIV(_dividend64, _divisor64) ((_dividend64) / (_divisor64))
	// signed 64bit integer from float
	#define INT64_FROM_FLOAT(_f) ((AMint64)(_f))
	// clamp a signed 64bit value between 0 and a positive 8bit number; the result is an unsigned 32bit
	#define INT64_CLAMP_ZERO_UINT8(_v64, _unsignedInt8) ((AMuint32)(AM_CLAMP((_v64), 0, (AMint32)(_unsignedInt8))))
	// "equal to" comparison
	#define UINT64_EQUAL(_a64, _b64) ((_a64) == (_b64))
	#define INT64_EQUAL(_a64, _b64) ((_a64) == (_b64))
	// "greater than" comparison
	#define UINT64_GREATER(_a64, _b64) ((_a64) > (_b64))
	#define INT64_GREATER(_a64, _b64) ((_a64) > (_b64))
	// "greater or equal than" comparison
	#define UINT64_GREATER_EQUAL(_a64, _b64) ((_a64) >= (_b64))
	// "lesser than" comparison
	#define UINT64_LESSER(_a64, _b64) ((_a64) < (_b64))
	#define INT64_LESSER(_a64, _b64) ((_a64) < (_b64))
	// "lesser or equal than" comparison
	#define UINT64_LESSER_EQUAL(_a64, _b64) ((_a64) <= (_b64))
	// "equal zero" comparison
	#define UINT64_EQUAL_ZERO(_v64) ((_v64) == 0)
	#define INT64_EQUAL_ZERO(_v64) ((_v64) == 0)
	// "greater than zero" comparison
	#define INT64_GREATER_ZERO(_v64) ((_v64) > 0)
	// "lesser than zero" comparison
	#define INT64_LESSER_ZERO(_v64) ((_v64) < 0)
	// "greater or equal zero" comparison
	#define INT64_GREATER_EQUAL_ZERO(_v64) ((_v64) >= 0)
	// "lesser or equal zero" comparison
	#define INT64_LESSER_EQUAL_ZERO(_v64) ((_v64) <= 0)
#endif

#endif
