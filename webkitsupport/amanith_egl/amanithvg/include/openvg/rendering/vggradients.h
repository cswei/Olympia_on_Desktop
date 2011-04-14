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

#ifndef _VGGRADIENTS_H
#define _VGGRADIENTS_H

/*!
	\file vggradients.h
	\brief Gradients utilities, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "vgpaint.h"
#include "vgcontext.h"

//! Atan2 table (used for conical gradients) half width.
#define AM_ATAN2_TABLE_SIZE_K ((AM_GRADIENTS_CONICAL_TEXTURE_WIDTH - 1) * 0.5f)
//! Atan2 table precision bits.
#define AM_ATAN2_TABLE_PRECISION_BITS 13
//! Linear and radial gradients, texture width in pixels.
#define AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH (1 << AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS)
//! Conical gradients, texture width in pixels.
#define AM_GRADIENTS_CONICAL_TEXTURE_WIDTH (1 << AM_GRADIENTS_CONICAL_TEXTURE_WIDTH_BITS)
//! Conical gradients, precision bits to represent repeats.
#define AM_GRADIENTS_CONICAL_REPEATS_PRECISION_BITS 5
//! Conical gradients, maximum number of repeats.
#define AM_GRADIENTS_CONICAL_MAX_REPEATS (1 << (32 - AM_ATAN2_TABLE_PRECISION_BITS - AM_GRADIENTS_CONICAL_REPEATS_PRECISION_BITS - AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS))

// Generate textures used to realize linear and radial gradients.
AMbool amGradientsTexturesUpdate(AMPaintDesc *paintDesc,
							   const AMContext *context,
							   const AMDrawingSurface *surface,
							   const AMuint32 ctReferenceHash);

#endif
