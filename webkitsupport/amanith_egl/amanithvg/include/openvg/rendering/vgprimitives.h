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

#ifndef _VGPRIMITIVES_H
#define _VGPRIMITIVES_H

/*!
	\file vgprimitives.h
	\brief Path and image drawing functions entry point, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "vg_priv.h"
#include "vgcontext.h"
#include "vgmatrix.h"

//! Clamp corner coordinates against surface space (16bit).
#define AM_SRFSPACE_CORNER_CLAMP(_dst, _src) \
	if ((_src).x > AM_MAX_INT16_F) \
		(_dst).x = AM_MAX_INT16; \
	else \
	if ((_src).x < AM_MIN_INT16_F) \
		(_dst).x = AM_MIN_INT16; \
	else \
		(_dst).x = (AMint32)((_src).x); \
	if ((_src).y > AM_MAX_INT16_F) \
		(_dst).y = AM_MAX_INT16; \
	else \
	if ((_src).y < AM_MIN_INT16_F) \
		(_dst).y = AM_MIN_INT16; \
	else \
		(_dst).y = (AMint32)((_src).y);

#define AM_SRFSPACE_CORNER_CLAMPX(_dst, _src) \
	if ((_src).x > AM_MAX_INT16) \
		(_dst).x = AM_MAX_INT16; \
	else \
	if ((_src).x < AM_MIN_INT16) \
		(_dst).x = AM_MIN_INT16; \
	else \
		(_dst).x = (_src).x; \
	if ((_src).y > AM_MAX_INT16) \
		(_dst).y = AM_MAX_INT16; \
	else \
	if ((_src).y < AM_MIN_INT16) \
		(_dst).y = AM_MIN_INT16; \
	else \
		(_dst).y = (_src).y;

// Update an axes-aligned box, according to a specified float matrix.
void amAABox2fTransform(AMAABox2f *dst,
						const AMAABox2f *src,
						const AMMatrix33f *matrix);
#if defined(AM_FIXED_POINT_PIPELINE)
// Update an axes-aligned box, according to a specified fixed point matrix.
void amAABox2fTransformx(AMAABox2x *dst,
						 const AMAABox2f *src,
						 const AMMatrix33x *matrix);
#endif

// Update flattening parameters inside the context, according to the current VG_MATRIX_PATH_USER_TO_SURFACE
// matrix and stroke line thickness.
void amPathDeviationUpdate(AMContext *context,
						   const AMPath *path,
						   const VGbitfield paintModes,
						   const AMAABox2i *srfSpaceBox,
						   const AMUserToSurfaceDesc *userToSurfaceDesc);
// Check if an image can be drawn.
AMbool amImageToDraw(AMAABox2i *srfSpaceBox,
					 AMVect2f *q0,
					 AMVect2f *q1,
					 AMVect2f *q2,
					 AMVect2f *q3,
					 const AMImage *image,
					 const AMUserToSurfaceDesc *userToSurfaceDesc);
// Entry point for path drawing function.
AMbool amPathDraw(AMContext *context,
				  AMDrawingSurface *surface,
				  AMPath *path,
				  const VGbitfield paintModes);
// Entry point for image drawing function.
AMbool amImageDraw(AMContext *context,
				   AMDrawingSurface *surface,
				   VGImage handle);

#endif
