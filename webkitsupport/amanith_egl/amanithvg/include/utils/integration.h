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

#ifndef _INTEGRATION_H
#define _INTEGRATION_H

/*!
	\file integration.h
	\brief Romberg integrator, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "amanith_globals.h"

//! Type definition of a scalar function callback.
typedef AMfloat (*AMFloatScalarFunction)(const AMfloat,
										 void *);

// Romberg integration for continuous scalar functions.
AMbool amRombergf(AMfloat *result,
				  const AMfloat u0,
				  const AMfloat u1,
				  AMFloatScalarFunction function,
				  void *userData,
				  const AMfloat maxError);


#endif
