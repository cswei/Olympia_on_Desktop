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
	\file integration.c
	\brief Romberg integrator, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "integration.h"
#include "mathematics.h"

/*
   Romberg formula
  
                            R(j, k-1) - R(j-1, k-1)
    R(j, k) = R(j, k-1) +  -------------------------
                                     k
                                    4  - 1
  
   Richardson (extrapolation coefficients) table for k = 1..11
   RichardsonInvK[k] = 1 / (4^k - 1)
*/

//! Size of the table containing Richardson extrapolation coefficients.
#define AM_RICHARDSON_TABLE_FLOAT_SIZE 11
#define AM_RICHARDSON_TABLE_FLOAT_HALF_SIZE (AM_RICHARDSON_TABLE_FLOAT_SIZE / 2)

const AMfloat richardsonInvKf[AM_RICHARDSON_TABLE_FLOAT_SIZE] = {
	3.333333433e-01f, 6.666667014e-02f, 1.587301679e-02f,
	3.921568859e-03f, 9.775171056e-04f, 2.442002587e-04f,
	6.103888154e-05f, 1.525902189e-05f, 3.814711818e-06f,
	9.536752259e-07f, 2.384186359e-07f
};

/*!
	\brief It integrates a scalar function over the specified interval, using Romberg schema.
	\param result result of integration.
	\param u0 lower bound of integration interval.
	\param u1 upper bound of integration interval.
	\param function pointer to the scalar function to integrate.
	\param userData pointer to custom user data to pass at the scalar function.
	\param maxError maximum allowed error.
	\return AM_TRUE if the error has been bound in less than AM_RICHARDSON_TABLE_FLOAT_SIZE iterations.
*/
AMbool amRombergf(AMfloat *result,
				  const AMfloat u0,
				  const AMfloat u1,
				  AMFloatScalarFunction function,
				  void *userData,
				  const AMfloat maxError) {

	AMuint32 k;
	AMfloat dt[1 + AM_RICHARDSON_TABLE_FLOAT_SIZE];
	AMfloat delta = 0.0f;
	AMfloat maxMachineError = AM_MIN(maxError,  2.0f * AM_EPSILON_FLOAT);
	AMfloat oldErr = AM_MAX_FLOAT;
	// initial interval and integralApprox approximation
	AMfloat h = u1 - u0;
	AMfloat integralApprox = 0.5f * (function(u0, userData) + function(u1, userData));

	AM_ASSERT(result);
	AM_ASSERT(function != 0);

	// initialize dt[0] to the integral numerical approximation (a trapezoid, h step)
	dt[0] = integralApprox * h;
	// calculate the new integral approximation using the Romberg rule and Richardson extrapolation
	for (k = 1; k < AM_RICHARDSON_TABLE_FLOAT_SIZE + 1; ++k) {
		
		AMfloat err, x;
		AMuint32 j;
		AMfloat old_h = h;

		h *= 0.5f;
		integralApprox = 0.0f;
		for (x = u0 + h; x < u1; x += old_h)
			integralApprox += function(x, userData);
		integralApprox = h * integralApprox + 0.5f * dt[0];
		for (j = 0; j < k; ++j) {
			delta = integralApprox - dt[j];
			dt[j] = integralApprox;
			integralApprox += richardsonInvKf[j] * delta;
		} 
		// if the current integral approximation doesn't satisfy the specified tolerance, loop again
		err = amAbsf(delta) / (amAbsf(integralApprox) + 1.0f);
		if ((err <= maxMachineError) || ((oldErr < err) && (k > (AM_RICHARDSON_TABLE_FLOAT_HALF_SIZE + 1)))) {
			if (k > 2) {
				*result = integralApprox;
				return (err <= maxMachineError) ? AM_TRUE : AM_FALSE;
			}
		}
		oldErr = err;
		dt[k] = integralApprox;
	}
	// the integral process didn't converge, return the current integral approximation
	*result = integralApprox;
	return AM_FALSE;
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

