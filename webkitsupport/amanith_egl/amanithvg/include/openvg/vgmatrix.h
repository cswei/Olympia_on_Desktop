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

#ifndef _VGMATRIX_H
#define _VGMATRIX_H

/*!
	\file vgmatrix.h
	\brief OpenVG matrix, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "matrix.h"

//! Bit value to indicate that an OpenVG matrix scale components have been changed.
#define AM_MATRIX_SCALE_MODIFIED			1
//! Bit value to indicate that an OpenVG inverse matrix must be recalculated.
#define AM_MATRIX_INVERSE_MODIFIED			2
//! Bit value to indicate that an OpenVG matrix is singular.
#define AM_MATRIX_SINGULAR					4
#if defined(AM_GLE)
	//! Bit value to indicate that OpenGL / OpenGL ES modelview matrix must be reloaded (GLE specific to minimize matrix uploading time).
	#define AM_MATRIX_MODELVIEW_MODIFIED	8
#endif

// Structure used to store information relative to a "user to surface transformation".
typedef struct _AMUserToSurfaceDesc {
	//! User to surface matrix, floating point components.
	const AMMatrix33f *userToSurface;
#if defined(AM_FIXED_POINT_PIPELINE)
	//! User to surface matrix, 17.15 fixed point components.
	AMMatrix33x userToSurfacex;
#endif
	//! User to surface inverse matrix, floating point components.
	const AMMatrix33f *inverseUserToSurface;
	//! Maximum scaling factor extracted from the user to surface matrix.
	const AMfloat *userToSurfaceScale;
	/*!
		Bitfield flag, indicating if the matrix has been modified (see AM_MATRIX_SCALE_MODIFIED), if
		the inverse matrix must be recalculated (see AM_MATRIX_INVERSE_MODIFIED), if the
		matrix is singular (see AM_MATRIX_SINGULAR).
	*/
	AMuint32 flags;
	//! AM_TRUE if the user to surface matrix is affine, else AM_FALSE.
	AMbool userToSurfaceAffine;
} AMUserToSurfaceDesc;

// Update matrices and scale factors.
void amMatricesUpdate(void *_context);

#endif
