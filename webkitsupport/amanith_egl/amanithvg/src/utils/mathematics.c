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
	\file mathematics.c
	\brief Math routines, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif


#include "mathematics.h"

// PI constant
const AMfloat AM_PI = 3.1415927f;
// 2 * PI constant
const AMfloat AM_2PI = 6.2831855f;
// PI / 180
const AMfloat AM_DEG2RAD = 0.017453292f;
// 180 / PI
const AMfloat AM_RAD2DEG = 57.295780f;

#if defined(AM_OS_BREW)

const AMdouble sq2p1 = 2.414213562373095048802e0;
const AMdouble sq2m1  = .414213562373095048802e0;
const AMdouble p4  = .161536412982230228262e2;
const AMdouble p3  = .26842548195503973794141e3;
const AMdouble p2  = .11530293515404850115428136e4;
const AMdouble p1  = .178040631643319697105464587e4;
const AMdouble p0  = .89678597403663861959987488e3;
const AMdouble q4  = .5895697050844462222791e2;
const AMdouble q3  = .536265374031215315104235e3;
const AMdouble q2  = .16667838148816337184521798e4;
const AMdouble q1  = .207933497444540981287275926e4;
const AMdouble q0  = .89678597403663861962481162e3;
const AMdouble PIO2 = 1.5707963267948966135e0;
const AMdouble PI = 3.1415926535897932384e0;
const AMdouble nan = 10e38;

AMdouble mxatan(const AMdouble arg) {

    AMdouble argsq, value;

    argsq = arg * arg;
    value = ((((p4 * argsq + p3) * argsq + p2) * argsq + p1) * argsq + p0);
    value = value / (((((argsq + q4) * argsq + q3) * argsq + q2) * argsq + q1) * argsq + q0);
    return value * arg;
}

AMdouble msatan(const AMdouble arg) {

    if (arg < sq2m1)
        return mxatan(arg);
    if (arg > sq2p1)
        return PIO2 - mxatan(1 / arg);
	return PIO2 / 2 + mxatan((arg - 1) / (arg + 1));
}

AMdouble brewAcosd(const AMdouble value) {

	if (value > 1.0 || value < -1.0)
        return nan;
    return PIO2 - brewAsind(value);
}

AMfloat brewAcosf(const AMfloat value) {

	return (AMfloat)brewAcosd(value);
}

AMdouble brewAsind(const AMdouble value) {

    AMdouble temp, arg = value;
    AMint32 sign = 0;

    if (arg < 0.0) {
        arg = -arg;
        sign++;
    }
    if (arg > 1.0)
        return nan;

	temp = FSQRT(1.0 - arg * arg);
	temp = (arg > 0.7) ? PIO2 - brewAtand(temp / arg) : brewAtand(arg / temp);
    if (sign > 0)
        temp = -temp;
    return temp;
}

AMfloat brewAsinf(const AMfloat value) {

	return (AMfloat)brewAsind(value);
}

AMdouble brewAtand(const AMdouble value) {

	if (value > 0.0)
        return msatan(value);
    return -msatan(-value);
}

AMfloat brewAtanf(const AMfloat value) {

	return (AMfloat)brewAtand(value);
}

AMdouble brewAtan2d(const AMdouble y,
					const AMdouble x) {

	AMdouble arg1 = y;
	AMdouble arg2 = x;

    if (arg1 + arg2 == arg1) {
        if (arg1 >= 0.0)
			return PIO2;
		return -PIO2;
    }
    arg1 = brewAtand(arg1 / arg2);
    if (arg2 < 0.0) {
        if (arg1 <= 0.0)
            return arg1 + PI;
        return arg1 - PI;
    }
    return arg1;
}

AMfloat brewAtan2f(const AMfloat y,
				   const AMfloat x) {

	return (AMfloat)brewAtan2d(y, x);
}

#endif

/*!
	\brief Quadratic formula used to compute the two roots of the given 2nd degree polynomial in the form
	of ax^2 + bx + c. Here's a numerical stable implementation, avoiding underflow/overflow (float version).
	\param r1 first output root.
	\param r2 second output root.
	\param a the coefficient to x^2.
	\param b the coefficient to x^1.
	\param c the coefficient to x^0.
	\return number of roots.
*/
AMint32 amQuadraticFormulaf(AMfloat *r1,
							AMfloat *r2,
							const AMfloat a,
							const AMfloat b,
							const AMfloat c) {

	AMfloat stableA, stableB, stableC;

	AM_ASSERT(r1);
	AM_ASSERT(r2);

	if (amAbsf(a) <= AM_EPSILON_FLOAT) {
		if (amAbsf(b) <= AM_EPSILON_FLOAT)
			return 0;
		(*r1) = (*r2) = (-c / b);
		return 1;
	}
	else {
		const AMfloat det = b * b - 4.0f * a * c;

		if (amAbsf(det) <= AM_EPSILON_FLOAT) {
			(*r1) = (*r2) = -b / (2.0f * a);
			return 1;
		}
		if (det > 0.0f) {
			if (amAbsf(b) <= AM_EPSILON_FLOAT) {
				(*r2) = amSqrtf(-c / a);
				(*r1) = -(*r2);
				return 2;
			}
			stableA = b / (2.0f * a);
			stableB = c / (a * stableA * stableA);
			stableC = -1.0f - amSqrtf(1.0f - stableB);
			(*r2) = stableA * stableC;
			(*r1) = (stableA * stableB) / stableC;
			return 2;
		}
		return 0;
	}
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif


