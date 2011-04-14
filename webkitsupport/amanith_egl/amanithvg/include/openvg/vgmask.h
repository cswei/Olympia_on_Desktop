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

#ifndef _VGMASK_H
#define _VGMASK_H

/*!
	\file vgmask.h
	\brief Masking and clearing, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "vg_priv.h"
#include "vgcontext.h"

// Update the drawing surface mask values.
void amMaskUpdate(AMDrawingSurface *surface,
					const AMImage *mask,
					const VGMaskOperation operation,
					const AMint32 x,
					const AMint32 y,
					const AMint32 width,
					const AMint32 height);
// Clear the drawing surface.
AMbool amDrawingSurfaceClear(AMContext *context,
							 AMDrawingSurface *surface,
							 AMint32 x,
							 AMint32 y,
							 AMint32 width,
							 AMint32 height);
#if defined RIM_VG_SRC
// Clear the internal drawing surface, meant to be used upon surface initialization.
void amInternalDrawingSurfaceClear(AMDrawingSurface *surface,
						           AMint32 x,
						           AMint32 y,
						           AMint32 width,
						           AMint32 height);
#endif

#endif
